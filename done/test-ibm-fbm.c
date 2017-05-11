/*
 * test_bmblock.c
 *
 *  Created on: May 4, 2017
 *      Author: damien
 */

#include <stdio.h>

#include "bmblock.h"
#include "mount.h"

/**
 * Tests for IBM and FBM allocation and filling
 * @param u the filesystem
 * @return 0
 */
int test(struct unix_filesystem *u) {
    puts("### fbm:");
    bm_print(u->fbm);
    puts("### ibm:");
    bm_print(u->ibm);

    return 0;
}
