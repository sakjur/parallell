#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define MAXELEMS 30
#define MAXWORKERS 30

typedef struct {
  int length;
  int* array;
} arraylist;

void* quicksort(void* arg);
pthread_attr_t attr;

int main(int argc, char *argv[]) {
  // pthread variables

  // initialize global pthread attributes
  pthread_attr_init(&attr);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

  int elems = MAXELEMS;
  arraylist* data = malloc(sizeof(arraylist));
  data->array = malloc(sizeof(int)*elems);
  data->length = elems;
  for (int i = 0; i < elems; i++) {
    data->array[i] = random() % 99;
  }

  quicksort(data);

  for (int i = 0; i < elems; i++) {
    printf("%d\n", data->array[i]);
  }
  return 0;
}

void* quicksort(void* arg) {
  arraylist* data = (arraylist*) arg;
  int pivot_index = data->length / 2;
  int pivot = data->array[pivot_index];
  int equal = 0;
  pthread_t lesserworker, greaterworker;

  arraylist* lesser = malloc(sizeof(arraylist));
  arraylist* greater = malloc(sizeof(arraylist));
  lesser->array = malloc(sizeof(int)*data->length);
  greater->array = malloc(sizeof(int)*data->length);

  for (int i = 0; i < data->length; i++) {
    int curr = data->array[i];
    if (curr < pivot) {
      lesser->array[lesser->length] = curr;
      lesser->length++;
    } else if (curr > pivot) {
      greater->array[greater->length] = curr;
      greater->length++;
    } else if (curr == pivot) {
      equal++;
    }
  }

  printf("%d + %d + %d = %d\n", lesser->length, equal, greater->length,
      data->length);

  if (lesser->length > 1) {
    pthread_create(&lesserworker, &attr, quicksort, (void *) lesser);
  }
  if (greater->length > 1) {
    pthread_create(&greaterworker, &attr, quicksort, (void *) greater);
  }

  int greater_offset = lesser->length+equal;
  if (lesser->length > 1)
    pthread_join(lesserworker, NULL);
  memcpy(data->array, lesser->array, lesser->length*sizeof(int));
  if (greater->length > 1)
    pthread_join(greaterworker, NULL);
  memcpy(data->array+greater_offset, greater->array,
      greater->length*sizeof(int));
  while (equal > 0) {
    data->array[lesser->length+equal-1] = pivot;
    equal--;
  }
  //free(lesser->array);
  //free(lesser);
  //free(greater->array);
  //free(greater);

  return (void *) data;
}
