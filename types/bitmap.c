#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "bitmap.h"

struct raw_bitmap {
    struct raw_bitmap_info rbi;
    struct rgba *color_map;
    uint32_t *bitmap_array;
    size_t line_words;
    uint32_t data[];
};

struct raw_bitmap_info get_raw_bitmap_info(const struct raw_bitmap *bm) {
    if (bm != NULL) {
        return bm->rbi;
    }
    struct raw_bitmap_info rbi = { 0 };
    return rbi;
}

int get_pixel(const struct raw_bitmap *bm, uint32_t x, uint32_t y, uint32_t *color_index) {
    if (bm == NULL) {
        return -1;
    }
    if (color_index == NULL) {
        return -1;
    }
    if (y >= bm->rbi.height) {
        return -1;
    }
    if (x >= bm->rbi.width) {
        return -1;
    }
    uint32_t *line = bm->bitmap_array + y * bm->line_words;
    if (bm->rbi.bits_per_pixel <= 8) {
        uint8_t *subline = (uint8_t*)line;
        size_t k = 8 / bm->rbi.bits_per_pixel;
        size_t counter_shift = 8 - bm->rbi.bits_per_pixel;
        size_t shift = (x%k) * bm->rbi.bits_per_pixel;
        uint8_t ci = subline[x / k] << shift;
        ci >>= counter_shift;
        *color_index = ci;
        return 0;
    }
    if (bm->rbi.bits_per_pixel == 32) {
        *color_index = line[x];
        return 0;
    }
    if (bm->rbi.bits_per_pixel == 16) {
        uint16_t *subline = (uint16_t *)line;
        *color_index = subline[x];
        return 0;
    }
    if (bm->rbi.bits_per_pixel == 24) {
        size_t shift = (x >> 2) * 3;
        switch (x % 4) {
            case 0:
                *color_index = line[shift] & UINT32_C(0xffffff);
                return 0;
            case 1:
                *color_index = (line[shift] << 24) | ((line[shift + 1] & UINT32_C(0xffff)) >> 8);
                return 0;
            case 2:
                *color_index = (line[shift + 1] << 16) | ((line[shift + 2] & UINT32_C(0xff)) >> 16);
                return 0;
            case 3:
                *color_index = line[shift + 2] << 8;
                return 0;
        }
    }
    return -1;
}

int set_pixel(struct raw_bitmap *bm, uint32_t x, uint32_t y, uint32_t color_index) {
    if (bm == NULL) {
        return -1;
    }
    if (y >= bm->rbi.height) {
        return -1;
    }
    if (x >= bm->rbi.width) {
        return -1;
    }
    uint32_t *line = bm->bitmap_array + y * bm->line_words;
    if (bm->rbi.bits_per_pixel <= 8) {
        uint8_t *subline = (uint8_t*)line;
        size_t k = 8 / bm->rbi.bits_per_pixel;
        size_t offset = x / k;
        size_t counter_shift = 8 - bm->rbi.bits_per_pixel;
        size_t shift = (x%k) * bm->rbi.bits_per_pixel;
        uint8_t mask = UINT8_MAX;
        uint8_t ci = color_index;
        ci <<= counter_shift;
        mask <<= counter_shift;
        ci >>= shift;
        mask >>= shift;
        subline[offset] &= ~mask;
        subline[offset] |= ci;
        return 0;
    }
    if (bm->rbi.bits_per_pixel == 32) {
        line[x] = color_index;
        return 0;
    }
    if (bm->rbi.bits_per_pixel == 16) {
        uint16_t *subline = (uint16_t *)line;
        subline[x] = color_index;
        return 0;
    }
    if (bm->rbi.bits_per_pixel == 24) {
        size_t shift = (x >> 2) * 3;
        switch (x % 4) {
            case 0:
                line[shift] &= UINT32_C(0xff000000);
                line[shift] |= (color_index & UINT32_C(0x00ffffff));
                return 0;
            case 1:
                line[shift] &= UINT32_C(0x00ffffff);
                line[shift] |= color_index << 24;
                line[shift + 1] &= UINT32_C(0xffff0000);
                line[shift + 1] |= (color_index >> 8) & UINT32_C(0x0000ffff);
                return 0;
            case 2:
                line[shift + 1] &= UINT32_C(0x0000ffff);
                line[shift + 1] |= color_index << 16;
                line[shift + 2] &= UINT32_C(0xffffff00);
                line[shift + 2] |= (color_index >> 16) & UINT32_C(0x000000ff);
                return 0;
            case 3:
                line[shift + 2] &= UINT32_C(0x000000ff);
                line[shift + 2] |= color_index << 8;
                return 0;
        }
    }
    return -1;
}

int get_color(const struct raw_bitmap *bm, uint32_t index, struct rgba *color) {
    if (bm == NULL) {
        return -1;
    }
    if (color == NULL) {
        return -1;
    }
    if (index >= bm->rbi.colors_in_color_map) {
        return -1;
    }
    *color = bm->color_map[index];
    return 0;
}

int set_color(struct raw_bitmap *bm, uint32_t index, struct rgba color) {
    if (bm == NULL) {
        return -1;
    }
    if (index >= bm->rbi.colors_in_color_map) {
        return -1;
    }
    bm->color_map[index] = color;
    return 0;
}

struct raw_bitmap *create_raw_bitmap(struct raw_bitmap_info rbi) {
    switch (rbi.bits_per_pixel) {
        case 1:
        case 4:
        case 8:
        case 16:
        case 24:
        case 32:
            break;
        default:
            return NULL;
    }
    uint32_t line_words = ((rbi.width * rbi.bits_per_pixel + 31) >> 5);
    uint32_t bitmap_size = line_words * rbi.height * sizeof(uint32_t);
    uint32_t color_map_size = 0;
    if (rbi.bits_per_pixel <= 8) {
        if (rbi.colors_in_color_map == 0) {
            return NULL;
        }
        if (((rbi.colors_in_color_map - 1) >> rbi.bits_per_pixel) > 0) {
            return NULL;
        }
        color_map_size = sizeof(uint32_t) * rbi.colors_in_color_map;
    } else {
        if (rbi.colors_in_color_map > 0) {
            return NULL;
        }
    }
    struct raw_bitmap *bm = malloc(sizeof(*bm) + color_map_size + bitmap_size);
    if (bm == NULL) {
        return NULL;
    }
    bm->rbi = rbi;
    bm->color_map = (struct rgba *)bm->data;
    bm->bitmap_array = bm->data + rbi.colors_in_color_map;
    bm->line_words = line_words;
    memset(bm->data, 0, color_map_size + bitmap_size);
    return bm;
}

void destroy_raw_bitmap(struct raw_bitmap *bm) {
    memset(bm, 0, sizeof(*bm));
    free(bm);
    return;
}

int set_color_map(struct raw_bitmap *bm, const struct rgba *color_map, size_t colors) {
    if (bm == NULL) {
        return -1;
    }
    if (bm->rbi.colors_in_color_map != colors) {
        return -1;
    }
    if (colors == 0) {
        return 0;
    }
    memcpy(bm->color_map, color_map, colors * sizeof(*color_map));
    return 0;
}

int get_color_map(const struct raw_bitmap *bm, struct rgba *color_map, size_t colors) {
    if (bm == NULL) {
        return -1;
    }
    if (bm->rbi.colors_in_color_map != colors) {
        return -1;
    }
    if (colors == 0) {
        return 0;
    }
    memcpy(color_map, bm->color_map, colors * sizeof(*color_map));
    return 0;
}

int set_bitmap(struct raw_bitmap *bm, const uint8_t *bitmap, size_t bitmap_size) {
    if (bm == NULL) {
        return -1;
    }
    if ((bm->line_words * bm->rbi.height * sizeof(uint32_t)) != bitmap_size) {
        return -1;
    }
    if (bitmap_size == 0) {
        return 0;
    }
    if (bitmap == NULL) {
        return -1;
    }
    memcpy(bm->bitmap_array, bitmap, bitmap_size);
    return 0;
}

int get_bitmap(const struct raw_bitmap *bm, uint8_t *bitmap, size_t bitmap_size) {
    if (bm == NULL) {
        return -1;
    }
    if ((bm->line_words * bm->rbi.height * sizeof(uint32_t)) != bitmap_size) {
        return -1;
    }
    if (bitmap_size == 0) {
        return 0;
    }
    if (bitmap == NULL) {
        return -1;
    }
    memcpy(bitmap, bm->bitmap_array, bitmap_size);
    return 0;
}
