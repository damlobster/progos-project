/*
 * inode.c
 *
 *  Created on: Mar 15, 2017
 *  Authors: Léonard Berney & Damien Martin
 */

#include "inode.h"
#include "error.h"
#include "sector.h"

/**
 * @brief read all inodes from disk and print out their content to
 *        stdout according to the assignment
 * @param u the filesystem
 * @return 0 on success; < 0 on error.
 */
int inode_scan_print(const struct unix_filesystem *u) {
	M_REQUIRE_NON_NULL(u);

	// loop on all inode sectors
	for (int k = u->s.s_inode_start; k < u->s.s_isize; k++) {

		struct inode inodes[INODES_PER_SECTOR];
		int r = sector_read(u->f, k, inodes);
		if (r < 0) return r; // propagate sector_read error code if any

		// loop on all inode of the current sector
		for (unsigned int i = 0; i < INODES_PER_SECTOR; i++) {

			if (inodes[i].i_mode & IALLOC) {
				// inode is allocated --> print it
				int size = inode_getsize(&inodes[i]);
				char *type = (inodes[i].i_mode & IFDIR) ? SHORT_DIR_NAME : SHORT_FIL_NAME;
				printf("inode %3d (%s) len %4d\n", i, type, size);
			}
		}
	}

	return 0; // print successful
}
