/*
 * test-inodes.c
 *
 *  Created on: Mar 13, 2017
 *      Author: damien
 */

#include "inode.h"

int test(struct unix_filesystem *u){
	return inode_scan_print(u);
}

