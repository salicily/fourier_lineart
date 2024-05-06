#ifndef DOUBLESLIST_FOURIER_H_
#define DOUBLESLIST_FOURIER_H_

#include "../types/doubleslist.h"

double scalar_product(const struct doubles_list *dl, double (*base)(size_t,double), size_t mode);

void add_base_vector(struct doubles_list *dl, double (*base)(size_t,double), size_t mode, double k);

#endif
