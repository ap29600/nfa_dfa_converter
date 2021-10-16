#ifndef AUTOMATA_H_
#define AUTOMATA_H_
#include <stdlib.h>
#include "set.h"


typedef struct transition_matrix {
    size_t size;
    size_t capacity;
    state_id_t (*data)[256];
} transition_matrix;

typedef struct {
    size_t n_states;

    // NFAs derived from Thompson's construction have 
    // at most 2 exiting epsilon-moves per state
    state_id_t(*epsilon)[2]; 

    transition_matrix T;
    set accepting_states;
} nfa;

typedef struct {
    state_id_t n_states;
    transition_matrix T;
    set accepting_states;
} dfa;

void transition_matrix_insert(transition_matrix *T, state_id_t start, unsigned char c, state_id_t dest);
set eps_closure(nfa *N, const set *in);
set delta(nfa *N, set *q, unsigned char c);
dfa *to_dfa(nfa *N);

dfa *minimize(dfa *D);

#endif // AUTOMATA_H_
