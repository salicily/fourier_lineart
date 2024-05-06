#include "bitmap_pointslist.h"
#include <stdlib.h>
#include <stdio.h>

static size_t get_points_(const struct raw_bitmap *bm, struct points_list *pts, uint32_t pixel) {
    size_t points = 0;
    struct raw_bitmap_info rbi = get_raw_bitmap_info(bm);
    if (rbi.width > UINT16_MAX) {
        return 0;
    }
    if (rbi.height > UINT16_MAX) {
        return 0;
    }
    for (uint16_t j = 0; j < rbi.height; ++j) {
        for (uint16_t i = 0; i < rbi.width; ++i) {
            uint32_t col;
            (void)get_pixel(bm, i, j, &col);
            if (col == pixel) {
                if (pts != NULL) {
                    struct point pt = {
                        .x = i,
                        .y = j,
                    };
                    set_point_from_points_list(pts, points, &pt);
                }
                ++points;
            }
        }
    }
    return points;
}

struct points_list *get_points_list(const struct raw_bitmap *bm, uint32_t pixel) {
    size_t points = get_points_(bm, NULL, pixel);
    struct points_list *pl = create_points_list(points);
    if (pl == NULL) {
        return NULL;
    }
    (void)get_points_(bm, pl, pixel);
    return pl;
}

int draw_points_list(struct raw_bitmap *bm, const struct points_list *pts, uint32_t pixel) {
    if (bm == NULL) {
        return -1;
    }
    if (pts == NULL) {
        return -1;
    }
    size_t points_num = get_points_num(pts);
    int result = 0;
    for (size_t i = 0; i < points_num; ++i) {
        struct point pt;
        (void)get_point_from_points_list(pts, i, &pt);
        if (set_pixel(bm, pt.x, pt.y, pixel) != 0) {
            ++result;
        }
    }
    return result;
}
