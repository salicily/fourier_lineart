#include "doubleslist.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct doubles_list {
    size_t doubles_num;
    double doubles[];
};

struct doubles_list *create_doubles_list(size_t doubles_num) {
    struct doubles_list *res = malloc(sizeof(*res) + sizeof(double) * doubles_num);
    if (res == NULL) {
        return NULL;
    }
    memset(res->doubles, 0, sizeof(double) * doubles_num);
    res->doubles_num = doubles_num;
    return res;
}

void destroy_doubles_list(struct doubles_list *dl) {
    if (dl == NULL) {
        return;
    }
    free(dl);
    return;
}

size_t get_doubles_num(const struct doubles_list *dl) {
    if (dl == NULL) {
        return 0;
    }
    return dl->doubles_num;
}

double get_double_from_doubles_list(const struct doubles_list *dl, size_t index) {
    if (dl == NULL) {
        return 0.0;
    }
    if (index < dl->doubles_num) {
        return dl->doubles[index];
    }
    return 0.0;
}

void set_double_from_doubles_list(struct doubles_list *dl, size_t index, double pt) {
    if (dl == NULL) {
        return;
    }
    if (index < dl->doubles_num) {
        dl->doubles[index] = pt;
    }
    return;
}
