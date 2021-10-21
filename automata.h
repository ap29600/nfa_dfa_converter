#ifndef AUTOMATA_H_
#define AUTOMATA_H_
#include <stdlib.h>
#include "util.h"

typedef struct {
  state_id_t id;
  vector paths;
} line;

typedef struct {
  unsigned char trigger;
  state_id_t end_state;
} path;

int line_cmp(const void *a, const void *b);
int path_cmp(const void *a, const void *b);

#define L_VEC(...) VEC(line, line_cmp, ##__VA_ARGS__)
#define P_VEC(...) VEC(path, path_cmp, ##__VA_ARGS__)

typedef struct transition_matrix {
    size_t size;
    size_t capacity;
    state_id_t (*data)[256];
} transition_matrix;
/*
typedef struct {
    size_t n_states;

    // NFAs derived from Thompson's construction have 
    // at most 2 exiting epsilon-moves per state
    state_id_t(*epsilon)[2]; 

    transition_matrix T;
    vector accepting_states;
} nfa;
*/

typedef struct nfa {
  vector t_matrix;
  state_id_t start_id;
  state_id_t end_id;
} nfa ;


typedef struct {
    state_id_t n_states;
    vector t_matrix;
    bit_set accepting_states;
} dfa;

void transition_matrix_insert(vector*T, state_id_t start, unsigned char c, state_id_t dest);
bit_set eps_closure_(nfa *N, const bit_set *in);
vector eps_closure(nfa *N, const vector *in);
bit_set delta_(nfa *N, bit_set *q, unsigned char c);
vector delta(nfa *N, vector *q, unsigned char c);
dfa *to_dfa(nfa *N);

dfa *minimize(dfa *D);

#endif // AUTOMATA_H_
