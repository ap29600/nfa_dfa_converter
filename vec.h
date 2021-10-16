#ifndef VEC_H_
#define VEC_H_
#include <stdlib.h>
#include <stdio.h>
#include "set.h"

typedef struct set_vec {
    size_t size;
    size_t capacity;
    set *data;
} set_vec;

void set_vec_insert(struct set_vec *V, set *s);
set set_vec_pop_back(struct set_vec *V);
int set_vec_find(struct set_vec *V, set *s);
set *find_owner(set_vec *V, state_id_t s );

#endif // VEC_H_
