#ifndef POINTSLIST_DOUBLESLIST_
#define POINTSLIST_DOUBLESLIST_

#include "../types/pointslist.h"
#include "../types/doubleslist.h"

struct split {
    struct doubles_list *dlx;
    struct doubles_list *dly;
};

struct split split_points_list(struct points_list *pl, uint32_t width, uint32_t height);

struct points_list *merge_doubles_list(struct split sp, uint32_t width, uint32_t height);

#endif
