#pragma once
#include <stdint.h>

typedef struct {
  int64_t column;
  int64_t row;
  int64_t val;
} matrix_element;

typedef struct {
  matrix_element min;
  matrix_element max;
  int64_t sum;
  bool useful;
} properties;

double read_timer();
void matrixcpy(matrix_element*, matrix_element*);
void properties_print(properties);
void update_matrix_element(matrix_element*, int64_t, int64_t, int64_t);
