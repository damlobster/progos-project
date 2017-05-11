/*
 * test-inodes.c
 *
 *  Created on: Mar 13, 2017
 *      Author: damien
 */

#include "inode.h"

/**
 * Tests for inode.c
 * @param u the file system
 * @return <0 if error occured
 */

int test(struct unix_filesystem *u){
	return inode_scan_print(u);
}

