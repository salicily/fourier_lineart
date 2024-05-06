#include "pointslist_doubleslist.h"

struct split split_points_list(struct points_list *pl, uint32_t width, uint32_t height) {
    struct split result = {
        .dlx = NULL,
        .dly = NULL,
    };
    if (pl == NULL) {
        return result;
    }
    if (width == 0) {
        return result;
    }
    if (height == 0) {
        return result;
    }
    size_t points_num = get_points_num(pl);
    result.dlx = create_doubles_list(points_num);
    if (result.dlx == NULL) {
        return result;
    }
    result.dly = create_doubles_list(points_num);
    if (result.dly == NULL) {
        destroy_doubles_list(result.dlx);
        result.dlx = NULL;
        return result;
    }
    for (size_t i = 0; i < points_num; ++i) {
        struct point pt = { .x = 0, .y = 0, };
        (void)get_point_from_points_list(pl, i, &pt);
        double fx = ((double)pt.x) / (double)width;
        double fy = ((double)pt.y) / (double)height;
        (void)set_double_from_doubles_list(result.dlx, i, fx);
        (void)set_double_from_doubles_list(result.dly, i, fy);
    }
    return result;
}

struct points_list *merge_doubles_list(struct split sp, uint32_t width, uint32_t height) {
    if (sp.dlx == NULL) {
        return NULL;
    }
    if (sp.dly == NULL) {
        return NULL;
    }
    if (width == 0) {
        return NULL;
    }
    if (height == 0) {
        return NULL;
    }
    size_t xnum = get_doubles_num(sp.dlx);
    size_t ynum = get_doubles_num(sp.dly);
    if (xnum != ynum) {
        return NULL;
    }
    struct points_list *pl = create_points_list(xnum);
    if (pl == NULL) {
        return NULL;
    }
    for (size_t i = 0; i < xnum; ++i) {
        double fx = get_double_from_doubles_list(sp.dlx, i);
        double fy = get_double_from_doubles_list(sp.dly, i);
        fx *= (double)width;
        fy *= (double)height;
        if ((fx < 0.0) || (fx > (double)UINT16_MAX) || (fy < 0.0) || (fy > (double)UINT16_MAX)) {
            fx = 0.0;
            fy = 0.0;
        }
        struct point pt = {
            .x = (uint16_t)fx,
            .y = (uint16_t)fy,
        };
        (void)set_point_from_points_list(pl, i, &pt);
    }
    return pl;
}
