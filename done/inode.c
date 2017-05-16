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

#define SMALL_FILE_SIZE 8 * SECTOR_SIZE
#define EXTRA_LARGE_FILE_SIZE 7 * ADDRESSES_PER_SECTOR * SECTOR_SIZE

#define M_INODE_GET_SECTOR_ADDR(u, inr) (u->s.s_inode_start + (uint32_t)(inr / INODES_PER_SECTOR))
#define M_INODE_GET_INDEX_IN_SECTOR(inr) (inr % INODES_PER_SECTOR)

/**
 * Read all inodes that are part of the same sector containing inode nb 'inr'
 * @param u IN the filesystem
 * @param inr IN the inode number
 * @param inodes OUT the array where to write the indoes
 * @return <0 in case of error, 0 otherwise
 */
int inode_read_many(const struct unix_filesystem *u, uint16_t inr,
        struct inode *inodes);

/**
 * @brief read all inodes from disk and print out their content to
 *        stdout according to the assignment
 * @param u the filesystem
 * @return 0 on success; < 0 on error.
 */
int inode_scan_print(const struct unix_filesystem *u) {
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
                const char *type = (inodes[i].i_mode & IFDIR) ?
                SHORT_DIR_NAME :
                                                                SHORT_FIL_NAME;
                printf("inode %3zu (%s) len %4d\n",
                        i + (k - 2) * INODES_PER_SECTOR, type, size);
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

    struct inode inodes[INODES_PER_SECTOR];
    int err = inode_read_many(u, inr, inodes);
    if (err != 0) {
        return err;
    }

    uint8_t index = M_INODE_GET_INDEX_IN_SECTOR(inr);
    if (!(inodes[index].i_mode & IALLOC)) {
        return ERR_UNALLOCATED_INODE;
    }

    *inode = inodes[index];
    debug_print("INODE %d ***********************************\n", inr);
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
        debug_print("UNNALLOCATED INODE\n%c", ' ');
        return ERR_UNALLOCATED_INODE;
    }
    int32_t isize = inode_getsize(i);
    if (file_sec_off * SECTOR_SIZE >= isize) {
        debug_print("ERR_OFFSET_OUT_OF_RANGE: offset %d\n", file_sec_off);
        return ERR_OFFSET_OUT_OF_RANGE;
    }

    if (isize <= SMALL_FILE_SIZE) {
        //direct addressing
        //debug_print("has sector %d\n", i->i_addr[file_sec_off]);
        return i->i_addr[file_sec_off];
    } else if (isize <= EXTRA_LARGE_FILE_SIZE) {
        // indirect addressing
        int16_t addrs[ADDRESSES_PER_SECTOR];
        int err = sector_read(u->f,
                i->i_addr[file_sec_off / ADDRESSES_PER_SECTOR], addrs);
        if (err != 0) {
            return err;
        }
        /*debug_print("inode has sector %d\n",
                addrs[file_sec_off % ADDRESSES_PER_SECTOR]);*/
        return addrs[file_sec_off % ADDRESSES_PER_SECTOR];
    } else {
        // extra-large file, not handled
        debug_print("ERR_FILE_TO_LARGE\n%c", ' ');
        return ERR_FILE_TOO_LARGE;
    }
}

/**
 * @brief write the content of an inode to disk
 * @param u the filesystem (IN)
 * @param inr the inode number of the inode to read (IN)
 * @param inode the inode structure, read from disk (IN)
 * @return 0 on success; <0 on error
 */
int inode_write(struct unix_filesystem *u, uint16_t inr,
        const struct inode *inode) {
    M_REQUIRE_NON_NULL(u);
    M_REQUIRE_NON_NULL(inode);

    struct inode inodes[INODES_PER_SECTOR];
    int err = inode_read_many(u, inr, inodes);
    if (err != 0) {
        return err;
    }

    inodes[M_INODE_GET_INDEX_IN_SECTOR(inr)] = *inode;

    err = sector_write(u->f, M_INODE_GET_SECTOR_ADDR(u, inr), inodes);
    return err;
}

/**
 * @brief read the content of an inode from disk
 * @param u the filesystem (IN)
 * @param inr the inode number of the inode to read (IN)
 * @param inode the inode structure, read from disk (OUT)
 * @return 0 on success; <0 on error
 */
int inode_read_many(const struct unix_filesystem *u, uint16_t inr,
        struct inode *inodes) {
    // local function, don't check args validity

    long unsigned int sector_to_read = M_INODE_GET_SECTOR_ADDR(u, inr);
    if (sector_to_read > u->s.s_isize) {
        return ERR_INODE_OUTOF_RANGE;
    }

    int error = sector_read(u->f, (uint16_t) sector_to_read, inodes);
    return error; // propagate sector_read error
}

/**
 * @brief alloc a new inode (returns its inr if possible)
 * @param u the filesystem (IN)
 * @return the inode number of the new inode or error code on error
 */
int inode_alloc(struct unix_filesystem* u) {
    M_REQUIRE_NON_NULL(u);
    int inr = bm_find_next(u->ibm);
    if (inr < 0) {
        return ERR_NOMEM;
    }
    bm_set(u->ibm, (uint64_t) inr);

    return inr;
}

/**
 * @brief set the size of a given inode to the given size
 * @param inode the inode
 * @param new_size the new size
 * @return 0 on success; <0 on error
 */
int inode_setsize(struct inode *inode, int new_size) {
    M_REQUIRE_NON_NULL(inode);

    if (new_size < 0) {
        return ERR_NOMEM;
    }
    if (new_size > EXTRA_LARGE_FILE_SIZE) {
        return ERR_FILE_TOO_LARGE;
    }

    inode->i_size1 = (uint16_t)(new_size & UINT16_MAX);
    inode->i_size0 = (uint8_t)((new_size >> 16) & UINT8_MAX);

    return 0;
}
