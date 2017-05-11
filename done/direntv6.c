#include "direntv6.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "error.h"
#include "inode.h"
#include "unixv6fs.h"

#define MAXPATHLEN_UV6 1024 // max size in chars of a path

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

    struct inode inode;
    int error = inode_read(u, inr, &inode);
    if (error < 0) {
        return error;
    }
    if (!(inode.i_mode & IFDIR)) {
        return ERR_INVALID_DIRECTORY_INODE;
    }

    error = filev6_open(u, inr, &d->fv6);
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

    struct directory_reader d;
    int error = direntv6_opendir(u, inr, &d);
    if (error < 0) {
        printf("FIL %s\n", prefix);
        return error;
    } else {
        printf("DIR %s/\n", prefix);
    }

    int ret;
    char name[DIRENT_MAXLEN + 1];
    memset(name, 0, DIRENT_MAXLEN + 1);
    uint16_t inode_nr;
    while ((ret = direntv6_readdir(&d, name, &inode_nr)) > 0) {
        struct inode inode;
        error = inode_read(u, inode_nr, &inode);
        if (error < 0) {
            return error;
        }

        char new_prefix[MAXPATHLEN_UV6];
        memset(new_prefix, 0, MAXPATHLEN_UV6);
        strncat(new_prefix, prefix, MAXPATHLEN_UV6 - 1);
        strncat(new_prefix, "/", MAXPATHLEN_UV6 - strlen(new_prefix) - 1);
        strncat(new_prefix, name, MAXPATHLEN_UV6 - strlen(new_prefix) - 1);

        direntv6_print_tree(u, inode_nr, new_prefix);
    }

    return 0;
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
        if (read <= 0) return read; // an error occured
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
        d->last += read / sizeof (struct direntv6);
#pragma GCC diagnostic pop
    }
    struct direntv6 *curdir = &d->dirs[d->cur % DIRENTRIES_PER_SECTOR];
    strncpy(name, curdir->d_name, DIRENT_MAXLEN);
    *child_inr = curdir->d_inumber;
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
    const char* next = strchr(entry, '/');

    // strip multiples '/'
    while (next != NULL && (next - current <= 1)) {
        current += 1;
        next = strchr(current, '/');
    }

    if (current[0] == '\0' || (current[0] == '/' && current[1] == '\0')) {
        return inr; // empty path return current inr
    }

    struct directory_reader dr;
    int err = direntv6_opendir(u, inr, &dr);
    if (err < 0) return err; // cannot open current dir

    char name[DIRENT_MAXLEN + 1];
    name[DIRENT_MAXLEN] = '\0';
    err = 1;
    size_t len = (next == NULL) ? strlen(current) : (size_t) (next - current);
    while (1 == err) {
        err = direntv6_readdir(&dr, name, &inr);
        if (err < 0) return err;
        if (0 == err) {
            return ERR_INODE_OUTOF_RANGE;
        }
        if (strncmp(name, current, len) == 0 && strlen(name) == len) err = 0;
    }
    //at this point a matching inode was found

    struct inode inode;
    err = inode_read(u, inr, &inode);
    if (err < 0) return err;
    if (inode.i_mode & IFDIR) {
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

int direntv6_create(struct unix_filesystem *u, const char *entry, uint16_t mode) {
    if (direntv6_dirlookup(u, 1, entry) > 0) {
        return ERR_FILENAME_ALREADY_EXISTS;
    }

    if (strlen(entry) > MAXPATHLEN_UV6 || entry[0] != '/') {
        return ERR_BAD_PARAMETER;
    }

    char parent[MAXPATHLEN_UV6 + 1];
    memset(parent, 0, MAXPATHLEN_UV6 + 1);
    char child[DIRENT_MAXLEN + 1];
    memset(child, 0, DIRENT_MAXLEN + 1);

    char* last_slash = strrchr(entry, '/');
    if (last_slash == NULL || strlen(last_slash) == 1) {
        return ERR_BAD_PARAMETER;
    }

    strncpy(parent, entry, last_slash - entry + 1);
    if (direntv6_dirlookup(u, 1, parent) < 0) {
        return ERR_BAD_PARAMETER;
    }

    if (strlen(child) > DIRENT_MAXLEN) {
        return ERR_FILENAME_TOO_LONG;
    }
    strncpy(child, last_slash + 1, strlen(last_slash) - 1);

    debug_print("parent: %s\n", parent);
    debug_print("child: %s\n", child);

    int inr = inode_alloc(u);
    if (inr < 0) {
        return inr;
    }

    struct inode inode = {0};
    inode.i_mode = mode | IALLOC;

    int err = inode_write(u, (uint16_t) inr, &inode);
    if (err < 0) {
        return err;
    }

    struct filev6 fv6;
    fv6.i_node = inode;
    fv6.i_number = (uint16_t) inr;
    fv6.offset = 0;
    fv6.u = u;

    err = filev6_writebytes(u, &fv6, NULL, 0);
    if (err < 0) {
        return err;
    }

    return 0;
}

