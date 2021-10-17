#include "automata.h"
#include "set.h"
#include "string.h"

unsigned transition_matrix_find(vector *matrix, state_id_t row,
                                unsigned char col) {
  line l = {row, P_VEC()};
  const path p = {col};

  const line *ll = vec_find_sorted(matrix, &l);
  destroy(&l.paths);
  if (!ll)
    return 0;

  const path *pp = vec_find_sorted(&ll->paths, &p);
  if (!pp)
    return 0;

  return pp->end_state;
}

void transition_matrix_insert(vector *matrix, state_id_t start, unsigned char c, state_id_t dest) {
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
  const set *A = a;
  const set *B = b;
  for (size_t i = 0; i < A->size && i < B->size; i++) {
    if (A->ids[i] != B->ids[i])
      return A->ids[i] - B->ids[i];
  }
  return A->size - B->size;
}

#define S_VEC(...) VEC(set, set_cmp, ##__VA_ARGS__)

dfa *to_dfa(nfa *N) {
  dfa *result = calloc(sizeof(dfa), 1);
  result->t_matrix = L_VEC();

  set err_state = {0};
  set_insert(&err_state, 0);

  set n0 = {0};
  set_insert(&n0, 1); // valid states are 1-indexed, state 0 is ERR
  set q0 = eps_closure(N, &n0);

  vector Q = S_VEC(err_state, q0);
  vector Wl = S_VEC(q0);

  while (Wl.size) {
    set q;
    vec_pop_back(&Wl, &q);

    set *source = vec_find(&Q, &q);

    // we should be able to find the element in Q,
    // since Wl is a subset of Q.
    assert(source != NULL);
    state_id_t id_source = index_of(&Q, source);

    for (unsigned c = 0; c < 256; c++) {
      set tmp = delta(N, &q, c);

      if (!tmp.size) {
        set_destroy(&tmp);
        continue;
      }

      set t = eps_closure(N, &tmp);
      set_destroy(&tmp);

      set *dest_p = vec_find(&Q, &t);
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

  ITER(set, q, &Q) {
    for (size_t i = 0; i < q->size; i++) {
      if (set_find(&N->accepting_states, q->ids[i]) &&
          !set_find(&result->accepting_states, index_of(&Q, q))) {
        set_insert(&result->accepting_states, index_of(&Q, q));
      }
    }
  }

  return result;
}

set delta(nfa *N, set *q, unsigned char c) {
  set result = {0};
  for (size_t i = 0; i < q->size; i++) {
    state_id_t id = q->ids[i];
    if (N->T.data[id][c]) {
      set_insert(&result, N->T.data[id][c]);
    }
  }
  return result;
}

set eps_closure(nfa *N, const set *in) {
  set result = {0}, Wl = {0};

  for (size_t i = 0; i < in->size; i++) {
    set_insert(&result, in->ids[i]);
    set_insert(&Wl, in->ids[i]);
  }

  while (Wl.size > 0) {
    state_id_t id = set_pop(&Wl);

    for (size_t i = 0; i < 2; i++) {
      state_id_t j;
      if ((j = N->epsilon[id][i]) > 0 && !set_find(&result, j)) {
        set_insert(&result, j);
        set_insert(&Wl, j);
      }
    }
  }
  set_destroy(&Wl);
  return result;
}

set_tuple split(dfa *D, vector *P, set *s) {
  set_tuple result = {0};

  assert(s->size);

  for (unsigned c = 0; c < 256; c++) {
    int expect = 0;

    state_id_t dest = transition_matrix_find(&D->t_matrix, s->ids[0], c);

    if (dest == 0)
      continue;

    ITER(set, s, P) {
      if (set_find(s, dest)) {
        expect = index_of(P, s);
        break;
      }
    }

    assert(expect >= 0);

    for (state_id_t i = 1; i < s->size; i++) {

      state_id_t dest = transition_matrix_find(&D->t_matrix, s->ids[i], c);

      if (dest > 0 && set_find(elem_at(P, expect), dest)) {
        continue;
      }

      // now all the values up to i-1 belong to the set at [expect].
      for (state_id_t j = 0; j < i; j++) {
        set_insert(&result.l, s->ids[j]);
      }

      // value at [i] does not.
      set_insert(&result.r, s->ids[i]);

      // the others still need to be checked.
      for (state_id_t j = i + 1; j < s->size; j++) {
        dest = transition_matrix_find(&D->t_matrix, s->ids[j], c);
        if (dest > 0 && set_find(elem_at(P, expect), dest))
          set_insert(&result.l, s->ids[j]);
        else
          set_insert(&result.r, s->ids[j]);
      }
      return result;
    }
  }
  result.l = *s;
  return result;
}

dfa *minimize(dfa *D) {
  // set_vec T = {0};
  // set_vec P = {0};

  set elems = set_iota(1, D->n_states - 1);
  set c = set_complement(&elems, &D->accepting_states);

  vector T = S_VEC(c, D->accepting_states);
  vector P = S_VEC();

  while (T.size > P.size) {
    destroy(&P);
    P = T;
    T = S_VEC();

    ITER(set, s, &P) {
      set_tuple parts = split(D, &P, s);
      vec_insert(&T, &parts.l);
      if (parts.r.size > 0)
        vec_insert(&T, &parts.r);
    }
  }

  dfa *R = calloc(sizeof(dfa), 1);
  R->t_matrix = L_VEC();

  R->n_states = T.size + 1;

  for (state_id_t i = 1; i < R->n_states; i++) {

    set *start = elem_at(&T, i-1);
    assert(start->size);
    state_id_t start_id = start->ids[0];

    if (set_find(&D->accepting_states, start_id)) {
      set_insert(&R->accepting_states, i);
    }

    for (unsigned c = 0; c < 256; c++) {
      state_id_t end_id = transition_matrix_find(&D->t_matrix, start_id, c);
      for (state_id_t j = 1; j < T.size + 1; j++) {
        if (set_find(elem_at(&T, j - 1), end_id)) {
          transition_matrix_insert(&R->t_matrix, i, c, j);
          goto next;
        }
      }
    next:;
    }
  }
  return R;
}
