/*
 FUSE: Filesystem in Userspace
 Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>

 This program can be distributed under the terms of the GNU GPL.
 See the file COPYING.

 gcc -Wall hello.c `pkg-config fuse --cflags --libs` -o hello
 */

#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>
#include "mount.h"
#include "inode.h"
#include "unixv6fs.h"
#include "direntv6.h"
#include "error.h"

// the filesystem struct
struct unix_filesystem fs;

/**
 * get the attributes
 * @param path
 * @param stbuf
 * @return
 */
static int fs_getattr(const char *path, struct stat *stbuf);

/**
 * Read a directory
 * @param path
 * @param buf
 * @param filler
 * @param offset
 * @param fi
 * @return
 */
static int fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
        off_t offset, struct fuse_file_info *fi);

/**
 * Read the content of a file
 * @param path IN the path
 * @param buf OUT the buffer where to put the data
 * @param size the nb of bytes to read
 * @param offset from where to read
 * @param fi
 * @return the nb of byte read
 */
static int fs_read(const char *path, char *buf, size_t size, off_t offset,
        struct fuse_file_info *fi);

/**
 * From https://github.com/libfuse/libfuse/wiki/Option-Parsing.
 * This will look up into the args to search for the name of the FS.
 */
static int arg_parse(void *data, const char *filename, int key,
        struct fuse_args *outargs);

static struct fuse_operations available_ops = { .getattr = fs_getattr,
        .readdir = fs_readdir, .read = fs_read };

int main(int argc, char *argv[]) {
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    int ret = fuse_opt_parse(&args, NULL, NULL, arg_parse);
    if (ret == 0) {
        ret = fuse_main(args.argc, args.argv, &available_ops, NULL);
        (void) umountv6(&fs);
    }
    return ret;
}

/**
 * get the attributes
 * @param path
 * @param stbuf
 * @return
 */
static int fs_getattr(const char *path, struct stat *stbuf) {
    int res = 0;

    memset(stbuf, 0, sizeof(struct stat));

    int inr = direntv6_dirlookup(&fs, 1, path);
    if (inr < 0) {
        debug_print("fs_getattr 1 %s: %d\n", path, inr);
        return inr;
    }

    struct filev6 file;
    int ret = filev6_open(&fs, (uint16_t) inr, &file);
    if (ret < 0) {
        debug_print("fs_getattr 2: %d\n", ret);
        return ret;
    }

    struct timespec atim = { file.i_node.i_atime[0], file.i_node.i_atime[1] };
    stbuf->st_atim = atim;
    stbuf->st_blksize = SECTOR_SIZE;
    stbuf->st_blocks = inode_getsectorsize(&file.i_node);

    struct timespec ctim = { 0l, 0l };
    stbuf->st_ctim = ctim;
    stbuf->st_dev = 0;
    stbuf->st_gid = file.i_node.i_gid;
    stbuf->st_ino = (size_t) inr;

    if (file.i_node.i_mode & IFDIR) {
        stbuf->st_mode = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH
                | S_IFDIR;
    } else {
        stbuf->st_mode = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH
                | S_IFREG;
    }

    struct timespec mtim = { file.i_node.i_mtime[0], file.i_node.i_mtime[1] };
    stbuf->st_mtim = mtim;
    stbuf->st_nlink = file.i_node.i_nlink;
    stbuf->st_rdev = 0;
    stbuf->st_size = inode_getsize(&file.i_node);
    stbuf->st_uid = file.i_node.i_uid;

    debug_print("fs_getattr 3 %s: %d\n", path, res);

    return res;
}

/**
 * Read a directory
 * @param path
 * @param buf
 * @param filler
 * @param offset
 * @param fi
 * @return
 */
static int fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
        off_t offset, struct fuse_file_info *fi) {
    (void) offset;
    (void) fi;

    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);

    int inr = direntv6_dirlookup(&fs, 1, path);
    if (inr < 0) {
        debug_print("fs_readdir 1: %d\n", inr);
        return inr;
    }

    struct directory_reader reader;
    int ret = direntv6_opendir(&fs, (uint16_t) inr, &reader);
    if (ret < 0) {
        debug_print("fs_readdir 2: %d\n", ret);
        return ret;
    }

    char name[DIRENT_MAXLEN + 1];
    memset(name, 0, DIRENT_MAXLEN + 1);
    uint16_t i = (uint16_t) inr;
    while ((ret = direntv6_readdir(&reader, name, &i)) > 0) {
        filler(buf, name, NULL, 0);
    }

    debug_print("fs_readdir 3: %d\n", 0);

    return 0;
}

/**
 * Read the content of a file
 * @param path IN the path
 * @param buf OUT the buffer where to put the data
 * @param size the nb of bytes to read
 * @param offset from where to read
 * @param fi
 * @return the nb of byte read
 */
static int fs_read(const char *path, char *buf, size_t size, off_t offset,
        struct fuse_file_info *fi) {
    (void) fi;

    int inr = direntv6_dirlookup(&fs, 1, path);
    if (inr < 0) {
        return inr;
    }

    struct filev6 file;
    int ret = filev6_open(&fs, (uint16_t) inr, &file);
    if (ret < 0) {
        return ret;
    }

    unsigned char sector[SECTOR_SIZE + 1];
    sector[SECTOR_SIZE] = '\0';
    int read = 0;
    int total = 0;
    filev6_lseek(&file, (int32_t) offset);
    while ((read = filev6_readblock(&file, sector)) > 0 && total < (int) size) {
        memcpy(buf + total, sector, (size_t) read);
        total += read;
    }

    return total;
}

/* From https://github.com/libfuse/libfuse/wiki/Option-Parsing.
 * This will look up into the args to search for the name of the FS.
 */
static int arg_parse(void *data, const char *filename, int key,
        struct fuse_args *outargs) {
    (void) data;
    (void) outargs;
    if (key == FUSE_OPT_KEY_NONOPT && fs.f == NULL && filename != NULL) {
        if (mountv6(filename, &fs) < 0) {
            debug_print("Unable to mount FS: %50s", filename);
            exit(1);
        }
        debug_print("FS mounted: %50s", filename);
        return 0;
    }
    return 1;
}
