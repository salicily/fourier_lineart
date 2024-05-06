#include "homothetie.h"

struct doubles_list *homothetie(struct doubles_list *l, double scale, double shift) {
    if (l == NULL) {
        return NULL;
    }
    size_t items = get_doubles_num(l);
    struct doubles_list *res = create_doubles_list(items);
    if (res == NULL) {
        return NULL;
    }
    for (size_t i = 0; i < items; ++i) {
        double d = get_double_from_doubles_list(l, i);
        d *= scale;
        d += shift;
        set_double_from_doubles_list(res, i, d);
    }
    return res;
}
