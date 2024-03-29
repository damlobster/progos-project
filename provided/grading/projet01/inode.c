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
int inode_scan_print(const struct unix_filesystem *u) {//correcteur : see comments in week8 file for this one (except comments in findsector here) and the test files (files are almost identical so comments stand for both weeks)
    M_REQUIRE_NON_NULL(u);

    // loop on all inode sectors
    for (uint32_t k = u->s.s_inode_start; k < u->s.s_isize; k++) {

        struct inode inodes[INODES_PER_SECTOR];
        int error = sector_read(u->f, k, inodes);
        if (error != 0) return error; // propagate sector_read error code if any

        // loop on all inode of the current sector
        for (unsigned int i = 0; i < INODES_PER_SECTOR; i++) {

            if (inodes[i].i_mode & IALLOC) {
                // inode is allocated --> print it
                int size = inode_getsize(&inodes[i]);
                const char *type =
                        (inodes[i].i_mode & IFDIR) ?
                                SHORT_DIR_NAME : SHORT_FIL_NAME;
                printf("inode %3zu (%s) len %4d\n", i+(k-2)*INODES_PER_SECTOR, type, size);
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
int inode_read(const struct unix_filesystem *u, uint16_t inr,
        struct inode *inode) {
    M_REQUIRE_NON_NULL(u);
    M_REQUIRE_NON_NULL(inode);

    long unsigned int sector_to_read = u->s.s_inode_start;
    sector_to_read += inr / INODES_PER_SECTOR;
    if (sector_to_read > u->s.s_isize) {
        return ERR_INODE_OUTOF_RANGE;
    }

    struct inode inodes[INODES_PER_SECTOR];
    int error = sector_read(u->f, (uint16_t)sector_to_read, inodes);
    if (error != 0) return error; // propagate sector_read error

    uint8_t index = inr % INODES_PER_SECTOR;
    if (!(inodes[index].i_mode & IALLOC)) {
        return ERR_UNALLOCATED_INODE;
    }

    *inode = inodes[index];
    return 0;
}

/**
 * @brief identify the sector that corresponds to a given portion of a file
 * @param u the filesystem (IN)
 * @param inode the inode (IN)
 * @param file_sec_off the offset within the file (in sector-size units)
 * @return >0: the sector on disk;  0: unallocated;  <0 error
 */
int inode_findsector(const struct unix_filesystem *u, const struct inode *i,
        int32_t file_sec_off) {
    M_REQUIRE_NON_NULL(u);
    M_REQUIRE_NON_NULL(i);
    M_REQUIRE_NON_NULL(i->i_addr);
    if (!(i->i_mode & IALLOC)) {
        return ERR_UNALLOCATED_INODE;
    }

    int32_t size_in_sec = (inode_getsize(i) / SECTOR_SIZE);

    if (file_sec_off > size_in_sec) {
        return ERR_OFFSET_OUT_OF_RANGE;
    } else if (size_in_sec <= 8) { //correcteur : avoid hard values and comment more
        //direct addressing
        return i->i_addr[file_sec_off];//correcteur : test error if small space to be read over offset, you corrected it in week8 so well done!
    } else if (size_in_sec < 7 * ADDRESSES_PER_SECTOR) {
        // indirect addressing
        int16_t addrs[ADDRESSES_PER_SECTOR];
        int err = sector_read(u->f,
                i->i_addr[file_sec_off / ADDRESSES_PER_SECTOR], addrs);
        if (err != 0) {
            return err;
        }
        return addrs[file_sec_off % ADDRESSES_PER_SECTOR];
    } else {
        // extra-large file, not handled
        return ERR_FILE_TOO_LARGE;
    }
}

