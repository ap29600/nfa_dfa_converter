#ifndef UTIL_H_
#define UTIL_H_
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_NFA_SIZE 1000

#define UNIMPLEMENTED                                                          \
  {                                                                            \
    fprintf(stderr, "ERR: \"%s\" is not implemented.\n", __func__);            \
    exit(1);                                                                   \
  }

typedef struct {
  size_t elem_size;
  size_t size;
  size_t cap;
  void *ptr;
  int (*compar)(const void *, const void *);
} vector;

#define VEC(T, C, ...)                                                         \
  ({                                                                           \
    const T tmp_arr[] = {__VA_ARGS__};                                         \
    void *const tmp_p = malloc(sizeof(tmp_arr));                               \
    memcpy(tmp_p, tmp_arr, sizeof(tmp_arr));                                   \
    (vector){                                                                  \
        .elem_size = sizeof(T),                                                \
        .size = sizeof(tmp_arr) / sizeof(tmp_arr[0]),                          \
        .cap = sizeof(tmp_arr) / sizeof(tmp_arr[0]),                           \
        .ptr = tmp_p,                                                          \
        .compar = (C),                                                         \
    };                                                                         \
  })

#define VEC_INSERT(V, L)                                                       \
  {                                                                            \
    const __typeof__(L) l = L;                                                 \
    assert(sizeof(l) == (V)->elem_size);                                       \
    vec_insert(V, &l);                                                         \
  }

#define ITER(T, P, V)                                                          \
  assert(sizeof(T) == (V)->elem_size);                                         \
  for (T *P = (T *)(V)->ptr;                                                   \
       (uintptr_t)P < (uintptr_t)(V)->ptr + (V)->size * (V)->elem_size;        \
       *(uintptr_t *)&P += (V)->elem_size)

typedef unsigned short state_id_t;

typedef unsigned long bitset_block_t;
#define BS_BLOCK_SIZ (sizeof(bitset_block_t) * 8)
#define BS_N_BLOCKS ((MAX_NFA_SIZE - 1) / BS_BLOCK_SIZ + 1)
typedef struct {
  bitset_block_t data[BS_N_BLOCKS];
} bit_set;

#define ITERATE_BITSET(N, B)                                                   \
  for (state_id_t block__ = 0, N = 0; block__ < BS_N_BLOCKS;                   \
       block__++, N = block__ * BS_BLOCK_SIZ)                                  \
    if ((B).data[block__])                                                     \
      for (state_id_t i__ = 0; i__ < BS_BLOCK_SIZ; i__++, N++)                 \
        if (((B).data[block__] >> i__) & 1)

int st_cmp(const void *a, const void *b);

void vec_sort(vector *vec);
void *vec_find_sorted(const vector *vec, const void *element);
void *vec_find(const vector *vec, const void *element);
void vec_insert_sorted(vector *vec, const void *element);
void vec_insert(vector *vec, const void *element);
void vec_pop_back(vector *vec, void *dest);
void destroy(vector *v);
size_t index_of(const vector *vec, void *element);
void *elem_at(const vector *vec, size_t index);
bit_set set_iota(state_id_t start, state_id_t end);
bit_set set_complement(const bit_set *source, const bit_set *exclude);

void debug_ivec(vector *v);
void inspect (const bit_set *s);

inline static int set_has(bit_set *s, size_t id) {
  assert(MAX_NFA_SIZE >= id);
  return (s->data[id / BS_BLOCK_SIZ] >> (id % BS_BLOCK_SIZ)) & 1;
}

static inline void set_insert(bit_set *s, size_t id) {
  assert(MAX_NFA_SIZE >= id);
  s->data[id / BS_BLOCK_SIZ] |= ((bitset_block_t)1) << (id % BS_BLOCK_SIZ);
}

static inline void set_remove(bit_set *s, size_t id) {
  assert(MAX_NFA_SIZE >= id);
  s->data[id / BS_BLOCK_SIZ] &= ~(((bitset_block_t)1) << (id % BS_BLOCK_SIZ));
}

static inline state_id_t set_pop(bit_set *s) {
  size_t i = 0;
  for (; i < BS_N_BLOCKS && s->data[i] == 0; i++);
  unsigned j = 0;

  // 0 means empty list, since it is not a valid id.
  if (i == BS_N_BLOCKS)
    return 0;

  for (; j < BS_BLOCK_SIZ && !((s->data[i] >> j) & 1); j++);
  s->data[i] &= ~((bitset_block_t)1 << j);
  return i * BS_BLOCK_SIZ + j;
}

static inline state_id_t set_peek(bit_set *s) {
  size_t i = 0;
  for (; i < BS_N_BLOCKS && s->data[i] == 0; i++);
  unsigned j = 0;
  // 0 means empty list, since it is not a valid id.
  if (i == BS_N_BLOCKS)
    return 0;
  for (; j < BS_BLOCK_SIZ && !((s->data[i] >> j) & 1); j++);
  return i * BS_BLOCK_SIZ + j;
}

static inline int empty(const bit_set *s) {
  for (size_t i = 0; i < BS_N_BLOCKS; i++)
    if (s->data[i] != 0)
      return 0;
  return 1;
}

#endif // UTIL_H_
