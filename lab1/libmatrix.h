#pragma once
#include <stdint.h>

typedef struct {
  int64_t x;
  int64_t y;
  int64_t val;
} matrix_element;

typedef struct {
  matrix_element min;
  matrix_element max;
  int64_t sum;
} properties;

double read_timer();
void matrixcpy(matrix_element*, matrix_element*);
void properties_print(properties);
