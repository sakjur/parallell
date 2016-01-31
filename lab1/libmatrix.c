#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include "libmatrix.h"

void properties_print(properties p) {
  printf("Min: %ld (%ld, %ld); Max: %ld (%ld, %ld)\n",
         p.min.val, p.min.x, p.min.y, p.max.val, p.max.x, p.max.y);
  printf("Sum: %ld\n", p.sum);
}

void matrixcpy(matrix_element* from, matrix_element* to) {
  to->x = from->x;
  to->y = from->y;
  to->val = from->val;
}

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


