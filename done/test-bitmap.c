/*
 * test_bmblock.c
 *
 *  Created on: May 4, 2017
 *      Author: damien
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "bmblock.h"
#include "error.h"
#include "mount.h"

/**
 * Tests for the bitmap API.
 *
 * @param u unused here
 * @return 0;
 */
int test(struct unix_filesystem *u) {
    (void) u;

    struct bmblock_array *bm = bm_alloc(4, 131);
    if (NULL == bm) {
        return ERR_NOMEM;
    }

    bm_print(bm);
    printf("find_next() = %d\n", bm_find_next(bm));

    bm_set(bm, 4);
    bm_set(bm, 5);
    bm_set(bm, 6);
    bm_print(bm);
    printf("find_next() = %d\n", bm_find_next(bm));

    for (uint64_t i = 4; i <= 130; i += 3) {
        bm_set(bm, i);
    }
    bm_print(bm);
    printf("find_next() = %d\n", bm_find_next(bm));

    for (uint64_t i = 5; i <= 130; i += 5) {
        bm_clear(bm, i);
    }
    bm_print(bm);
    printf("find_next() = %d\n", bm_find_next(bm));

    free(bm);
    return 0;
}
