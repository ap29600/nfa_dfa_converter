#include "automata.h"
#include "string.h"
#include "util.h"
#include <stdio.h>

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
  const vector *A = a;
  const vector *B = b;

  assert(A->elem_size == sizeof(state_id_t));
  assert(B->elem_size == sizeof(state_id_t));

  for (size_t i = 0; i < A->size && i < B->size; i++) {
    if (A->compar(elem_at(A, i), elem_at(B, i)) != 0)
      return A->compar(elem_at(A, i), elem_at(B, i));
  }
  return A->size - B->size;
}

#define S_VEC(...) VEC(vector, set_cmp, ##__VA_ARGS__)
#define SET(...) VEC(state_id_t, st_cmp, ##__VA_ARGS__)

dfa *to_dfa(nfa *N) {
  dfa *result = calloc(sizeof(dfa), 1);
  result->t_matrix = L_VEC();

  vector err_state = SET(0);
  vector n0 = SET(1);
  vector q0 = eps_closure(N, &n0);

  vector Q = S_VEC(err_state, q0);
  vector Wl = S_VEC(q0);

  while (Wl.size) {
    vector q;
    vec_pop_back(&Wl, &q);

    vector *source = vec_find(&Q, &q);

    // we should be able to find the element in Q,
    // since Wl is a subset of Q.
    assert(source != NULL);
    state_id_t id_source = index_of(&Q, source);

    for (unsigned c = 1; c < 256; c++) {
      vector tmp = delta(N, &q, c);

      if (!tmp.size) {
        destroy(&tmp);
        continue;
      }

      vector t = eps_closure(N, &tmp);
      destroy(&tmp);

      vector *dest_p = vec_find(&Q, &t);
      state_id_t id_dest;
      if (!dest_p) {
        vec_insert(&Q, &t);
        vec_insert(&Wl, &t);
        id_dest = Q.size - 1;
      } else {
        id_dest = index_of(&Q, dest_p);
      }

      transition_matrix_insert(&result->t_matrix, id_source, c, id_dest);
    }
  }

  result->n_states = Q.size; // 1 for the ERR state
  result->accepting_states = SET();

  ITER(vector, q, &Q) {
    state_id_t dfa_id = index_of(&Q, q);
    ITER(state_id_t, nfa_id, q) {
      if (*nfa_id == N->end_id &&
          !vec_find(&result->accepting_states, &dfa_id)) {
        vec_insert_sorted(&result->accepting_states, &dfa_id);
      }
    }
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

vec_tuple split(dfa *D, vector *P, vector *s) {
  vec_tuple result = {SET(), SET()};

  assert(s->size);

  for (unsigned c = 1; c < 256; c++) {
    int expect = 0;

    state_id_t dest =
        transition_matrix_find(&D->t_matrix, *(state_id_t *)s->ptr, c);

    if (dest == 0)
      continue;

    ITER(vector, q, P) {
      if (vec_find_sorted(q, &dest)) {
        expect = index_of(P, q);
        break;
      }
    }

    assert(expect >= 0);

    for (state_id_t i = 1; i < s->size; i++) {
      state_id_t id = ((state_id_t *)s->ptr)[i];
      state_id_t dest = transition_matrix_find(&D->t_matrix, id, c);

      if (dest > 0 && vec_find(elem_at(P, expect), &dest)) {
        continue;
      }

      // now all the values up to i-1 belong to the set at [expect].
      for (state_id_t j = 0; j < i; j++) {
        vec_insert_sorted(&result.l, (state_id_t *)s->ptr + j);
      }

      // value at [i] does not.
      vec_insert_sorted(&result.r, &id);

      // the others still need to be checked.
      for (state_id_t j = i + 1; j < s->size; j++) {
        dest =
            transition_matrix_find(&D->t_matrix, ((state_id_t *)s->ptr)[j], c);
        if (dest > 0 && vec_find(elem_at(P, expect), &dest))
          vec_insert_sorted(&result.l, (state_id_t *)s->ptr + j);
        else {
          vec_insert_sorted(&result.r, (state_id_t *)s->ptr + j);
        }
      }
      return result;
    }
  }
  result.l = *s;
  return result;
}

dfa *minimize(dfa *D) {

  vector elems = vec_iota(1, D->n_states - 1);

  vector c = vec_complement(&elems, &D->accepting_states);

  vector T = S_VEC(c, D->accepting_states);
  vector P = S_VEC();

  while (T.size > P.size) {
    destroy(&P);
    P = T;
    T = S_VEC();

    ITER(vector, s, &P) {
      vec_tuple parts = split(D, &P, s);
      vec_insert(&T, &parts.l);
      if (parts.r.size > 0)
        vec_insert(&T, &parts.r);
    }
  }

  dfa *R = calloc(sizeof(dfa), 1);
  R->t_matrix = L_VEC();

  R->n_states = T.size + 1;
  R->accepting_states = SET();

  for (state_id_t i = 1; i < R->n_states; i++) {

    vector *start = elem_at(&T, i - 1);
    assert(start->size);
    state_id_t start_id = *(state_id_t *)start->ptr;

    if (vec_find(&D->accepting_states, &start_id)) {
      vec_insert_sorted(&R->accepting_states, &i);
    }

    for (unsigned c = 1; c < 256; c++) {
      state_id_t end_id = transition_matrix_find(&D->t_matrix, start_id, c);
      for (state_id_t j = 1; j < T.size + 1; j++) {
        if (vec_find(elem_at(&T, j - 1), &end_id)) {
          transition_matrix_insert(&R->t_matrix, i, c, j);
          goto next;
        }
      }
    next:;
    }
  }
  return R;
}
