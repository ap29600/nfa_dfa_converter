#include "set.h"
#include <stdio.h>

#define UNIMPLEMENTED {fprintf(stderr, "UNIMPLEMENTED: %s\n", __func__); exit(1);}

#define assert(X) do{ \
    if(!(X)) { \
        printf("assertion failed: (" #X ")\n" );\
        exit(1);\
    } \
} while(0);


void set_insert(set *s, state_id_t id) {
    assert(!set_find(s, id));
    if (s->capacity <= s->size) {
        s->capacity *= 2;
        s->capacity += 1;
        s->ids = realloc(s->ids, s->capacity * sizeof(s->ids[0]));
    }
    s->size += 1;
    s->ids[s->size - 1] = id;
    for(size_t i = s->size - 1; i > 0 && s->ids[i-1] > s->ids[i] ; i--) {
        state_id_t tmp = s->ids[i];
        s->ids[i] = s->ids[i-1];
        s->ids[i-1] = tmp;
    }
}

int set_find(const set*const s, state_id_t id) {
    for(size_t i = 0; i < s->size; i++)
        if (s->ids[i] == id) return 1;
    return 0;
}

void inspect(const set *const q) {
    printf("set{");
    for(size_t i = 0; i<q->size; i++) {
        printf("%d%s", 
                q->ids[i], 
                (i == q->size - 1) ? "" : ","
                );
    }
    printf("}");
}

int set_eq(const set *const a, const set *const b) {
    if (a->size != b->size)
        return 0;
    for(size_t i = 0; i<a->size; i++) {
        if (a->ids[i] != b->ids[i])
            return 0;
    }
    return 1;
}

state_id_t set_pop(set *s) {
    assert(s->size > 0);
    return s->ids[--s->size];
}

set set_complement(const set *a, const set *b) {
    set result = {0};
    size_t i = 0;
    size_t j = 0;
    for(; i < a->size && j < b->size;) {
        if (a->ids[i] < b->ids[j])
            set_insert(&result, a->ids[i++]);
        else if (a->ids[i] > b->ids[j])
            j++;
        else
            i++,  j++;
    }
    return result;
}

set set_iota(state_id_t fst, state_id_t last) {
    set res = {
        .size = last - fst + 1,
        .capacity = last - fst + 1,
        .ids = malloc((last - fst + 1) * sizeof(res.ids[0]))
    };
    for(size_t i = fst; i<=last; i++)
        res.ids[i - fst] = i;
    return res;
}
