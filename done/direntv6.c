#include <stdint.h>
#include <string.h>

#include "direntv6.h"
#include "error.h"
#include "unixv6fs.h"
#include "inode.h"

#define MAXPATHLEN_UV6 1024 // FIXME delete when found

int direntv6_opendir(const struct unix_filesystem *u, uint16_t inr, struct directory_reader *d) {
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

int direntv6_print_tree(const struct unix_filesystem *u, uint16_t inr, const char *prefix) {
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
        if (read <= 0) return read;
        d->last += read / sizeof (struct direntv6);
    }
    struct direntv6 *curdir = &d->dirs[d->cur % DIRENTRIES_PER_SECTOR];
    strncpy(name, curdir->d_name, DIRENT_MAXLEN);
    *child_inr = curdir->d_inumber;
    d->cur++;

    return 1;
}
