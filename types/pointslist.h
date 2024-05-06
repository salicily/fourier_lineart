#ifndef POINTSLIST_H_
#define POINTSLIST_H_

#include <stdint.h>
#include <stddef.h>

struct point {
    uint16_t x;
    uint16_t y;
};

struct points_list;

struct points_list *create_points_list(size_t points_num);

void destroy_points_list(struct points_list *pl);

size_t get_points_num(const struct points_list *pl);

int get_point_from_points_list(const struct points_list *pl, size_t index, struct point *pt);

int set_point_from_points_list(struct points_list *pl, size_t index, const struct point *pt);

#endif
