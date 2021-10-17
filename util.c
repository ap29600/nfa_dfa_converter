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

size_t index_of(vector *vec, void *element) {
  return ((char *)element - (char *)vec->ptr) / vec->elem_size;
}


void* elem_at(vector *vec, size_t index) {
  return (char*)vec->ptr + index * vec->elem_size;
}

void destroy(vector *vec) {
    assert(vec != NULL);
    free(vec->ptr);
    *vec = (vector){0};
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

int line_cmp(const void *a, const void *b) {
  return ((line *)a)->id - ((line *)b)->id;
}
int path_cmp(const void *a, const void *b) {
  return ((path *)a)->trigger - ((path *)b)->trigger;
}

#define L_VEC(...) VEC(line, line_cmp, ##__VA_ARGS__)
#define P_VEC(...) VEC(path, path_cmp, ##__VA_ARGS__)

/*
int main() {
  vector transition_matrix = L_VEC(
          {0,  P_VEC()},
          {1,  P_VEC({'t', 2}, {'h', 7})},
          {2,  P_VEC({'h', 3})},
          {3,  P_VEC({'e', 4})},
          {4,  P_VEC({'r', 5})},
          {5,  P_VEC({'e', 6})},
          {6,  P_VEC()},
          {7,  P_VEC({'e', 8})},
          {8,  P_VEC({'r', 9})},
          {9,  P_VEC({'e', 10})},
          {10, P_VEC()}
          );

  ITER(line, start_state, &transition_matrix) {
    ITER(path, pth, &start_state->paths) {
      printf("%d --[%c]--> %d\n", start_state->id, pth->trigger, pth->end_state);
    }
  }
}
*/
