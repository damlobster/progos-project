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
#include "sha.h"
#include "unixv6fs.h"

void _print_inode(const struct unix_filesystem *u, uint16_t inr);

/**
 * Tests for filev6.c
 * @param u the file system
 * @return <0 if error occured
 */
int test(const struct unix_filesystem *u) {
    M_REQUIRE_NON_NULL(u);

    // print inode 3 and 5
    _print_inode(u, 3);
    _print_inode(u, 5);

    struct inode in;
    puts("\nListing inodes SHA:");
    for (uint16_t i = 1; i <= u->s.s_isize * INODES_PER_SECTOR; ++i) {
        if (inode_read(u, i, &in) == 0) print_sha_inode(u, in, i);
    }
    return 0;
}

void _print_inode(const struct unix_filesystem *u, uint16_t inr){
    struct filev6 fv6;
    memset(&fv6, 0, sizeof(fv6));

    putchar('\n');

    int err = filev6_open(u, inr, &fv6);
    if (err != 0) {
        printf("filev6 open failed for inode #%d.\n", inr);
        return;
    } else {
        printf("Printing inode #%d:\n", inr);
        struct inode* inode = &fv6.i_node;
        inode_print(inode);

        if ((inode->i_mode & IFDIR) != 0) {
            puts("which is a directory.");
        } else {
            puts("the first sector of data which contains:");
            char data[SECTOR_SIZE + 1];
            data[SECTOR_SIZE] = '\0';

            err = filev6_readblock(&fv6, data);
            if(err<0){
                return;
            }

            printf("%s\n", data);
            puts("----");
        }
    }

    return;
}
