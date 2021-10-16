#ifndef SET_H_
#define SET_H_
#include <stdlib.h>
#include <limits.h>

typedef unsigned short state_id_t;

typedef struct {
    size_t size;
    size_t capacity;
    unsigned short*ids;
} set;

typedef struct {
    set l;
    set r;
}set_tuple;


void inspect(const set *const q);
void set_insert(set *s, state_id_t id);

inline void merge(set *const s, const set *const q){
    for (size_t i = 0; i < q->size; i++) {
        set_insert(s, q->ids[i]);
    }
}

state_id_t set_pop(set *s);
int set_find(const set*const s, state_id_t id);
int set_eq(const set *const a, const set *const b);
set set_complement(const set *a, const set *b);
set set_iota(state_id_t fst, state_id_t last);

static inline void set_destroy(set *s) {
    free(s->ids);
    s->ids = NULL;
    s->size = 0;
}


#endif // SET_H_
