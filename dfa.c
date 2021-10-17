#include "automata.h"
#include "set.h"
#include "util.h"
// #include "vec.h"
#include <stdio.h>
#include <stdlib.h>

#define FIRST_EXAMPLE_

#ifdef FIRST_EXAMPLE_
#define A_STATES 11
state_id_t(delta_A[A_STATES])[256] = {
    [0] = {0}, 
    [1] = {['a'] = 2}, 
    [2] = {0},  
    [3] = {0},
    [4] = {0}, 
    [5] = {['b'] = 6}, 
    [6] = {0},  
    [7] = {['c'] = 8},
    [8] = {0}, 
    [9] = {0},         
    [10] = {0},
};
state_id_t(epsilon_A[A_STATES])[2] = {
    [0] = {0, 0}, 
    [1] = {0, 0},  
    [2] = {3, 0},  
    [3] = {4, 10},
    [4] = {5, 7}, 
    [5] = {0, 0},  
    [6] = {9, 0},  
    [7] = {0, 0},
    [8] = {9, 0}, 
    [9] = {10, 4}, 
    [10] = {0, 0},
};

nfa A = {.n_states = A_STATES,
         .accepting_states = (set){.ids = (state_id_t[]){10}, .size = 1},
         .T.data = delta_A,
         .T.size = A_STATES,
         .T.capacity = A_STATES,
         .epsilon = epsilon_A};

#else

#define A_STATES 11
state_id_t(delta_A[A_STATES])[256] = {
    [0] = {0},         
    [1] = {['t'] = 2, ['h'] = 7},
    [2] = {['h'] = 3}, 
    [3] = {['e'] = 4},
    [4] = {['r'] = 5}, 
    [5] = {['e'] = 6},
    [6] = {0},         
    [7] = {['e'] = 8},
    [8] = {['r'] = 9}, 
    [9] = {['e'] = 10},
    [10] = {0},
};

state_id_t(epsilon_A[A_STATES])[2] = {
    [0] = {0, 0}, 
    [1] = {0, 0}, 
    [2] = {0, 0},  
    [3] = {0, 0},
    [4] = {0, 0}, 
    [5] = {0, 0}, 
    [6] = {0, 0},  
    [7] = {0, 0},
    [8] = {0, 0}, 
    [9] = {0, 0}, 
    [10] = {0, 0},
};

nfa A = {.n_states = A_STATES,
         .accepting_states = (set){.ids = (state_id_t[]){6, 10}, .size = 2},
         .T.data = delta_A,
         .T.size = A_STATES,
         .T.capacity = A_STATES,
         .epsilon = epsilon_A};

#endif // FIRST_EXAMPLE

void dump_nfa_to_dot(nfa *N, const char *filename) {
  FILE *out = fopen(filename, "w");
  assert(N);
  fprintf(out, "digraph {\n");
  fprintf(out, "  node [shape = circle]\n");
  for (size_t i = 0; i < N->accepting_states.size; i++) {
    fprintf(out, " d%u [shape = doublecircle];\n", N->accepting_states.ids[i]);
  }
  for (size_t i = 0; i < N->T.capacity; i++) {
    for (unsigned c = 0; c < 256; c++) {
      if (N->T.data[i][c] > 0) {
        fprintf(out, " d%zu -> d%u [label = \"%c\"];\n", i, N->T.data[i][c], c);
      }
    }

    for (unsigned j = 0; j < 2; j++) {
      if (N->epsilon[i][j] > 0) {
        fprintf(out, " d%zu -> d%u [label = \"eps\" style=dashed];\n", i,
                N->epsilon[i][j]);
      }
    }
  }
  fprintf(out, "}\n");
  fclose(out);
}

void dump_dfa_to_dot(dfa *D, const char *filename) {
  FILE *out = fopen(filename, "w");
  assert(D);
  fprintf(out, "digraph {\n");
  fprintf(out, "  node [shape = circle]\n");
  for (size_t i = 0; i < D->accepting_states.size; i++) {
    fprintf(out, " d%u [shape = doublecircle];\n", D->accepting_states.ids[i]);
  }

  ITER(line, start, &D->t_matrix) {
    ITER(path, p, &start->paths) {
      fprintf(out, " d%u -> d%u [label = \"%c\"];\n", start->id, p->end_state,
              p->trigger);
    }
  }

  fprintf(out, "}\n");
  fclose(out);
}

int main() {
  dump_nfa_to_dot(&A, "initial.dot");
  dfa *D = to_dfa(&A);
  dump_dfa_to_dot(D, "intermediate.dot");
  dfa *R = minimize(D);
  dump_dfa_to_dot(R, "minimised.dot");
}
