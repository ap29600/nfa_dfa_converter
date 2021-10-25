#include "automata.h"
#include "thompson.h"
#include "util.h"

void scanner_from_dfa(dfa *D, const char *scanner_name, FILE *stream) {
  fprintf(stream, "unsigned long scan_%s (const char *s) {\n", scanner_name);
  fprintf(stream, "  unsigned last_accepting = 0;\n"
                  "  unsigned char c;\n"
                  "  unsigned long count = 0;\n");

  for (state_id_t i = 1; i < D->n_states; i++) {
    line key = {.id = i};
    line *ll = vec_find(&D->t_matrix, &key);
    fprintf(stream, "s_%u:\n", i);

    if (set_has(&D->accepting_states, i))
      fprintf(stream, "  last_accepting = count;\n");
    fprintf(stream, "  c = s[count++];\n");

    if (ll) {
      fprintf(stream, "  switch (c) {\n");
      ITER(path, p, &ll->paths)
      fprintf(stream, "    case %u: goto s_%u;\n", p->trigger, p->end_state);
      fprintf(stream, "    default: goto s_out;\n");
      fprintf(stream, "  }\n");
    } else {
      fprintf(stream, "  goto s_out;\n");
    }
  }
  fprintf(stream, "s_out: return last_accepting;\n");
  fprintf(stream, "}\n");
}

void scanner_from_regex(const char *regex, const char *scanner_name, FILE *stream) {
  nfa initial = regex_to_nfa(regex, strlen(regex));
  dfa *intermediate = to_dfa(&initial);
  dfa *minimal = minimize(intermediate);
  scanner_from_dfa(minimal, scanner_name, stream);
}
