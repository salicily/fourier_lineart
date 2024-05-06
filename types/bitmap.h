#ifndef BITMAP_H
#define BITMAP_H

#include <stdint.h>
#include <stddef.h>

struct rgba {
    uint8_t b;
    uint8_t g;
    uint8_t r;
    uint8_t a;
};

struct raw_bitmap;

struct raw_bitmap_info {
    uint32_t width;
    uint32_t height;
    uint16_t bits_per_pixel;
    uint32_t w_ppm;
    uint32_t h_ppm;
    uint32_t colors_in_color_map;
};

struct raw_bitmap_info get_raw_bitmap_info(const struct raw_bitmap *bm);

int get_pixel(const struct raw_bitmap *bm, uint32_t x, uint32_t y, uint32_t *color_index);

int set_pixel(struct raw_bitmap *bm, uint32_t x, uint32_t y, uint32_t color_index);

int get_color(const struct raw_bitmap *bm, uint32_t index, struct rgba *color);

int set_color(struct raw_bitmap *bm, uint32_t index, struct rgba color);

int set_color_map(struct raw_bitmap *bm, const struct rgba *color_map, size_t colors);

int get_color_map(const struct raw_bitmap *bm, struct rgba *color_map, size_t colors);

int set_bitmap(struct raw_bitmap *bm, const uint8_t *bitmap, size_t bitmap_size);

int get_bitmap(const struct raw_bitmap *bm, uint8_t *bitmap, size_t bitmap_size);

struct raw_bitmap *create_raw_bitmap(struct raw_bitmap_info rbi);

void destroy_raw_bitmap(struct raw_bitmap *bm);

#endif
