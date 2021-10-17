#ifndef AUTOMATA_H_
#define AUTOMATA_H_
#include <stdlib.h>
#include "set.h"
#include "util.h"

typedef struct {
  state_id_t id;
  vector paths;
} line;

typedef struct {
  unsigned char trigger;
  state_id_t end_state;
} path;

static int line_cmp(const void *a, const void *b) {
  return ((line *)a)->id - ((line *)b)->id;
}
static int path_cmp(const void *a, const void *b) {
  return ((path *)a)->trigger - ((path *)b)->trigger;
}

#define L_VEC(...) VEC(line, line_cmp, ##__VA_ARGS__)
#define P_VEC(...) VEC(path, path_cmp, ##__VA_ARGS__)

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
    vector t_matrix;
    set accepting_states;
} dfa;

void transition_matrix_insert(vector*T, state_id_t start, unsigned char c, state_id_t dest);
set eps_closure(nfa *N, const set *in);
set delta(nfa *N, set *q, unsigned char c);
dfa *to_dfa(nfa *N);

dfa *minimize(dfa *D);

#endif // AUTOMATA_H_
