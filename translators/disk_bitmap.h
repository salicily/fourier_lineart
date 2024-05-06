#ifndef DISK_BITMAP_H
#define DISK_BITMAP_H

#include <stdint.h>
#include "../types/bitmap.h"

struct raw_bitmap *disk_to_bitmap(const char *fname);

int bitmap_to_disk(const struct raw_bitmap *bm, const char *fname);

#endif
