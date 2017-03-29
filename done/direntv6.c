#include "direntv6.h"
#include <string.h>
#include "error.h"

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
        d->last += read / sizeof(struct direntv6);
    }
    struct direntv6 *curdir = &d->dirs[d->cur % DIRENTRIES_PER_SECTOR];
    strncpy(name, curdir->d_name, DIRENT_MAXLEN);
    child_inr = &curdir->d_inumber;
    d->cur++;

    return 1;
}
