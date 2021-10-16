#include "vec.h"
#define assert(X) do{ \
    if(!(X)) { \
        printf("assertion failed: (" #X ")\n" );\
        exit(1);\
    } \
} while(0);

set set_vec_pop_back(struct set_vec *V) {
    if (!V->size) {
        fprintf(stderr, "popping from empty vector\n");
        exit(1);
    }

    V->size--;
    return V->data[V->size];
}

void set_vec_insert(struct set_vec *V, set *s) {
    if (V->capacity <= V->size) {
        V->capacity *= 2;
        V->capacity += 1;
        V->data = realloc(V->data, V->capacity * sizeof(V->data[0]));
    }
    V->data[V->size++] = *s;
}

int set_vec_find(struct set_vec *V, set *s) {
    for (size_t i = 0; i<V->size; i++) {
        if (set_eq(&V->data[i], s))
            return i;
    }
    return 0;
}

set *find_owner(set_vec *V, state_id_t s ) {
    for(size_t i = 0; i<V->size; i++) {
        if (set_find(&V->data[i], s))
            return &V->data[i];
    }
    assert(0 && "state has no owner");
}
