/*
 * test-create.c
 *
 *  Created on: 10.05.2017
 *      Author: Damien & Léonard
 */

#include "inode.h"

/**
 * Tests for the creation of new inodes
 * @param u the file system
 * @return <0 if error occured
 */

int test(struct unix_filesystem *u) {

    struct inode i;
    i.i_mode = IALLOC;
    int err = inode_write(u, 12, &i);
    if (err != 0) {
        return err;
    }
    puts("INODE 12 should be a FILE!");
    inode_scan_print(u);

    i.i_mode = IALLOC|IFDIR;
    inode_write(u, 12, &i);
    puts("INODE 12 should be a DIR!");
    inode_scan_print(u);

    i.i_mode = 0;
    inode_write(u, 12, &i);
    puts("INODE 12 shouldn't be ALLOCATED!");
    inode_scan_print(u);
    return 0;
}

