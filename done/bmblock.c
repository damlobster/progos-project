/*
 * bmblock.c
 *
 * Created on: May 3, 2017
 * Author: Léonard et Damien
 */
#include <stdlib.h>
#include <stdio.h>
#include "bmblock.h"
#include "error.h"

/**
 * @brief allocate a new bmblock_array to handle elements indexed
 * between min and may (included, thus (max-min+1) elements).
 * @param min the mininum value supported by our bmblock_array
 * @param max the maxinum value supported by our bmblock_array
 * @return a pointer of the newly created bmblock_array or NULL on failure
 */
struct bmblock_array *bm_alloc(uint64_t min, uint64_t max) {
    if (min > max) return NULL;
    uint64_t nb_bits = max - min + 1;
    struct bmblock_array* bmb = calloc(1,
            sizeof(struct bmblock_array) + (nb_bits / 64));
    bmb->cursor = 0;
    bmb->length = nb_bits;
    bmb->min = min;
    bmb->max = max;
    return bmb;
}

/**
 * @brief return the bit associated to the given value
 * @param bmblock_array the array containing the value we want to read
 * @param x an integer corresponding to the number of the value we are looking for
 * @return <0 on failure, 0 or 1 on success
 */
int bm_get(struct bmblock_array *bmblock_array, uint64_t x) {
    M_REQUIRE_NON_NULL(bmblock_array);
    if (x < bmblock_array->min || x > bmblock_array->max) {
        return ERR_BAD_PARAMETER;
    }
    return (bmblock_array->bm[(x - bmblock_array->min) / 64]
            >> ((x - bmblock_array->min) % 64)) & 1;
}

/**
 * @brief set to true (or 1) the bit associated to the given value
 * @param bmblock_array the array containing the value we want to set
 * @param x an integer corresponding to the number of the value we are looking for
 */
void bm_set(struct bmblock_array *bmblock_array, uint64_t x) {
    if (bmblock_array == NULL) return;
    if (x < bmblock_array->min || x > bmblock_array->max) return;
    bmblock_array->bm[(x - bmblock_array->min) / 64] |= UINT64_C(1)
            << ((x - bmblock_array->min) % 64);
}

/**
 * @brief set to false (or 0) the bit associated to the given value
 * @param bmblock_array the array containing the value we want to clear
 * @param x an integer corresponding to the number of the value we are looking for
 */
void bm_clear(struct bmblock_array *bmblock_array, uint64_t x) {
    if (bmblock_array == NULL) return;
    if (x < bmblock_array->min || x > bmblock_array->max) return;
    bmblock_array->bm[(x - bmblock_array->min) / 64] &= (UINT64_MAX - 1)
            << ((x - bmblock_array->min) % 64);
}

/**
 * @brief return the next unused bit
 * @param bmblock_array the array we want to search for place
 * @return <0 on failure, the value of the next unused value otherwise
 */
int bm_find_next(struct bmblock_array *bmblock_array) {
    return -100;
}

/**
 * @brief usefull to see (and debug) content of a bmblock_array
 * @param bmblock_array the array we want to see
 */
void bm_print(struct bmblock_array *bmblock_array) {
    if (bmblock_array == NULL) debug_print("bmblock_array is NULL",);

    puts("**********BitMap Block START**********");
    printf("length: %zu\n", bmblock_array->length);
    printf("min: %zu\n", bmblock_array->min);
    printf("max: %zu\n", bmblock_array->max);
    printf("cursor: %zu\n", bmblock_array->cursor);

    puts("content:");
    uint64_t size_in_word = (bmblock_array->length - 1) / 64;
    for (uint64_t i = 0; i < size_in_word; i++) {
        printf("\n%zu: ", i);

        uint64_t word = bmblock_array->bm[i];

        for (int bit = 64; bit >= 0; bit--) {
            putchar((word & 1) ? '1' : '0');
            word = word >> 1;
            if (word % 8 == 0) putchar(' ');
        }
    }

    printf("\n**********BitMap Block END************\n");
}
