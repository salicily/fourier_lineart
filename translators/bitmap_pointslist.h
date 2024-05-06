#ifndef BITMAP_POINTSLISTS_H_
#define BITMAP_POINTSLISTS_H_

#include "../types/pointslist.h"
#include "../types/bitmap.h"

struct points_list *get_points_list(const struct raw_bitmap *bm, uint32_t pixel);

int draw_points_list(struct raw_bitmap *bm, const struct points_list *pts, uint32_t pixel);

#endif
