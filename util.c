#include "util.h"
#include <stdio.h>

void *vec_find_sorted(const vector *vec, const void *element) {
  assert(vec != NULL);
  if (vec->size == 0)
    return NULL;
  assert(vec->ptr != NULL);
  assert(vec->compar != NULL);
  return bsearch(element, vec->ptr, vec->size, vec->elem_size, vec->compar);
}

void *vec_find(const vector *vec, const void *element) {
  assert(vec != NULL);
  if (vec->size == 0)
    return NULL;
  assert(vec->ptr != NULL);
  assert(vec->compar != NULL);
  size_t s = vec->elem_size;
  size_t n = vec->size;
  const uintptr_t p = (uintptr_t)vec->ptr;
  for (void *test = (void*)p; (uintptr_t)test < p + n * s; *(uintptr_t*)&test += s) {
    if (vec->compar(element, test) == 0) 
      return test;
  }
  return NULL;
}

void vec_insert(vector *vec, const void *element) {
  assert(vec != NULL);
  assert(element != NULL);

  const size_t s = vec->elem_size;
  if (vec->size + 1 >= vec->cap) {
    vec->cap = vec->cap * 2 + 1;
    vec->ptr = realloc(vec->ptr, vec->cap * s);
  }
  memcpy((char*)vec->ptr + (vec->size++) * s, element, s);
}

void vec_insert_sorted(vector *vec, const void *element) {
  assert(vec != NULL);
  assert(element != NULL);

  const size_t s = vec->elem_size;
  if (vec->size + 1 >= vec->cap) {
    vec->cap = vec->cap * 2 + 1;
    vec->ptr = realloc(vec->ptr, vec->cap * s);
  }

  char *const p = vec->ptr;
  vec->size++;
  size_t i = 0;
  for (; i < vec->size - 1 && vec->compar(element, p + i * s) > 0; i++) ;
  memmove(p + (i + 1) * s, p + i * s, (vec->size - i - 1) * s);
  memcpy(p + i * s, element, s);
}

void vec_pop_back(vector *vec, void *dest) {
  assert(dest != NULL);
  assert(vec != NULL);
  assert(vec->ptr != NULL);
  assert(vec->size > 0);
  memcpy(dest, vec->ptr + (--vec->size) * vec->elem_size, vec->elem_size);
}

size_t index_of(const vector *vec, void *element) {
  return ((char *)element - (char *)vec->ptr) / vec->elem_size;
}


void* elem_at(const vector *vec, size_t index) {
  return (char*)vec->ptr + index * vec->elem_size;
}

void destroy(vector *vec) {
    assert(vec != NULL);
    free(vec->ptr);
    *vec = (vector){0};
}

int st_cmp(const void*a, const void*b) {
    return *(state_id_t*)a - *(state_id_t*)b;
}

vector vec_iota(state_id_t start, state_id_t end) {
    vector result = VEC(state_id_t, st_cmp);
    result.cap = end - start + 1;
    result.size = end - start + 1;
    result.ptr = realloc(result.ptr, result.cap * sizeof(state_id_t));
    state_id_t *p = result.ptr;
    for (state_id_t i = 0; i< result.size; i++) {
        p[i] = i + start;
    }
    return result;
}

vector vec_complement(const vector *source, const vector *exclude) {
    vector result = VEC(state_id_t, st_cmp);

    ITER(state_id_t, id, source) {
        if (!vec_find_sorted(exclude, id)) {
            vec_insert_sorted(&result, id);
        }
    }

    return result;
}

int int_cmp(const void *a, const void *b) { return *(int *)a - *(int *)b; }

typedef struct {
  int id;
  vector paths;
} line;

typedef struct {
  unsigned char trigger;
  int end_state;
} path;

#define L_VEC(...) VEC(line, line_cmp, ##__VA_ARGS__)
#define P_VEC(...) VEC(path, path_cmp, ##__VA_ARGS__)

