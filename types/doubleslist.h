#ifndef DOUBLESLIST_H_
#define DOUBLESLIST_H_

#include <stdint.h>
#include <stddef.h>

struct doubles_list;

struct doubles_list *create_doubles_list(size_t doubles_num);

void destroy_doubles_list(struct doubles_list *dl);

size_t get_doubles_num(const struct doubles_list *dl);

double get_double_from_doubles_list(const struct doubles_list *dl, size_t index);

void set_double_from_doubles_list(struct doubles_list *dl, size_t index, double d);

#endif
