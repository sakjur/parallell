/* matrix summation using OpenMP

   usage with gcc (version 4.2 or higher required):
   gcc -O -fopenmp -o matrixSum-openmp matrixSum-openmp.c
   ./matrixSum-openmp size numWorkers

*/

#include <omp.h>

double start_time, end_time;

#include <stdio.h>
#include <stdlib.h>
#define MAXSIZE 10000  /* maximum matrix size */
#define MAXWORKERS 8   /* maximum number of workers */

int numWorkers;
int size;
int matrix[MAXSIZE][MAXSIZE];
void *Worker(void *);

typedef struct {
  size_t column;
  size_t row;
  int64_t value;
} matrixelem;

void minmaxprint(matrixelem min, matrixelem max) {
  printf("Min: %li [%li, %li]; Max: %li [%li, %li]\n",
      min.value, min.row, min.column,
      max.value, max.row, max.column);
}

/* read command line, initialize, and create threads */
int main(int argc, char *argv[]) {
  int64_t i, j, total=0, val;
  matrixelem min, max, localmin, localmax;

  /* read command line args if any */
  size = (argc > 1)? atoi(argv[1]) : MAXSIZE;
  numWorkers = (argc > 2)? atoi(argv[2]) : MAXWORKERS;
  if (size > MAXSIZE) size = MAXSIZE;
  if (numWorkers > MAXWORKERS) numWorkers = MAXWORKERS;

  omp_set_num_threads(numWorkers);

  /* initialize the matrix */
  for (i = 0; i < size; i++) {
    for (j = 0; j < size; j++) {
      matrix[i][j] = rand()%99;
    }
  }

  min.column = 0;
  min.row = 0;
  min.value = matrix[0][0];
  max.column = 0;
  max.row = 0;
  max.value = matrix[0][0];

  start_time = omp_get_wtime();
#pragma omp parallel private(j, localmin, localmax) shared(min, max)
  {
    localmin = min;
    localmax = max;
#pragma omp for reduction (+:total)
    for (i = 0; i < size; i++) {
      for (j = 0; j < size; j++){
        val = matrix[i][j];
        // Update the thread local min matrix element
        if (localmin.value > val) {
          localmin.column = j;
          localmin.row = i;
          localmin.value = val;
        }
        // Update the thread local max matrix element
        if (localmax.value < val) {
          localmax.column = j;
          localmax.row = i;
          localmax.value = val;
        }
        total += val;
      }
    }
#pragma omp critical
    {
      if (min.value > localmin.value) {
        min = localmin;
      }
    }
#pragma omp critical
    {
      if (max.value < localmax.value) {
        max = localmax;
      }
    }
#pragma omp taskwait
#pragma omp master
    {
      end_time = omp_get_wtime();
      printf("the total is %ld\n", total);
      minmaxprint(min, max);
      printf("it took %g seconds\n", end_time - start_time);
    }
  }


}

