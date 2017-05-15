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

#define SMALL_FILE_SIZE 8 * SECTOR_SIZE
#define EXTRA_LARGE_FILE_SIZE 7 * ADDRESSES_PER_SECTOR * SECTOR_SIZE

#define M_REQUIRE_FS_MOUNTED(u) \
    do{ \
        M_REQUIRE_NON_NULL(u); \
        M_REQUIRE_NON_NULL(u->f); \
    } while(0)

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
        // already at the end of file --> read nothing !
        return 0;
    }

    int sector = inode_findsector(fv6->u, &fv6->i_node,
            fv6->offset / SECTOR_SIZE);
    if (sector <= 0) {
        return sector; // inode not allocated or error
    }

    // read the sector
    int error = sector_read(fv6->u->f, (uint32_t) sector, buf);
    if (error < 0) {
        return error;
    }

    // caclulate the bytes read
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

    fv6->offset += read;

    return read;
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
        // the inode number is out of range
        return ERR_INODE_OUTOF_RANGE;
    }

    fv6->i_node.i_mode = mode; //FIXME ajouter | IALLOC ? création d'un fichier seulement? validation?
    fv6->i_node.i_nlink = 0;
    fv6->i_node.i_uid = 0;
    fv6->i_node.i_gid = 0;
    fv6->i_node.i_size0 = 0;
    fv6->i_node.i_size1 = 0;
    memset(fv6->i_node.i_addr, 0,
    ADDR_SMALL_LENGTH * sizeof(fv6->i_node.i_addr[0]));
    memset(fv6->i_node.i_atime, 0, 2 * sizeof(fv6->i_node.i_atime[0]));
    memset(fv6->i_node.i_mtime, 0, 2 * sizeof(fv6->i_node.i_mtime[0]));

    // write the inode to disk
    int err = inode_write(u, fv6->i_number, &fv6->i_node);
    if (err != 0) {
        return err;
    }

    return 0;
}

/**
 * @brief write a sector new sector or fill an existing one
 * @param u the filesystem
 * @param fv6 the file
 * @param buf the data to write
 * @param len the number of bytes in buf
 * @return <0 in case of error, otherwise the number of bytes written
 */
int filev6_writesector(struct unix_filesystem *u, struct filev6 *fv6,
        const void *buf, uint32_t len);

/**
 * @brief write the len bytes of the given buffer on disk to the given filev6
 * @param u the filesystem (IN)
 * @param fv6 the filev6 (IN)
 * @param buf the data we want to write (IN)
 * @param len the length of the bytes we want to write
 * @return 0 on success; <0 on errror
 */
int filev6_writebytes(struct unix_filesystem *u, struct filev6 *fv6,
        const void *buf, int len) {
    M_REQUIRE_FS_MOUNTED(u);
    M_REQUIRE_NON_NULL(fv6);
    M_REQUIRE_NON_NULL(buf);

    int32_t file_size = inode_getsize(&fv6->i_node);
    if (file_size + len > EXTRA_LARGE_FILE_SIZE) {
        return ERR_FILE_TOO_LARGE;
    }
    fv6->offset = file_size;

    int remain = len;
    const char* b = buf;
    while (remain > 0) {
        int written_bytes = filev6_writesector(u, fv6, b, (uint32_t) remain);
        if (written_bytes < 0) {
            return written_bytes;
        }
        b += written_bytes;
        remain -= written_bytes;
    }

    inode_setsize(&fv6->i_node, fv6->offset);

    int err = inode_write(u, fv6->i_number, &fv6->i_node);
    if (err < 0) {
        return err;
    }
    return len;
}

int filev6_writesector(struct unix_filesystem *u, struct filev6 *fv6,
        const void *buf, uint32_t len) {
    // internal method: no check of arguments validity

    uint32_t write_from = (uint32_t) fv6->offset % SECTOR_SIZE;
    uint32_t bytes_to_write = SECTOR_SIZE - write_from;
    if (len < bytes_to_write) {
        bytes_to_write = len;
    }

    unsigned char sec_buf[SECTOR_SIZE];
    int sec_nb = 0;
    if (write_from > 0) {
        // the last sector is not full, fill it
        sec_nb = inode_findsector(u, &fv6->i_node, fv6->offset / SECTOR_SIZE);
        if (sec_nb < 0) {
            return sec_nb;
        }
        int read = sector_read(u->f, (uint32_t) sec_nb, sec_buf);
        if (read < 0) {
            return read;
        }
    } else {
        // allocate a new sector
        sec_nb = bm_find_next(u->fbm);
        if (sec_nb < 0) {
            return ERR_NOMEM;
        }

        // FIXME handle here the large files: go to indirect addressing
        fv6->i_node.i_addr[inode_getsize(&fv6->i_node) % SECTOR_SIZE + 1] =
                (uint16_t) sec_nb;
    }

    memcpy(&sec_buf[write_from], buf, bytes_to_write);

    int err = sector_write(u->f, (uint32_t) sec_nb, sec_buf);
    if (err < 0) {
        return err;
    }

    fv6->offset += (int32_t) bytes_to_write;

    // set (maybe again) that this sector is used
    bm_set(u->ibm, (uint64_t) sec_nb);
    return (int) bytes_to_write;
}
