/*
 * inode.c
 *
 *  Created on: Mar 15, 2017
 *  Authors: Léonard Berney & Damien Martin
 */

#include "inode.h"

/**
 * @brief read all inodes from disk and print out their content to
 *        stdout according to the assignment
 * @param u the filesystem
 * @return 0 on success; < 0 on error.
 */
int inode_scan_print(const struct unix_filesystem *u) {
	M_REQUIRE_NON_NULL(u);

	for (int i = u->s.s_inode_start; i < u->s.s_isize; i++) {
		const struct inode inodes[INODES_PER_SECTOR];
		sector_read(u->f, i, &inodes);

		for (int j = 0; j < INODES_PER_SECTOR; j++) {
			if (inodes[i].i_mode & IALLOC) {
				char itype[3] = //
						(inodes[i].i_mode & IFDIR) ? SHORT_DIR_NAME : SHORT_FIL_NAME;
				int size = inodes[i].i_size1 << 8 + inodes.i_size0;
				printf("inode %3d (%s) len %4d", i, itype, size);
			}
		}
	}
	return 0;
}
