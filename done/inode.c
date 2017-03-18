/*
 * inode.c
 *
 *  Created on: Mar 15, 2017
 *  Authors: Léonard Berney & Damien Martin
 */

#include <inttypes.h>
#include <string.h>
#include "inode.h"
#include "error.h"
#include "sector.h"

/**
 * @brief read all inodes from disk and print out their content to
 *        stdout according to the assignment
 * @param u the filesystem
 * @return 0 on success; < 0 on error.
 */
int inode_scan_print(const struct unix_filesystem *u) {
    M_REQUIRE_NON_NULL(u);

    // loop on all inode sectors
    for (int k = u->s.s_inode_start; k < u->s.s_isize; k++) {

        struct inode inodes[INODES_PER_SECTOR];
        int r = sector_read(u->f, k, inodes);
        if (r != 0) return r; // propagate sector_read error code if any

        // loop on all inode of the current sector
        for (unsigned int i = 0; i < INODES_PER_SECTOR; i++) {

            if (inodes[i].i_mode & IALLOC) {
                // inode is allocated --> print it
                int size = inode_getsize(&inodes[i]);
                char *type = (inodes[i].i_mode & IFDIR) ? SHORT_DIR_NAME : SHORT_FIL_NAME;
                printf("inode %3d (%s) len %4d\n", i, type, size);
            }
        }
    }

    return 0; // print successful
}

/**
 * @brief prints the content of an inode structure
 * @param inode the inode structure to be displayed
 */
void inode_print(const struct inode *inode) {
    if (inode == NULL) {
        puts("NULL");
    } else {
        puts("**********FS INODE START**********");
        printf("i_mode: %" PRIu16 "\n", inode->i_mode);
        printf("i_nlink: %" PRIu8 "\n", inode->i_nlink);
        printf("i_uid: %" PRIu8 "\n", inode->i_uid);
        printf("i_gid: %" PRIu8 "\n", inode->i_gid);
        printf("i_size0: %" PRIu8 "\n", inode->i_size0);
        printf("i_size1: %" PRIu16 "\n", inode->i_size1);
        printf("size: %" PRIu32 "\n", inode_getsize(inode));
        puts("**********FS INODE END**********");
    }
}

/**
 * @brief read the content of an inode from disk
 * @param u the filesystem (IN)
 * @param inr the inode number of the inode to read (IN)
 * @param inode the inode structure, read from disk (OUT)
 * @return 0 on success; <0 on error
 */
int inode_read(const struct unix_filesystem *u, uint16_t inr, struct inode *inode) {
    M_REQUIRE_NON_NULL(u);

    uint16_t sector_to_read = u->s.s_inode_start;
    sector_to_read += inr / INODES_PER_SECTOR;
    if (sector_to_read < 0 || sector_to_read > u->s.s_isize) {
        return ERR_INODE_OUTOF_RANGE;
    }

    struct inode inodes[INODES_PER_SECTOR];
    int r = sector_read(u->f, sector_to_read, inodes);
    if (r != 0) return r; // propagate sector_read error code if any

    uint8_t index = inr % INODES_PER_SECTOR;
    if (!(inodes[index].i_mode & IALLOC)) {
        return ERR_UNALLOCATED_INODE;
    }

    *inode = inodes[index];
    return 0;
}

