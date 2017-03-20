/*
 * test-file.c
 *
 *  Created on: Mar 20, 2017
 *      Author: damien
 */
#include "unixv6fs.h"
#include "mount.h"
#include "inode.h"

int test(const struct unix_filesystem *u){
    struct inode in;
    int err = inode_read(u, 5, &in);
    if (err != 0) {
        return err;
    }
    inode_print(&in);

    printf("inode_findsector(u, i@3, 0)= %d\n", inode_findsector(&u, &in, 0));
    printf("inode_findsector(u, i@3, 1)= %d\n", inode_findsector(&u, &in, 1));
    printf("inode_findsector(u, i@3, 37)= %d\n", inode_findsector(&u, &in, 37));

    return 0;

}
