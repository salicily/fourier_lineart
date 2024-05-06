#include "pointslist.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct points_list {
    size_t points_num;
    struct point points[];
};

struct points_list *create_points_list(size_t points_num) {
    struct points_list *res = malloc(sizeof(*res) + sizeof(struct point) * points_num);
    if (res == NULL) {
        return NULL;
    }
    memset(res->points, 0, sizeof(struct point) * points_num);
    res->points_num = points_num;
    return res;
}

void destroy_points_list(struct points_list *pl) {
    if (pl == NULL) {
        return;
    }
    free(pl);
    return;
}

size_t get_points_num(const struct points_list *pl) {
    if (pl == NULL) {
        return 0;
    }
    return pl->points_num;
}

int get_point_from_points_list(const struct points_list *pl, size_t index, struct point *pt) {
    if (pl == NULL) {
        return -1;
    }
    if (pt == NULL) {
        return -1;
    }
    if (index < pl->points_num) {
        *pt = pl->points[index];
        return 0;
    }
    return -1;
}

int set_point_from_points_list(struct points_list *pl, size_t index, const struct point *pt) {
    if (pl == NULL) {
        return -1;
    }
    if (pt == NULL) {
        return -1;
    }
    if (index < pl->points_num) {
        pl->points[index] = *pt;
        return 0;
    }
    return -1;
}
