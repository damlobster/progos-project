#include <stdint.h>
#include <string.h>

#include "direntv6.h"
#include "error.h"
#include "unixv6fs.h"
#include "inode.h"

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
        return error;
    }

    int ret;
    char name[DIRENT_MAXLEN + 1];
    memset(name, NULL, DIRENT_MAXLEN + 1);
    uint16_t inode_nr;
    while ((ret = direntv6_readdir(d, name, &inode_nr)) > 0) {
        struct inode inode;
        error = inode_read(u, inode_nr, &inode);
        if (error < 0) {
            return error;
        }
        
        if (inode.i_mode & IFDIR) {
            printf("DIR ");
        } else {
            printf("FIL ");
        }
        char new_prefix[MAXPATHLEN_UV6];
        memset(new_prefix, NULL, MAXPATHLEN_UV6);
        strncat(new_prefix, prefix, MAXPATHLEN_UV6 - 1);
        strncat(new_prefix, "/", strlen(new_prefix) -1)
        strncat(new_prefix, name, strlen(new_prefix) -1);
        printf("%s\n", new_prefix);
        
        direntv6_print_tree(u, inode_nr, new_prefix);
    }

    return 0;
}
