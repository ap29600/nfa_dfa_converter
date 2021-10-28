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

nfa clone_regex(const nfa *n) {
  state_id_t offset = n->end_id;
  nfa result = {
      .start_id = n->start_id + offset,
      .end_id = n->end_id + offset,
      .t_matrix = L_VEC(),
  };

  ITER(line, l, &n->t_matrix) {
    line ll = {
        .id = l->id + offset,
        .paths = P_VEC(),
    };
    ITER(path, p, &l->paths) {
      path pp = {.trigger = p->trigger, .end_state = p->end_state + offset};
      vec_insert_sorted(&ll.paths, &pp);
    }
    vec_insert_sorted(&result.t_matrix, &ll);
  }

  return result;
}

const char escape_sequences[] = {
    ['n'] = '\n',
    ['t'] = '\t',
    ['s'] = ' ',
    ['('] = '(',
    [')'] = ')',
    ['*'] = '*',
    ['+'] = '+',
    ['['] = '[',
    [']'] = ']',
};

struct nfa regex_to_nfa(const char *regex, size_t regex_len) {

  struct nfa result = {.t_matrix = L_VEC(), .start_id = 0, .end_id = 0};
  struct nfa tmp = {.t_matrix = L_VEC(), .start_id = 0, .end_id = 0};
  struct nfa tmp2;

  size_t depth;
  size_t j;
  int escaped = 0;
  for (size_t i = 0; i < regex_len; i++) {

    if (escaped) {
      escaped = 0;

      char c = escape_sequences[(unsigned char)regex[i]];

      if (c == '\0') {
        fprintf(stderr, "unknown char escape code: '\\%c' (%d)\n", c, c);
        exit(1);
      }

      if (tmp.t_matrix.size > 0)
        concatenate_regex(&result, &tmp);

      tmp = (struct nfa){
          .t_matrix = L_VEC({
              .id = result.end_id + 1,
              .paths = P_VEC({
                  .trigger = c,
                  .end_state = result.end_id + 2,
              }),
          }),
          .start_id = result.end_id + 1,
          .end_id = result.end_id + 2,
      };

      continue;
    }

    switch (regex[i]) {
    case '|':
      if (tmp.t_matrix.size > 0)
        concatenate_regex(&result, &tmp);
      else if (result.t_matrix.size == 0) {
        result.t_matrix = L_VEC({.id = 1,
                                 .paths = P_VEC({
                                     .trigger = '\0',
                                     .end_state = 2,
                                 })});
        result.start_id = 1;
        result.end_id = 2;
      }

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

        if (escaped) {
          escaped = 0;
          continue;
        }

        switch (regex[j]) {
        case '(':
          depth++;
          break;
        case ')':
          depth--;
          break;
        case '\\':
          escaped = 1;
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
    case '\\':
      escaped = 1;
      break;
    case '[':
      do {
        char start = regex[++i];
        assert(regex[++i] == '-');
        char end = regex[++i];
        assert(regex[++i] == ']');

        if (tmp.t_matrix.size > 0)
          concatenate_regex(&result, &tmp);

        line l = {
            .id = result.end_id + 1,
            .paths = P_VEC(),
        };

        for (char c = start; c <= end; c++) {
          path p = {.trigger = c, .end_state = result.end_id + 2};
          vec_insert_sorted(&l.paths, &p);
        }

        tmp = (struct nfa){
            .t_matrix = L_VEC(l),
            .start_id = result.end_id + 1,
            .end_id = result.end_id + 2,
        };

      } while (0);
      break;

    case ']':
      assert(0 && "Closing square brackets without opening.");

    case '*':
      loop_regex(&tmp);
      break;
    case '+':
      tmp2 = clone_regex(&tmp);
      loop_regex(&tmp2);
      concatenate_regex(&tmp, &tmp2);
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
