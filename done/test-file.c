/*
 * test-file.c
 *
 *  Created on: Mar 20, 2017
 *      Author: damien
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "filev6.h"
#include "inode.h"
#include "mount.h"
#include "sector.h"
#include "unixv6fs.h"

int test(const struct unix_filesystem *u) {

	struct filev6 fs;
	memset(&fs, 255, sizeof(fs));

	struct inode in;
	int err = inode_read(u, 3, &in);
	if (err != 0) {
		puts("filev6 open failed for inode #3");
	}
	inode_print(&in);

	if ((in.i_mode & IFDIR) != 0) {
		puts("which is a directory.");
	} else {
		char data[SECTOR_SIZE + 1];
		memset(data, 0, sizeof(data));
		sector_read(u->f, 0, data);
		printf("%s", data);
	}

	return 0;

}
