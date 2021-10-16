#include "automata.h"
#include "set.h"
#include "vec.h"
#include "string.h"

#define UNIMPLEMENTED {fprintf(stderr, "UNIMPLEMENTED: %s\n", __func__); exit(1);}

#define assert(X) do{ \
    if(!(X)) { \
        printf("assertion failed: (" #X ")\n" );\
        exit(1);\
    } \
} while(0);

void transition_matrix_insert(transition_matrix *T, state_id_t start, unsigned char c, state_id_t dest) {
    if (T->capacity <= start) {
        size_t new_cap = 2 * start + 1;
        void*p = calloc(new_cap * sizeof(T->data[0]), 1);
        assert(p != NULL);
        memcpy(p, T->data, T->capacity * sizeof(T->data[0]));
        free(T->data);
        T->data = p;
        T->capacity = new_cap;
    }
    T->data[start][c] = dest;
}

dfa *to_dfa(nfa *N) {
    dfa *result  = calloc(sizeof(dfa), 1);

    set err_state = {0};
    set_insert(&err_state, 0);
    
    set n0 = {0};
    set_insert(&n0, 1); // valid states are 1-indexed, state 0 is ERR
    set q0 = eps_closure(N, &n0);

    struct set_vec Q = {0};
    struct set_vec Wl = {0};

    set_vec_insert(&Q, &err_state);
    set_vec_insert(&Q, &q0);
    set_vec_insert(&Wl, &q0);

    while (Wl.size) {
        set q = set_vec_pop_back(&Wl);
        state_id_t id_source = set_vec_find(&Q, &q);

        for (unsigned c = 0; c < 256; c++) {
            set tmp = delta(N, &q, c);

            if (!tmp.size) {
                set_destroy(&tmp);
                continue;
            }

            set t = eps_closure(N, &tmp);
            set_destroy(&tmp);

            state_id_t id_dest;
            if (!(id_dest = set_vec_find(&Q, &t))) {
                set_vec_insert(&Q, &t);
                set_vec_insert(&Wl, &t);
                id_dest = Q.size - 1;
            }

            transition_matrix_insert(&result->T, id_source, c, id_dest);
        }
    }

    result->n_states = Q.size; // 1 for the ERR state

    for ( size_t i = 0; i < result->n_states; i++) {
        for (size_t j = 0; j < Q.data[i].size; j++) {
            if (set_find(&N->accepting_states, Q.data[i].ids[j])) {
                set_insert(&result->accepting_states, i);
            }
        }
    };

    return result;
} 

set delta(nfa *N, set *q, unsigned char c) {
    set result = {0};
    for (size_t i = 0; i<q->size; i++) {
        state_id_t id = q->ids[i];
        if (N->T.data[id][c]) {
            set_insert(&result, N->T.data[id][c]);
        }
    }
    return result;
}

set eps_closure(nfa *N, const set *in) {
    set result = {0}, Wl = {0}; 

    for(size_t i = 0; i < in->size; i++){
        set_insert(&result, in->ids[i]);
        set_insert(&Wl, in->ids[i]);
    }

    while (Wl.size > 0) {
        state_id_t id = set_pop(&Wl);

        for(size_t i = 0; i<2; i++) {
            state_id_t j;
            if ((j = N->epsilon[id][i]) > 0 && !set_find(&result, j)) {
                set_insert(&result, j);
                set_insert(&Wl, j);
            }
        }
    }
    set_destroy(&Wl);
    return result;
}

set_tuple split(dfa *D, set_vec *P, set *s) {
    set_tuple result = {0};

    assert(s->size);

    for (unsigned c = 0; c < 256; c++) {
        int expect = 0;

        assert(D->T.capacity > s->ids[0]);
        state_id_t dest = D->T.data[s->ids[0]][c];


        if (dest == 0)
            continue;

        for(size_t i = 0; i<P->size; i++) {
            if (set_find (&P->data[i], dest)) {
                expect = i;
                break;
            }
        };

        assert(expect >= 0);

        for (state_id_t i = 1; i<s->size; i++) {
            assert(D->T.capacity > s->ids[i]);
            state_id_t dest;
            if ((dest = D->T.data[s->ids[i]][c]) > 0 && set_find (&P->data[expect], dest)) {
                continue;
            }

            // now all the values up to i-1 belong to the set at [expect].
            for (state_id_t j = 0; j < i; j++) {
                set_insert(&result.l, s->ids[j]);
            }

            // value at [i] does not.
            set_insert(&result.r, s->ids[i]);

            // the others still need to be checked.
            for (state_id_t j = i+1; j < s->size; j++) {
                assert(D->T.capacity > s->ids[j]);
                if ((dest = D->T.data[s->ids[j]][c]) > 0 && set_find(&P->data[expect], dest))
                    set_insert(&result.l, s->ids[j]);
                else 
                    set_insert(&result.r, s->ids[j]);
            }
            return result;
        }
    }
    result.l = *s;
    return result;
}

dfa *minimize(dfa *D) {
    set_vec T = {0};
    set_vec P = {0};

    set elems = set_iota(1, D->n_states - 1);
    set c = set_complement(&elems, &D->accepting_states);

    set_vec_insert(&T, &c);
    set_vec_insert(&T, &D->accepting_states);

    while (T.size > P.size) {
        free (P.data);
        P = T;
        T = (set_vec){0};

        for(size_t i = 0; i<P.size; i++) {
            set_tuple parts = split(D, &P, &P.data[i]);
            set_vec_insert(&T, &parts.l);
            if (parts.r.size > 0)
                set_vec_insert(&T, &parts.r);
        }
    }

    dfa *R = calloc(sizeof(dfa), 1);

    R->n_states = T.size + 1;

    for(state_id_t i = 1; i < R->n_states; i++) {

        set *start = &T.data[i-1];
        assert(start->size);
        state_id_t start_id = start->ids[0];
        assert(D->T.capacity > start_id);

        if (set_find(&D->accepting_states, start_id)) {
            set_insert(&R->accepting_states, i);
        }

        for (unsigned c = 0; c < 256; c++) {
            state_id_t end_id = D->T.data[start_id][c];
            for(state_id_t j = 1; j < T.size + 1; j++) {
                if (set_find(&T.data[j - 1], end_id)) {
                    transition_matrix_insert(&R->T, i, c, j);
                    goto next;
                }
            }
next:;
        }
    }

    return R;
}


