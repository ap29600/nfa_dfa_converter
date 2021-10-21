#include "automata.h"
#include "string.h"
#include "util.h"
#include <stdio.h>

int line_cmp(const void *a, const void *b) {
  return ((line *)a)->id - ((line *)b)->id;
}
int path_cmp(const void *a, const void *b) {
  return ((path *)a)->trigger - ((path *)b)->trigger;
}

unsigned transition_matrix_find(vector *matrix, state_id_t row,
                                unsigned char col) {
  line l = {row, P_VEC()};
  const path p = {col, 0};

  const line *ll = vec_find_sorted(matrix, &l);
  destroy(&l.paths);
  if (!ll)
    return 0;

  const path *pp = vec_find_sorted(&ll->paths, &p);
  if (!pp)
    return 0;

  return pp->end_state;
}

void transition_matrix_insert(vector *matrix, state_id_t start, unsigned char c,
                              state_id_t dest) {
  const path p = {c, dest};
  line l = {start, P_VEC()};

  line *ll = vec_find_sorted(matrix, &l);
  if (!ll) {
    vec_insert_sorted(&l.paths, &p);
    vec_insert_sorted(matrix, &l);
    return;
  }
  // only freed if not inserted into the matrix as a line
  destroy(&l.paths);

  path *pp = vec_find_sorted(&ll->paths, &p);
  if (!pp) {
    vec_insert_sorted(&ll->paths, &p);
    return;
  }
  pp->end_state = dest;
}

int set_cmp(const void *a, const void *b) {
  const bit_set *aa = a;
  const bit_set *bb = b;
  return memcmp(aa->data, bb->data, BS_N_BLOCKS * sizeof(bitset_block_t));
}

#define S_VEC(...) VEC(bit_set, set_cmp, ##__VA_ARGS__)
#define SET(...) VEC(state_id_t, st_cmp, ##__VA_ARGS__)

dfa *to_dfa(nfa *N) {
  dfa *result = calloc(sizeof(dfa), 1);
  result->t_matrix = L_VEC();

  bit_set err_state = {.data = {1 << 0}};
  bit_set n0 = {0};
  set_insert(&n0, N->start_id);

  bit_set q0 = eps_closure_(N, &n0);

  vector Q = S_VEC(err_state, q0);
  vector Wl = S_VEC(q0);

  while (Wl.size) {

    bit_set q;
    vec_pop_back(&Wl, &q);

    bit_set *source = vec_find(&Q, &q);
    // we should be able to find the element in Q,
    // since Wl is a subset of Q.
    assert(source != NULL);
    state_id_t id_source = index_of(&Q, source);

    for (unsigned c = 1; c < 256; c++) {

      bit_set tmp = delta_(N, &q, c);
      if (empty(&tmp))
        continue;

      bit_set t = eps_closure_(N, &tmp);
      bit_set *dest_p = vec_find(&Q, &t);

      state_id_t id_dest;
      if (dest_p)
        id_dest = index_of(&Q, dest_p);
      else {
        vec_insert(&Q, &t);
        vec_insert(&Wl, &t);
        id_dest = Q.size - 1;
      }

      transition_matrix_insert(&result->t_matrix, id_source, c, id_dest);
    }
  }

  result->n_states = Q.size; // 1 for the ERR state
  result->accepting_states = (bit_set){0};

  ITER(bit_set, q, &Q) {
    state_id_t dfa_id = index_of(&Q, q);
    ITERATE_BITSET(nfa_id, *q) {
      if (nfa_id == N->end_id) {
        set_insert(&result->accepting_states, dfa_id);
        goto next_iter;
        // there are a few macros in the way, so `break` won't
        // work here.
      }
    }
  next_iter:;
  }
  return result;
}

vector delta(nfa *N, vector *q, unsigned char c) {
  vector result = SET();
  ITER(state_id_t, id, q) {
    line key = {.id = *id};
    line *l = vec_find(&N->t_matrix, &key);
    if (l) {
      ITER(path, p, &l->paths) {
        if (p->trigger == c) {
          vec_insert_sorted(&result, &p->end_state);
        }
      }
    }
  }
  return result;
}

bit_set delta_(nfa *N, bit_set *q, unsigned char c) {
  bit_set result = {0};
  ITERATE_BITSET(id, *q) {
    line key = {.id = id};
    line *l = vec_find(&N->t_matrix, &key);
    if (l) {
      ITER(path, p, &l->paths) {
        if (p->trigger == c) {
          set_insert(&result, p->end_state);
        }
      }
    }
  }
  return result;
}

bit_set eps_closure_(nfa *N, const bit_set *in) {
  bit_set result = *in;
  bit_set worklist = *in;

  state_id_t start_id;
  while ((start_id = set_pop(&worklist)) != 0) {
    line key = {.id = start_id};
    // here we could skip the construction of the key.
    // since we only check the id, &start_id looks like a valid line*.
    line *l = vec_find(&N->t_matrix, &key);
    if (l) {
      ITER(path, p, &l->paths) {
        if (p->trigger == '\0' && !set_has(&result, p->end_state)) {
          set_insert(&result, p->end_state);
          set_insert(&worklist, p->end_state);
        }
      }
    }
  }
  return result;
}

vector eps_closure(nfa *N, const vector *in) {
  assert(in->elem_size == sizeof(state_id_t));

  // TODO: introduce vector clone to make this better.
  vector result = SET();
  vector work_list = SET();

  ITER(state_id_t, i, in) {
    vec_insert_sorted(&result, i);
    vec_insert(&work_list, i);
  }

  while (work_list.size > 0) {
    state_id_t start_id = 0;
    vec_pop_back(&work_list, &start_id);

    line key = {.id = start_id};
    line *l = vec_find(&N->t_matrix, &key);
    if (l) {
      ITER(path, p, &l->paths) {
        if (p->trigger == '\0' && !vec_find_sorted(&result, &p->end_state)) {
          vec_insert_sorted(&result, &p->end_state);
          vec_insert(&work_list, &p->end_state);
        }
      }
    }
  }
  destroy(&work_list);
  return result;
}

typedef struct {
  vector l;
  vector r;
} vec_tuple;

typedef struct {
  bit_set l;
  bit_set r;
} set_tuple;

set_tuple split(dfa *D, vector *P, const bit_set *s) {
  set_tuple result = {0};
  assert(!empty(s));

  for (unsigned c = 1; c < 256; c++) {
    bit_set *expect = NULL;
    int first_iter = 1;
    state_id_t dest;

    ITERATE_BITSET(id, *s) {
      dest = transition_matrix_find(&D->t_matrix, id, c);
      if (first_iter) { // set the expectation
        ITER(bit_set, b, P) {
          assert(b);
          if (set_has(b, dest)) {
            expect = b;
            break;
          }
        }
        first_iter = 0;
      } else { // check that every id meets the expectation
        if ((expect && !dest) || (!expect && dest) ||
            (expect && !set_has(expect, dest))) {
          goto split_set;
        }
      }
    }
    continue;

  split_set:
    printf("split on char %c: ", c);
    inspect(s);
    printf("\n");
    ITERATE_BITSET(id, *s) {
      dest = transition_matrix_find(&D->t_matrix, id, c);
      if ((expect && !dest) || (!expect && dest) ||
          (expect && !set_has(expect, dest))) {
        set_insert(&result.r, id);
      } else {
        set_insert(&result.l, id);
      }
    }
    printf("results in: ");
    inspect(&result.l);
    inspect(&result.r);
    printf("\n");

    return result;
  }

  result.l = *s;
  result.r = (bit_set){0};
  return result;
}

dfa *minimize(dfa *D) {

  bit_set elems = set_iota(1, D->n_states - 1);
  bit_set c = set_complement(&elems, &D->accepting_states);
  vector T;
  vector P;

  // TODO: clean up this mess.
  // currently the minimisation roughly preserves order of the states, so the
  // first state of the first set is kept there.  it would be nice not to
  // depend on this behaviour.
  if (!empty(&c)) {
    state_id_t start = 1;
    if (set_has(&D->accepting_states, start))
      T = S_VEC(D->accepting_states, c);
    else
      T = S_VEC(c, D->accepting_states);
  } else
    T = S_VEC(D->accepting_states);

  P = S_VEC();

  while (T.size > P.size) {
    destroy(&P);

    P = T;
    T = S_VEC();

    ITER(bit_set, s, &P) {
      set_tuple parts = split(D, &P, s);
      assert(!empty(&parts.l));
      vec_insert(&T, &parts.l);
      if (!empty(&parts.r)) {
        vec_insert(&T, &parts.r);
      }
    }
  }

  dfa *R = calloc(sizeof(dfa), 1);
  R->t_matrix = L_VEC();

  R->n_states = T.size + 1;
  R->accepting_states = (bit_set){0};


  ITER(bit_set, b, &T) {
    state_id_t start_id =
        index_of(&T, b) + 1; // +1 leaves space for ERR state = 0
    assert(!empty(b));
    state_id_t elem = set_peek(b);
    if (set_has(&D->accepting_states, elem)) {
      set_insert(&R->accepting_states, start_id);
    }

    // all elements of this set should have the same
    // destination as each other for every char, so we can just check
    // for the first.
    line key = {.id = elem};
    line *ll = vec_find(&D->t_matrix, &key);

    if (ll) {
      ITER(path, p, &ll->paths) {
        ITER(bit_set, q, &T) {
          if (set_has(q, p->end_state)) {
            transition_matrix_insert(&R->t_matrix, start_id,
                                     p->trigger, index_of(&T, q) + 1);
            break;
          }
        }
      }
    }
  }

  return R;
}
