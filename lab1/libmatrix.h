#pragma once
#include <stdint.h>

/*
 * Values and position of a single element within a matrix
 */
typedef struct {
  int64_t column;
  int64_t row;
  int64_t val;
} matrix_element;

/*
 * Contains the information returned by a thread
 */
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
