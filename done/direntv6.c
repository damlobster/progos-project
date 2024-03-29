#include "direntv6.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "error.h"
#include "unixv6fs.h"
#include "filev6.h"

#define MAXPATHLEN_UV6 1024 // max size in chars of a path

const char PATH_SEP[2] = { PATH_TOKEN, '\0' };

/**
 * @brief opens a directory reader for the specified inode 'inr'
 * @param u the mounted filesystem
 * @param inr the inode -- which must point to an allocated directory
 * @param d the directory reader (OUT)
 * @return 0 on success; <0 on errror
 */
int direntv6_opendir(const struct unix_filesystem *u, uint16_t inr,
        struct directory_reader *d) {
    M_REQUIRE_NON_NULL(u);
    M_REQUIRE_NON_NULL(d);
    M_REQUIRE_NON_NULL(u->f);

    int error = filev6_open(u, inr, &d->fv6);
    if (error < 0) {
        return error;
    }
    if (!(d->fv6.i_node.i_mode & IFDIR)) {
        // the inode is not a directory
        return ERR_INVALID_DIRECTORY_INODE;
    }
    d->cur = 0;
    d->last = 0;

    return error;
}

/**
 * @brief debugging routine; print the a subtree (note: recursive)
 * @param u a mounted filesystem
 * @param inr the root of the subtree
 * @param prefix the prefix to the subtree
 * @return 0 on success; <0 on error
 */
int direntv6_print_tree(const struct unix_filesystem *u, uint16_t inr,
        const char *prefix) {
    M_REQUIRE_NON_NULL(u);
    M_REQUIRE_NON_NULL(u->f);
    M_REQUIRE_NON_NULL(prefix);

    // open the current directory
    struct directory_reader d;
    int error = direntv6_opendir(u, inr, &d);
    if (error == ERR_INVALID_DIRECTORY_INODE) {
        // it's a file: print it!
        printf("FIL %s\n", prefix);
        return 0;
    } else if (error < 0) {
        return error;
    } else {
        // it's a directory: print it
        printf("DIR %s/\n", prefix);
    }

    //handle the children of the current directory
    char name[DIRENT_MAXLEN + 1];
    memset(name, 0, DIRENT_MAXLEN + 1);
    uint16_t inode_nr;
    struct filev6 fv6;
    while ((error = direntv6_readdir(&d, name, &inode_nr)) > 0) {
        error = filev6_open(u, inode_nr, &fv6);
        if (error < 0) {
            // inode not allocated or other error
            return error;
        }

        // build the new path prefix
        char new_prefix[MAXPATHLEN_UV6];
        memset(new_prefix, 0, MAXPATHLEN_UV6);
        strncat(new_prefix, prefix, MAXPATHLEN_UV6 - 1);
        strncat(new_prefix, PATH_SEP, MAXPATHLEN_UV6 - strlen(new_prefix) - 1);
        strncat(new_prefix, name, MAXPATHLEN_UV6 - strlen(new_prefix) - 1);

        // recurse on this child
        error = direntv6_print_tree(u, inode_nr, new_prefix);
        if(error < 0) {
            return error;
        }
    }

    return error; //==0 if current dir finished
}

/**
 * @brief return the next directory entry.
 * @param d the dierctory reader
 * @param name pointer to at least DIRENTMAX_LEN+1 bytes.  Filled in with the NULL-terminated string of the entry (OUT)
 * @param child_inr pointer to the inode number in the entry (OUT)
 * @return 1 on success;  0 if there are no more entries to read; <0 on error
 */
int direntv6_readdir(struct directory_reader *d, char *name,
        uint16_t *child_inr) {
    M_REQUIRE_NON_NULL(d);
    M_REQUIRE_NON_NULL(name);
    M_REQUIRE_NON_NULL(child_inr);

    if (d->cur == d->last) {
        int read = filev6_readblock(&d->fv6, d->dirs);
        if (read <= 0) {
            // an error occured or nothing to read
            return read;
        }
        d->last = (uint32_t) (d->last
                + (uint32_t) read / sizeof(struct direntv6));
    }

    // get the right directory entry
    struct direntv6 *curdir = &d->dirs[d->cur % DIRENTRIES_PER_SECTOR];
    strncpy(name, curdir->d_name, DIRENT_MAXLEN);
    // and set the child inode number
    *child_inr = curdir->d_inumber;
    // increment the cursor for next call
    d->cur++;

    return 1;
}

/**
 * @brief internal method used by direntv6_dirlookup
 * @param u a mounted filesystem
 * @param inr the root of the subtree
 * @param entry the pathname relative to the subtree
 * @return inr on success; <0 on error
 */
int direntv6_dirlookup_core(const struct unix_filesystem *u, uint16_t inr,
        const char *entry) {

    const char* current = entry;
    const char* next = strchr(entry, PATH_TOKEN);

    // strip multiples '/'
    while (next != NULL && (next - current <= 1)) {
        current += 1;
        next = strchr(current, PATH_TOKEN);
    }

    if (current[0] == '\0' || (current[0] == PATH_TOKEN && current[1] == '\0')) {
        return inr; // empty path return current inr
    }

    struct directory_reader dr;
    int err = direntv6_opendir(u, inr, &dr);
    if (err < 0) {
        return err; // cannot open current dir
    }

    char name[DIRENT_MAXLEN + 1];
    name[DIRENT_MAXLEN] = '\0';
    err = 1;
    size_t len = (next == NULL) ? strlen(current) : (size_t) (next - current);
    while (1 == err) {
        err = direntv6_readdir(&dr, name, &inr);
        if (err < 0) {
            return err;
        }
        if (0 == err) {
            return ERR_INODE_OUTOF_RANGE;
        }
        if (strncmp(name, current, len) == 0 && strlen(name) == len) {
            err = 0;
        }
    }
    //at this point a matching inode was found

    struct filev6 fv6;
    err = filev6_open(u, inr, &fv6);
    if (err < 0) {
        return err;
    }
    if (fv6.i_node.i_mode & IFDIR) {
        // recurce on child dir
        return direntv6_dirlookup_core(u, inr, current + len);
    } else {
        if (current[len] != '\0') return ERR_INODE_OUTOF_RANGE;
    }

    return inr;
}

/**
 * @brief get the inode number for the given path
 * @param u a mounted filesystem
 * @param inr the root of the subtree
 * @param entry the pathname relative to the subtree
 * @return inr on success; <0 on error
 */
int direntv6_dirlookup(const struct unix_filesystem *u, uint16_t inr,
        const char *entry) {
    M_REQUIRE_NON_NULL(u);
    M_REQUIRE_NON_NULL(u->f);
    M_REQUIRE_NON_NULL(entry);

    debug_print("dirlookup: root=%d, path=%s\n", inr, entry);
    return direntv6_dirlookup_core(u, inr, entry);

}

/**
 * @brief create a new direntv6 with the given name and given mode
 * @param u a mounted filesystem
 * @param entry the path of the new entry
 * @param mode the mode of the new inode
 * @return inr on success; <0 on error
 */
int direntv6_create(struct unix_filesystem *u, const char *entry, uint16_t mode) {
    M_REQUIRE_NON_NULL(u);
    M_REQUIRE_NON_NULL(entry);

    if (entry[0] != PATH_TOKEN) {
        // path must start with a slash !
        return ERR_BAD_PARAMETER;
    }

    if (strlen(entry) > MAXPATHLEN_UV6) {
        // path too long !
        return ERR_FILENAME_TOO_LONG;
    }

    char path[MAXPATHLEN_UV6 + 1];
    memset(path, 0, MAXPATHLEN_UV6 + 1);
    char filename[DIRENT_MAXLEN + 1];
    memset(filename, 0, DIRENT_MAXLEN + 1);

    char* last_slash = strrchr(entry, PATH_TOKEN);
    if (strlen(last_slash) == 1) {
        // entry could not end with a slash !
        return ERR_BAD_PARAMETER;
    }

    strncpy(path, entry, (size_t) (last_slash - entry + 1));
    int parent_dir_inr = direntv6_dirlookup(u, ROOT_INUMBER, path);
    if (parent_dir_inr < 0) {
        // the parent directory doesn't exists !
        return ERR_BAD_PARAMETER;
    }

    size_t len = strlen(last_slash) - 1;
    if (len > DIRENT_MAXLEN) {
        // the directory name > DIRENT_MAXLEN !
        return ERR_FILENAME_TOO_LONG;
    }
    strncpy(filename, last_slash + 1, len);

    debug_print("path: %s\n", path);
    debug_print("filename: %s\n", filename);

    if (direntv6_dirlookup(u, (uint16_t) parent_dir_inr, filename) > 0) {
        // the directory/file allready exist !
        return ERR_FILENAME_ALREADY_EXISTS;
    }

    // create the file inode
    struct filev6 file;
    int err = filev6_create(u, mode, &file);
    if (err < 0) {
        return err;
    }

    // update dir entries
    // first read dir inode
    struct filev6 parent;
    err = filev6_open(u, (uint16_t) parent_dir_inr, &parent);
    if (err < 0) {
        return err;
    }

    // then write the new mapping string-inr
    struct direntv6 dirent;
    dirent.d_inumber = (uint16_t) file.i_number;
    strncpy(dirent.d_name, filename, DIRENT_MAXLEN);

    err = filev6_writebytes(u, &parent, &dirent, sizeof(dirent));
    if (err < 0) {
        return err;
    }

    debug_print("DIRENTV6_CREATE: full path = \"%s\"\n", entry);

    return file.i_number;
}

