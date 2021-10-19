#include "automata.h"
#include <stdio.h>

void loop_regex(struct nfa *a) {
  state_id_t head = a->end_id + 1;
  state_id_t tail = head + 1;

  line tail_line = {
      .id = a->end_id,
      .paths = P_VEC({.trigger = '\0', .end_state = tail},
                     {.trigger = '\0', .end_state = a->start_id}),
  };

  line head_line = {
      .id = head,
      .paths = P_VEC({.trigger = '\0', .end_state = tail},
                     {.trigger = '\0', .end_state = a->start_id}),
  };

  vec_insert_sorted(&a->t_matrix, &head_line);
  vec_insert_sorted(&a->t_matrix, &tail_line);

  a->start_id = head;
  a->end_id = tail;
}

void adjoin_regex(struct nfa *a, struct nfa *b) {
  // we assume that b was constructed after a and has no
  // ids in common with a.
  state_id_t head = b->end_id + 1;
  state_id_t tail = head + 1;

  // copy over the paths.
  ITER(line, l, &b->t_matrix) { vec_insert(&a->t_matrix, l); }

  // paths that connect head to the start of a and b
  line head_paths = {
      .id = head,
      .paths = P_VEC({.trigger = '\0', .end_state = a->start_id},
                     {.trigger = '\0', .end_state = b->start_id}),
  };
  // there should not be a line with this id in a.
  vec_insert(&a->t_matrix, &head_paths);

  // a's and b's end path has no paths exiting from it, so we can just add the
  // lines.
  path tail_path = {.trigger = '\0', .end_state = tail};
  line a_line = {.id = a->end_id, .paths = P_VEC(tail_path)};
  line b_line = {.id = b->end_id, .paths = P_VEC(tail_path)};

  vec_insert_sorted(&a->t_matrix, &a_line);
  vec_insert_sorted(&a->t_matrix, &b_line);

  a->start_id = head;
  a->end_id = tail;

  destroy(&b->t_matrix);
  *b = (struct nfa){0};
}

void concatenate_regex(struct nfa *a, struct nfa *b) {
  if (a->t_matrix.size == 0) {
    destroy(&a->t_matrix);
    *a = *b;
    *b = (struct nfa){0};
    return;
  }

  line l = {.id = a->end_id, .paths = {0}};
  line *ll = vec_find(&a->t_matrix, &l);

  path p = {.trigger = '\0', .end_state = b->start_id};
  if (ll) {
    vec_insert_sorted(&ll->paths, &p);
  } else {
    l.paths = P_VEC(p);
    vec_insert(&a->t_matrix, &l);
  }

  ITER(line, ll, &b->t_matrix) { vec_insert(&a->t_matrix, ll); }

  a->end_id = b->end_id;

  destroy(&b->t_matrix);
  *b = (struct nfa){0};
}

struct nfa regex_to_nfa(const char *regex, size_t regex_len) {

  struct nfa result = {.t_matrix = L_VEC(), .start_id = 0, .end_id = 0};
  struct nfa tmp = {.t_matrix = L_VEC(), .start_id = 0, .end_id = 0};

  size_t depth;
  size_t j;
  for (size_t i = 0; i < regex_len; i++) {
    switch (regex[i]) {
    case '|':
      if (tmp.t_matrix.size > 0)
        concatenate_regex(&result, &tmp);

      tmp = regex_to_nfa(regex + i + 1, regex_len - i - 1);

      tmp.start_id += result.end_id;
      tmp.end_id += result.end_id;
      ITER(line, l, &tmp.t_matrix) {
        l->id += result.end_id;
        ITER(path, p, &l->paths) { p->end_state += result.end_id; }
      }

      adjoin_regex(&result, &tmp);
      return result;
    case '(':
      depth = 1;
      for (j = i + 1; j < regex_len && depth > 0; j++) {
        switch (regex[j]) {
        case '(':
          depth++;
          break;
        case ')':
          depth--;
          break;
        default:
          break;
        }
      }

      assert(depth == 0 && "unclosed parentheses");

      if (tmp.t_matrix.size > 0)
        concatenate_regex(&result, &tmp);

      tmp = regex_to_nfa(regex + i + 1, j - i - 2);

      // update the state ids;
      tmp.start_id += result.end_id;
      tmp.end_id += result.end_id;
      ITER(line, l, &tmp.t_matrix) {
        l->id += result.end_id;
        ITER(path, p, &l->paths) { p->end_state += result.end_id; }
      }

      i = j - 1; // consume up to the last parentheses;
      break;
    case ')':
      assert(0 && "Closing parentheses without opening.");
    case '*':
      loop_regex(&tmp);
      break;
    case '\0':
      assert(0 && "The string is shorter than expected");
    default:
      if (tmp.t_matrix.size > 0)
        concatenate_regex(&result, &tmp);
      tmp = (struct nfa){
          .t_matrix = L_VEC({
              .id = result.end_id + 1,
              .paths = P_VEC({
                  .trigger = regex[i],
                  .end_state = result.end_id + 2,
              }),
          }),
          .start_id = result.end_id + 1,
          .end_id = result.end_id + 2,
      };
      break;
    }
  }

  if (tmp.t_matrix.size > 0)
    concatenate_regex(&result, &tmp);

  return result;
}
