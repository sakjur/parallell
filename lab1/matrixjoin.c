/* matrix summation using pthreads

   features: uses a barrier; the Worker[0] computes
             the total sum from partial sums computed by Workers
             and prints the total sum to the standard output

   usage under Linux:
     gcc matrixSum.c -lpthread
     a.out size numWorkers

*/
#ifndef _REENTRANT
#define _REENTRANT
#endif
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "libmatrix.h"
#define MAXSIZE 10000  /* maximum matrix size */
#define MAXWORKERS 10   /* maximum number of workers */

int numWorkers;           /* number of workers */
int numArrived = 0;       /* number who have arrived */

int size, stripSize;  /* assume size is multiple of numWorkers */
int matrix[MAXSIZE][MAXSIZE]; /* matrix */

void *Worker(void *);

/* read command line, initialize, and create threads */
int main(int argc, char *argv[]) {
  int i, j;
  long l; /* use long in case of a 64-bit system */
  pthread_attr_t attr;
  pthread_t workerid[MAXWORKERS];

  /* set global thread attributes */
  pthread_attr_init(&attr);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

  /* read command line args if any */
  size = (argc > 1)? atoi(argv[1]) : MAXSIZE;
  numWorkers = (argc > 2)? atoi(argv[2]) : MAXWORKERS;
  if (size > MAXSIZE) size = MAXSIZE;
  if (numWorkers > MAXWORKERS) numWorkers = MAXWORKERS;
  stripSize = size/numWorkers;

  srandom((int)time(NULL));
  /* initialize the matrix */
  for (i = 0; i < size; i++) {
    for (j = 0; j < size; j++) {
          matrix[i][j] = random() % 0xCADA;
    }
  }

  /* print the matrix */
#ifdef DEBUG
  for (i = 0; i < size; i++) {
    printf("[ ");
    for (j = 0; j < size; j++) {
      printf(" %d", matrix[i][j]);
    }
    printf(" ]\n");
  }
#endif

  double start_time, end_time; /* start and end times */
  /* do the parallel work: create the workers */
  start_time = read_timer();
  for (l = 0; l < numWorkers; l++)
    pthread_create(&workerid[l], &attr, Worker, (void *) l);
  int64_t total = 0;
  matrix_element min, max = {0};
  properties* values;
  for (l = 0; l < numWorkers; l++) {
    pthread_join(workerid[l], (void**) &values);
    total += values->sum;
    if (l == 0) {
      matrixcpy(&values->min, &min);
      matrixcpy(&values->max, &max);
    } else {
      if (values->min.val < min.val) {
        matrixcpy(&values->min, &min);
      }
      if (values->max.val < max.val) {
        matrixcpy(&values->max, &max);
      }
    }
  }
  end_time = read_timer();
  printf("The total is %ld\n", total);
  printf("Max (column, row): %ld (%ld, %ld)\n", max.val, max.x, max.y);
  printf("Min (column, row): %ld (%ld, %ld)\n", min.val, min.x, min.y);
  printf("The execution time is %g sec\n", end_time - start_time);
  pthread_exit(NULL);
}

/* Each worker sums the values in one strip of the matrix. */
void *Worker(void *arg) {
  int64_t myid = (int64_t) arg;
  properties* value = malloc(sizeof(properties));
  int total, i, j, first, last;
  matrix_element min, max = {0};

#ifdef DEBUG
  printf("worker %ld (pthread id %lu) has started\n", myid, pthread_self());
#endif

  /* determine first and last rows of my strip */
  first = myid*stripSize;
  last = (myid == numWorkers - 1) ? (size - 1) : (first + stripSize - 1);

  /* initialize both max and min to first element */
  max.y   = first;
  min.y   = first;
  max.val = matrix[first][0];
  min.val = matrix[first][0];

  /* sum values in my strip */
  total = 0;
  for (i = first; i <= last; i++)
    for (j = 0; j < size; j++) {
      total += matrix[i][j];
      if (matrix[i][j] < min.val) {
        min.y   = i;
        min.x   = j;
        min.val = matrix[i][j];
      }
      if (matrix[i][j] > max.val) {
        max.y   = i;
        max.x   = j;
        max.val = matrix[i][j];
      }
    }
  value->sum = total;
  matrixcpy(&min, &value->min);
  matrixcpy(&max, &value->max);
#ifdef DEBUG
  properties_print(*value);
#endif
  return value;
}
