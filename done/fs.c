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
#include "mount.h"
#include "inode.h"
#include "unixv6fs.h"
#include "direntv6.h"

struct unix_filesystem fs;

static int fs_getattr(const char *path, struct stat *stbuf) {
    int res = 0;

    memset(stbuf, 0, sizeof (struct stat));

    int inr = direntv6_dirlookup(&fs, 1, path);
    if (inr < 0) {
        return inr;
    }

    struct inode inode;
    int ret = inode_read(&fs, inr, &inode);
    if (ret < 0) {
        return ret;
    }

    stbuf->st_atim = inode.i_atime;
    stbuf->st_blksize = SECTOR_SIZE;
    stbuf->st_blocks = inode_getsectorsize(&inode);
    stbuf->st_ctim = NULL;
    stbuf->st_dev = NULL;
    stbuf->st_gid = inode.i_gid;
    stbuf->st_ino = inr;

    if (inode.i_mode & IFDIR) {
        stbuf->st_mode = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH | S_IFDIR;
    } else {
        stbuf->st_mode = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH | S_IFREG;
    }

    stbuf->st_mtim = inode.i_mtime;
    stbuf->st_nlink = inode.i_nlink;
    stbuf->st_rdev = NULL;
    stbuf->st_size = inode_getsize(&inode);
    stbuf->st_uid = inode.i_uid;

    return res;
}

static int fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    (void) offset;
    (void) fi;

    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);

    int inr = direntv6_dirlookup(&fs, 1, path);
    if (inr < 0) {
        return inr;
    }

    struct directory_reader reader;
    int ret = direntv6_opendir(&fs, inr, &reader);
    if (ret < 0) {
        return ret;
    }

    char name[DIRENT_MAXLEN + 1];
    memset(name, 0, DIRENT_MAXLEN + 1);
    while ((ret = direntv6_readdir(&reader, name, &inr)) > 0) {
        filler(buf, name, NULL, 0);
    }

    return 0;
}

static int fs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    size_t len;
    (void) fi;

    int inr = direntv6_dirlookup(&fs, 1, path);
    if (inr < 0) {
        return inr;
    }

    struct inode in;
    int ret = inode_read(&fs, (uint16_t) inr, &in);
    if (ret < 0) {
        return ret;
    }

    struct filev6 fv6;
    ret = filev6_open(&fs, (uint16_t) inr, &fv6);
    if (ret < 0) {
        return ret;
    }
    
    unsigned char sector[SECTOR_SIZE + 1];
    sector[SECTOR_SIZE] = '\0';
    int read = 0;
    int total = 0;
    filev6_lseek(&fv6, offset);
    while ((read = filev6_readblock(&fv6, sector)) > 0 && total < size) {
        memcpy(buf + total, sector, read);
        total += read;
    }

    return total;
}

/* From https://github.com/libfuse/libfuse/wiki/Option-Parsing.
 * This will look up into the args to search for the name of the FS.
 */
static int arg_parse(void *data, const char *filename, int key, struct fuse_args *outargs) {
    (void) data;
    (void) outargs;
    if (key == FUSE_OPT_KEY_NONOPT && fs.f == NULL && filename != NULL) {
        if (mountv6(filename, &fs) < 0) {
            exit(1);
        }
        return 0;
    }
    return 1;
}

static struct fuse_operations available_ops = {.getattr = fs_getattr,
    .readdir = fs_readdir, .read = fs_read,};

int main(int argc, char *argv[]) {
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    int ret = fuse_opt_parse(&args, NULL, NULL, arg_parse);
    if (ret == 0) {
        ret = fuse_main(args.argc, args.argv, &available_ops, NULL);
        (void) umountv6(&fs);
    }
    return ret;
}
