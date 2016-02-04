#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include "libmatrix.h"

/*
 * Print the information in a thread's property field
 */
void properties_print(properties p) {
  printf("Min: %ld [%ld, %ld]; Max: %ld [%ld, %ld]\n",
         p.min.val, p.min.row, p.min.column,
         p.max.val, p.max.row, p.max.column);
  printf("Sum: %ld\n", p.sum);
}

/*
 * Copy the position and value of a matrix element pointer to another
 */
void matrixcpy(matrix_element* from, matrix_element* to) {
  to->row = from->row;
  to->column = from->column;
  to->val = from->val;
}

/*
 * Update the values of a matrix element pointer with the values in the
 * arguments
 */
void update_matrix_element(matrix_element* dest,
    int64_t val,
    int64_t row,
    int64_t column) {
  dest->val    = val;
  dest->row    = row;
  dest->column = column;
};

/* timer */
double read_timer() {
    static bool initialized = false;
    static struct timeval start;
    struct timeval end;
    if( !initialized )
    {
        gettimeofday( &start, NULL );
        initialized = true;
    }
    gettimeofday( &end, NULL );
    return (end.tv_sec - start.tv_sec) + 1.0e-6 * (end.tv_usec - start.tv_usec);
}


