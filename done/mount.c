/*
 * mount.c
 *
 *  Created on: Mar 15, 2017
 *  Authors: Léonard Berney & Damien Martin
 */
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>

#include "error.h"
#include "mount.h"
#include "sector.h"
#include "unixv6fs.h"
#include "inode.h"

/**
 * Fill the inodes bitmap
 * @param u the opened file system, not NULL
 */
void fill_ibm(struct unix_filesystem *u);

/**
 * Fill the sectors bitmap
 * @param u the opened file system, not NULL
 */
void fill_fbm(struct unix_filesystem *u);

/**
 * @brief  mount a unix v6 filesystem
 * @param filename name of the unixv6 filesystem on the underlying disk (IN)
 * @param u the filesystem (OUT)
 * @return 0 on success; <0 on error
 */
int mountv6(const char *filename, struct unix_filesystem *u) {
    M_REQUIRE_NON_NULL(filename);
    M_REQUIRE_NON_NULL(u);

    memset(u, 0, sizeof(*u));

    u->f = fopen(filename, "r"); //FIXME rw?
    if (u->f == NULL) {
        return ERR_IO;
    }

    uint8_t bootblock[SECTOR_SIZE];
    int error = sector_read(u->f, BOOTBLOCK_SECTOR, bootblock);
    if (error != 0) {
        return error;
    }

    if (bootblock[BOOTBLOCK_MAGIC_NUM_OFFSET] != BOOTBLOCK_MAGIC_NUM) {
        return ERR_BADBOOTSECTOR;
    }

    error = sector_read(u->f, SUPERBLOCK_SECTOR, &u->s);
    if (error != 0) {
        return error;
    }

    u->ibm = bm_alloc(2, (uint64_t) u->s.s_isize * INODES_PER_SECTOR);
    u->fbm = bm_alloc((uint64_t) u->s.s_block_start + 1,
            (uint64_t) u->s.s_fsize);
    fill_ibm(u);
    fill_fbm(u);

    return 0;
}

/**
 * @brief print to stdout the content of the superblock
 * @param u - the mounted filesytem
 */
void mountv6_print_superblock(const struct unix_filesystem *u) {
    if (u == NULL) {
        puts("NULL");
    } else {
        puts("**********FS SUPERBLOCK START**********");
        printf("s_isize             : %" PRIu16 "\n", u->s.s_isize);
        printf("s_fsize             : %" PRIu16 "\n", u->s.s_fsize);
        printf("s_fbmsize           : %" PRIu16 "\n", u->s.s_fbmsize);
        printf("s_ibmsize           : %" PRIu16 "\n", u->s.s_ibmsize);
        printf("s_inode_start       : %" PRIu16 "\n", u->s.s_inode_start);
        printf("s_block_start       : %" PRIu16 "\n", u->s.s_block_start);
        printf("s_fbm_start         : %" PRIu16 "\n", u->s.s_fbm_start);
        printf("s_ibm_start         : %" PRIu16 "\n", u->s.s_ibm_start);
        printf("s_flock             : %" PRIu8 "\n", u->s.s_flock);
        printf("s_ilock             : %" PRIu8 "\n", u->s.s_ilock);
        printf("s_fmod              : %" PRIu8 "\n", u->s.s_fmod);
        printf("s_ronly             : %" PRIu8 "\n", u->s.s_ronly);
        printf("s_time              : [0] %" PRIu16 "\n", u->s.s_time[0]);
        puts("**********FS SUPERBLOCK END**********");
    }
}

/**
 * @brief umount the given filesystem
 * @param u - the mounted filesytem
 * @return 0 on success; <0 on error
 */
int umountv6(struct unix_filesystem * u) {
    M_REQUIRE_NON_NULL(u);

    free(u->fbm);
    free(u->ibm);

    int error = fclose(u->f);
    if (error != 0) {
        return ERR_IO;
    }

    return 0;
}

void fill_ibm(struct unix_filesystem *u) {
    if (u == NULL) {
        return;
    }
    // loop on all inode sectors
    for (uint32_t k = 0; k < u->s.s_isize; k++) {
        struct inode inodes[INODES_PER_SECTOR];
        int error = sector_read(u->f, u->s.s_inode_start + k, inodes);

        // loop on all inode of the current sector
        for (unsigned int i = 0; i < INODES_PER_SECTOR; i++) {
            if (error != 0) {
                bm_set(u->ibm, k * INODES_PER_SECTOR + i); //FIXME pas sûr
            } else if (inodes[i].i_mode & IALLOC) {
                bm_set(u->ibm, k * INODES_PER_SECTOR + i);
            }
        }
    }
}

void fill_fbm(struct unix_filesystem *u) {
    if (u == NULL) {
        return;
    }

    for (uint16_t i = 0; i < u->s.s_isize * INODES_PER_SECTOR; i++) {
        struct inode inode;
        int ret = inode_read(u, i, &inode);
        if (ret < 0) {
            continue;
        }
        int32_t isize = inode_getsize(&inode);
        if (isize > INODE_SMALL_FILE && isize <= INODE_EXTRA_LARGE_FILE) {
            for (int j = 0; j <= isize / (SECTOR_SIZE * ADDRESSES_PER_SECTOR);
                    j++) {
                debug_print("INDIRECT SECTOR %d USED\n", inode.i_addr[j]);
                bm_set(u->fbm, inode.i_addr[j]);
            }
        }

        int sector;
        int offset = 0;
        while ((sector = inode_findsector(u, &inode, offset)) > 0) {
            bm_set(u->fbm, (uint64_t) sector);
            offset++;
        }
    }
}
