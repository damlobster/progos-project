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

#define BMB_GET_WORD_IDX(bmb, n) ((n - bmb->min) / BITS_PER_VECTOR)
#define BMB_GET_WORD(bmb, n) (bmb->bm[BMB_GET_WORD_IDX(bmb, n)])
#define BMB_GET_BIT_IDX(bmb, n) ((n - bmb->min) % BITS_PER_VECTOR)

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
            sizeof(struct bmblock_array) + (nb_bits / BITS_PER_VECTOR));
    bmb->cursor = 0;
    bmb->length = nb_bits / BITS_PER_VECTOR + 1;
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
    return (BMB_GET_WORD(bmblock_array, x) >> BMB_GET_BIT_IDX(bmblock_array, x))
            & 1;
}

/**
 * @brief set to true (or 1) the bit associated to the given value
 * @param bmblock_array the array containing the value we want to set
 * @param x an integer corresponding to the number of the value we are looking for
 */
void bm_set(struct bmblock_array *bmblock_array, uint64_t x) {
    if (bmblock_array == NULL) return;
    if (x < bmblock_array->min || x > bmblock_array->max) return;
    BMB_GET_WORD(bmblock_array, x) |= UINT64_C(
            1) << BMB_GET_BIT_IDX(bmblock_array, x);

    if (bmblock_array->cursor == BMB_GET_WORD_IDX(bmblock_array, x)
            && BMB_GET_WORD(bmblock_array, x) == UINT64_MAX) {
        bmblock_array->cursor++;
    }
}

/**
 * @brief set to false (or 0) the bit associated to the given value
 * @param bmblock_array the array containing the value we want to clear
 * @param x an integer corresponding to the number of the value we are looking for
 */
void bm_clear(struct bmblock_array *bmblock_array, uint64_t x) {
    if (bmblock_array == NULL) return;
    if (x < bmblock_array->min || x > bmblock_array->max) return;
    BMB_GET_WORD(bmblock_array, x) &= (UINT64_C(1)
            << BMB_GET_BIT_IDX(bmblock_array, x)) ^ UINT64_MAX;

    if (bmblock_array->cursor > BMB_GET_WORD_IDX(bmblock_array, x)) {
        bmblock_array->cursor = BMB_GET_WORD_IDX(bmblock_array, x);
    }
}

/**
 * @brief return the next unused bit
 * @param bmblock_array the array we want to search for place
 * @return <0 on failure, the value of the next unused value otherwise
 */
int bm_find_next(struct bmblock_array *bmblock_array) {
    M_REQUIRE_NON_NULL(bmblock_array);

    while (bmblock_array->bm[bmblock_array->cursor] == UINT64_C(-1)
            && bmblock_array->cursor <= bmblock_array->length) {
        //update cursor if current BITS_PER_VECTOR bits are used
        bmblock_array->cursor++;
    }

    if (bmblock_array->cursor > bmblock_array->length) {
        return ERR_BITMAP_FULL;
    }

    unsigned char n;
    for (n = 0; n < 63 && //
            (bmblock_array->bm[bmblock_array->cursor] & (UINT64_C(1) << n));
            n++) {
    }
    uint64_t next_free = bmblock_array->min
            + BITS_PER_VECTOR * bmblock_array->cursor + n;
    return next_free <= bmblock_array->max ? (int) next_free : ERR_BITMAP_FULL;
}

/**
 * @brief usefull to see (and debug) content of a bmblock_array
 * @param bmblock_array the array we want to see
 */
void bm_print(struct bmblock_array *bmblock_array) {
    if (bmblock_array == NULL) {
        debug_print("bmblock_array is %s", "NULL");
        return;
    }

    puts("**********BitMap Block START**********");
    printf("length: %zu\n", bmblock_array->length);
    printf("min: %zu\n", bmblock_array->min);
    printf("max: %zu\n", bmblock_array->max);
    printf("cursor: %zu\n", bmblock_array->cursor);

    printf("content:");
#ifdef DEBUG
    printf(
            "\n     0      7       15       23       31       39       45       55       63");
#endif
    for (uint64_t i = 0; i < bmblock_array->length; i++) {
#ifdef DEBUG
        printf("\n%03zu: ", i);
#else
        printf("\n%zu: ", i);
#endif
        uint64_t word = bmblock_array->bm[i];

        for (int bit = 63; bit >= 0; bit--) {
            putchar((word & 1) ? '1' : '0');
            word = word >> 1;
            if (bit % 8 == 0) putchar(' ');
        }
    }

    printf("\n**********BitMap Block END************\n");
    fflush(stdout);
}

/**
 * Get the size in byte of the bitmap block
 * @param bmblock_array
 * @return <0 if bmblock_array is NULL, otherwise the number of bytes
 */
size_t bm_sizeof(struct bmblock_array * bmblock_array) {
    if (bmblock_array == NULL) {
        return 0;
    }

    return sizeof(struct bmblock_array)
            + (bmblock_array->length - 1) * (BITS_PER_VECTOR / 8);
}

