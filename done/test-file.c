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
#include "sha.h"

int test(const struct unix_filesystem *u) {

	struct filev6 fs;
	memset(&fs, 255, sizeof(fs));

	struct inode in;

	for (int i_number = 3; i_number <= 5; i_number = i_number + 2) {
		putchar('\n');

		int err = inode_read(u, i_number, &in);
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
				//memset(data, 0, sizeof(data));
				int sector = inode_findsector(u, &in, 0);
				sector_read(u->f, sector, data);
				printf("%s\n", data);
				puts("----");
			}
		}
	}

	puts("\nListing inodes SHA:");
	puts("FIXME");
	for(int i = 0; i<=u->s.s_isize; i++){
		inode_read(u, i, &in);
		//FIXME print_sha_inode(u, in, i);
		putchar('\n');
	}

	return 0;

}