#include "automata.h"
#include "scanner_generator.h"
#include "thompson.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include "dfa.h"

#define FIRST_EXAMPLE_

void dump_nfa_to_dot(nfa *N, FILE *stream) {
  assert(N);
  fprintf(stream, "digraph {\n");
  fprintf(stream, "  node [shape = circle]\n");
  fprintf(stream, "  d%u [shape = record];\n", N->start_id);
  fprintf(stream, "  d%u [shape = doublecircle];\n", N->end_id);

  ITER(line, start, &N->t_matrix) {
    ITER(path, p, &start->paths) {
      if (p->trigger == '\0') {
        fprintf(stream, "  d%u -> d%u [label = \"'eps'\", style=dashed];\n",
                start->id, p->end_state);
      } else {
        fprintf(stream, "  d%u -> d%u [label = \"%c\"];\n", start->id,
                p->end_state, p->trigger);
      }
    }
  }
  fprintf(stream, "}\n");
}

void dump_dfa_to_dot(dfa *D, FILE *stream) {
  assert(D);
  fprintf(stream, "digraph {\n");
  fprintf(stream, "  node [shape = circle]\n");

  state_id_t start_id = 1;
  if (set_has(&D->accepting_states, start_id)) {
    fprintf(stream, "  d1 [shape = Msquare]\n");
  } else {
    fprintf(stream, "  d1 [shape = square]\n");
  }

  ITERATE_BITSET(id, D->accepting_states) {
    if (id != 1)
      fprintf(stream, " d%u [shape = doublecircle];\n", id);
  }

  ITER(line, start, &D->t_matrix) {
    ITER(path, p, &start->paths) {
      fprintf(stream, " d%u -> d%u [label = \"%c\"];\n", start->id, p->end_state,
              p->trigger);
    }
  }

  fprintf(stream, "}\n");
}
