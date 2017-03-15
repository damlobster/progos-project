/*
 * sector.c
 *
 *  Created on: Mar 13, 2017
 *      Author: Damien & Léonard
 */

#include "sector.h"

#include "error.h"
#include "unixv6fs.h"

/**
 * @brief read one 512-byte sector from the virtual disk
 * @param f open file of the virtual disk
 * @param sector the location (in sector units, not bytes) within the virtual disk
 * @param data a pointer to 512-bytes of memory (OUT)
 * @return 0 on success; <0 on error
 */
int sector_read(FILE *f, uint32_t sector, void *data) {

    M_REQUIRE_NON_NULL(f);
    M_REQUIRE_NON_NULL(data);
    //if (0 == sector || sector > 64*512-1) // TODO nécessaire ???
    //	return ERR_BAD_PARAMETER;

    if (fseek(f, sector * SECTOR_SIZE, SEEK_SET) != 0) {
        return ERR_IO;
    }

    if (fread(data, sizeof (uint8_t), SECTOR_SIZE, f) != SECTOR_SIZE) {
        return ERR_IO;
    }

    return 0;
}

