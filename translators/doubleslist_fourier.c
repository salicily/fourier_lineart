#include "doubleslist_fourier.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

double scalar_product(const struct doubles_list *dl, double (*base)(size_t,double), size_t mode) {
    if (dl == NULL) {
        return 0.0;
    }
    size_t points = get_doubles_num(dl);
    if (points == 0) {
        return 0.0;
    }
    double integrate = 0.0;
    double dt = 1.0 / (double)points;
    double t = 0.0;
    for (size_t i = 0; i < points; ++i) {
        double w = get_double_from_doubles_list(dl, i);
        integrate += w * base(mode, t);
        t += dt;
    }
    integrate /= ((double)points);
    return integrate;
}

void add_base_vector(struct doubles_list *dl, double (*base)(size_t,double), size_t mode, double k) {
    if (dl == NULL) {
        return;
    }
    size_t points = get_doubles_num(dl);
    if (points == 0) {
        return;
    }
    double dt = 1.0 / (double)points;
    double t = 0.0;
    for (size_t i = 0; i < points; ++i) {
        double fi = get_double_from_doubles_list(dl, i);
        fi += k * base(mode, t);
        set_double_from_doubles_list(dl, i, fi);
        t += dt;
    }
    return;
}
