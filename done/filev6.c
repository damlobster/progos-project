/*
 * filev6.c
 *
 *  Created on: Mar 15, 2017
 *  Authors: Léonard Berney & Damien Martin
 */
#include <string.h>

#include "filev6.h"
#include "error.h"
#include "inode.h"
#include "sector.h"
#include "unixv6fs.h"

/**
 * @brief open up a file corresponding to a given inode; set offset to zero
 * @param u the filesystem (IN)
 * @param inr he inode number (IN)
 * @param fv6 the complete filve6 data structure (OUT)
 * @return 0 on success; <0 on errror
 */
int filev6_open(const struct unix_filesystem *u, uint16_t inr,
        struct filev6 *fv6) {
    M_REQUIRE_NON_NULL(u);
    M_REQUIRE_NON_NULL(fv6);

    fv6->u = u;
    fv6->i_number = inr;
    int error = inode_read(u, inr, &fv6->i_node);
    fv6->offset = 0;

    return error;
}

/**
 * @brief read at most SECTOR_SIZE from the file at the current cursor
 * @param fv6 the filev6 (IN-OUT; offset will be changed)
 * @param buf points to SECTOR_SIZE bytes of available memory (OUT)
 * @return >0: the number of bytes of the file read; 0: end of file; <0 error
 */
int filev6_readblock(struct filev6 *fv6, void *buf) {
    M_REQUIRE_NON_NULL(fv6);
    M_REQUIRE_NON_NULL(fv6->u);
    M_REQUIRE_NON_NULL(buf);

    if (fv6->offset >= inode_getsize(&fv6->i_node)) {
        return 0;
    }

    int sector = inode_findsector(fv6->u, &fv6->i_node,
            fv6->offset / SECTOR_SIZE);
    if (sector <= 0) {
        return sector; // inode not allocated or error
    }

    int error = sector_read(fv6->u->f, (uint32_t) sector, buf);
    int last_sector_size = inode_getsize(&fv6->i_node) % SECTOR_SIZE;
    if (last_sector_size == 0) {
        last_sector_size = SECTOR_SIZE;
    }

    int read;
    if (fv6->offset == inode_getsize(&fv6->i_node) - last_sector_size) {
        read = last_sector_size;
    } else {
        read = SECTOR_SIZE;
    }

    fv6->offset += SECTOR_SIZE; //FIXME increment only of size last_sector_size for the last block

    return error < 0 ? error : read;
}

/**
 * @brief change the current offset of the given file to the one specified
 * @param fv6 the filev6 (IN-OUT; offset will be changed)
 * @param off the new offset of the file
 * @return 0 on success; <0 on errror
 */
int filev6_lseek(struct filev6 *fv6, int32_t offset) {
    M_REQUIRE_NON_NULL(fv6);
    int32_t size = inode_getsize(&fv6->i_node);
    if (offset > size) {
        return ERR_OFFSET_OUT_OF_RANGE;
    }
    fv6->offset = offset;
    return 0;

}

/**
 * @brief create a new filev6
 * @param u the filesystem (IN)
 * @param mode the new offset of the file
 * @param fv6 the filev6 (IN-OUT; offset will be changed)
 * @return 0 on success; <0 on errror
 */
int filev6_create(struct unix_filesystem *u, uint16_t mode, struct filev6 *fv6) {
    M_REQUIRE_FS_MOUNTED(u);
    M_REQUIRE_NON_NULL(fv6);

    if (u->s.s_inode_start > fv6->i_number) {
        return ERR_INODE_OUTOF_RANGE;
    }

    fv6->i_node.i_mode = mode; //FIXME validate modes !?
    fv6->i_node.i_nlink = 0;
    fv6->i_node.i_uid = 0;
    fv6->i_node.i_gid = 0;
    fv6->i_node.i_size0 = 0;
    fv6->i_node.i_size1 = 0;
    memset(fv6->i_node.i_addr, 0, ADDR_SMALL_LENGTH * sizeof(fv6->i_node.i_addr[0]));
    memset(fv6->i_node.i_atime, 0, 2 * sizeof(fv6->i_node.i_atime[0]));
    memset(fv6->i_node.i_mtime, 0, 2 * sizeof(fv6->i_node.i_mtime[0]));

    int err = inode_write(u, fv6->i_number, &fv6->i_node);
    if(err!=0){
        return err;
    }

    return 0;


}
