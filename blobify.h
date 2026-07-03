#ifndef __BLOBIFY_H__
#define __BLOBIFY_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>

typedef struct blb_block_t blb_block_t;
typedef struct blb_range_t blb_range_t;
typedef struct blb_cursor_t blb_cursor_t;

typedef struct blb_blob_t blb_blob_t;

#define BLB_BLOCK_MAX_SIZE 1024 * 1024 * 32

struct blb_block_t{
    uint8_t *base;
    uint32_t size;
    bool allocated;
};

blb_block_t *blb_block_create(uint32_t size);
void blb_block_delete(blb_block_t *block);

void blb_block_print(blb_block_t *block, FILE *output);

bool blb_block_put(blb_block_t *block, int32_t offset, uint8_t value);
bool blb_block_get(blb_block_t *block, int32_t offset, uint8_t *value);

struct blb_range_t{
    uint32_t start;
    uint32_t size;
    uint8_t step;
    bool fixed;
};

blb_range_t *blb_range_create(uint32_t start, uint32_t size, uint8_t step, bool fixed);
void blb_range_delete(blb_range_t *range);

// TODO: slide rotation
bool blb_range_slide(blb_range_t *range, int32_t steps);
bool blb_range_resize(blb_range_t *range, int32_t size);
bool blb_range_in(blb_range_t *range, int32_t value);

// TODO: update step value


struct blb_cursor_t{
    int32_t offset;
    bool fixed;
};

blb_cursor_t *blb_cursor_create(int32_t offset, bool fixed);
void blb_cursor_delete(blb_cursor_t *cursor);

bool blb_cursor_step(blb_cursor_t *cursor, int32_t step);
bool blb_cursor_jump(blb_cursor_t *cursor, uint32_t value);

struct blb_blob_t{
    blb_block_t *block;
    blb_range_t *range;
    blb_cursor_t *cursor;
};

blb_blob_t *blb_blob_create(uint32_t size, uint8_t step);
void blb_blob_delete(blb_blob_t *blob);

bool blb_blob_step(blb_blob_t *blob, int32_t step);

#endif
