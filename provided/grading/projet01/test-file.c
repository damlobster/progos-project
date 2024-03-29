/*
 * test-file.c
 *
 *  Created on: Mar 20, 2017
 *      Author: damien
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "error.h"
#include "filev6.h"
#include "inode.h"
#include "mount.h"
#include "sector.h"
#include "sha.h"
#include "unixv6fs.h"

int test(const struct unix_filesystem *u) {
    M_REQUIRE_NON_NULL(u);

    struct filev6 fs;
    memset(&fs, 255, sizeof(fs));

    struct inode in;

    // print inode 3 and 5
    for (int i_number = 3; i_number <= 5; i_number += 2) {
        putchar('\n');

        int err = inode_read(u, (uint16_t)i_number, &in);
        if (err != 0) {
            printf("filev6 open failed for inode #%d.\n", i_number);
        } else {
            printf("Printing inode #%d:\n", i_number);
            inode_print(&in);

            if ((in.i_mode & IFDIR) != 0) {
                puts("which is a directory.");
            } else {
                puts("the first sector of data which contains:");
                char data[SECTOR_SIZE + 1];
                data[SECTOR_SIZE] = '\0';

                int sector = inode_findsector(u, &in, 0);
                if(sector<0) return sector;

                sector_read(u->f, (uint32_t)sector, data);
                printf("%s\n", data);
                puts("----");
            }
        }
    }

    puts("\nListing inodes SHA:");
    for (uint16_t i = 1; i <= u->s.s_isize * INODES_PER_SECTOR; ++i) {
        if (inode_read(u, i, &in) == 0) print_sha_inode(u, in, i);
    }
    return 0;
}
