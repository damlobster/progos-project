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

int main(){
    struct bmblock_array* bmb = bm_alloc(67, 562);

    puts("BMSET *****************************************");
    puts("BM with a one every 7 bits");
    for(uint64_t i = 67; i<800; i+=7){
        bm_set(bmb, i);
    }
    bm_print(bmb);

    puts("\n\nBM_CLEAR ******************************************");
    puts("clear one bit at one every two");
    for(uint64_t i = 67; i<800; i+=14){
        bm_clear(bmb, i);
    }
    bm_print(bmb);

    puts("\n\nBM_SET with FIND **********************************");
    puts("find a free bit set it to one");
    for(uint64_t i = 67; i<560; i++){
        int b = bm_find_next(bmb);
        bm_set(bmb, (uint64_t)b);
    }
    puts("Should be all ones!");
    bm_print(bmb);

    puts("\n\nBM_CLEAR and FIND **********************************");
    puts("find a free bit after clearing it");
    bm_clear(bmb, 152LU);
    printf("%d == 152 ?\n", bm_find_next(bmb));
    printf("Cursor: %zu, should be 1 at this point!\n", bmb->cursor);
    bm_set(bmb, 152LU);
    printf("Cursor: %zu, should be 2 at this point!\n", bmb->cursor);
    printf("%d == -119 ?\n", bm_find_next(bmb));

    puts("\n\nBM_GET **********************************");
    puts("Change value of bit 153");
    bm_clear(bmb, 153LU);
    printf("%d == 0 ?\n", bm_get(bmb, 153LU));
    bm_set(bmb, 153LU);
    printf("%d == 1 ?\n", bm_get(bmb, 153LU));

    free(bmb);
}
