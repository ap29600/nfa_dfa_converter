#ifndef UTIL_H_
#define UTIL_H_
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

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
    void *const tmp_p = malloc(sizeof(tmp_arr));                            \
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
          (uintptr_t)P < (uintptr_t)(V)->ptr + (V)->size * (V)->elem_size;     \
          *(uintptr_t*)&P += (V)->elem_size)

void vec_sort(vector *vec);
void *vec_find_sorted(const vector *vec, const void *element);
void *vec_find(const vector *vec, const void *element);
void vec_insert_sorted(vector *vec, const void *element);
void vec_insert(vector *vec, const void *element);
void vec_pop_back(vector *vec, void *dest);
void destroy(vector*v);
size_t index_of(vector *vec, void *element);
void* elem_at(vector *vec, size_t index);

#endif // UTIL_H_
