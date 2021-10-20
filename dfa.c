#include "thompson.h"
#include "automata.h"
#include "util.h"
// #include "vec.h"
#include <stdio.h>
#include <stdlib.h>

#define FIRST_EXAMPLE_

void dump_nfa_to_dot(nfa *N, const char *filename) {
  FILE *out = fopen(filename, "w");
  assert(N);
  fprintf(out, "digraph {\n");
  fprintf(out, "  node [shape = circle]\n");
  fprintf(out, "  d%u [shape = record];\n", N->start_id);
  fprintf(out, "  d%u [shape = doublecircle];\n", N->end_id);

  ITER(line, start, &N->t_matrix) {
    ITER(path, p, &start->paths) {
      if (p->trigger == '\0') {
        fprintf(out, "  d%u -> d%u [label = \"'eps'\", style=dashed];\n",
                start->id, p->end_state);
      } else {
        fprintf(out, "  d%u -> d%u [label = \"%c\"];\n", start->id,
                p->end_state, p->trigger);
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

  state_id_t start_id = 1;
  if (vec_find(&D->accepting_states, &start_id)) {
      fprintf(out, "  d1 [shape = Msquare]\n");
  } else {
      fprintf(out, "  d1 [shape = square]\n");
  }

  ITER(state_id_t, id, &D->accepting_states)
    if(*id != 1)
      fprintf(out, " d%u [shape = doublecircle];\n", *id);


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
  char regex [1024];

  printf("Please insert the regular expression to translate:\n>>> ");
  scanf("%s", regex);

  nfa A = regex_to_nfa(regex, strlen(regex));
  dump_nfa_to_dot(&A, "initial.dot");
  dfa *D = to_dfa(&A);
  dump_dfa_to_dot(D, "intermediate.dot");
  dfa *R = minimize(D);
  dump_dfa_to_dot(R, "minimised.dot");
}
