#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/time.h>
#include <time.h>
#define MAXELEMS 1000000
#define MAXWORKERS 2

typedef struct {
  int length;
  int* array;
} arraylist;

typedef struct task {
  pthread_mutex_t lock;
  arraylist* list;
  bool sorted;
  bool partitioned;
  int pivot;
  int pequal; // Number equal to the pivot
  struct task* left;
  struct task* right;
} task;

double read_timer();
task* start_task(arraylist* data);
void free_task(task*);
void* worker(void*);
void sort (task*, size_t);
void partition (task*);
void merge (task*);

pthread_attr_t attr;
task* root = NULL;

int main(int argc, char *argv[]) {
  int n = MAXWORKERS;
  int elems = MAXELEMS;
  // pthread variables
  pthread_t workers[n];

  // initialize global pthread attributes
  pthread_attr_init(&attr);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

  arraylist* data = malloc(sizeof(arraylist));
  data->array = malloc(sizeof(int)*elems);
  data->length = elems;
  for (int i = 0; i < elems; i++) {
    data->array[i] = random() % MAXELEMS;
  }
  root = start_task(data);

  double start_time = read_timer();
  for (size_t i = 0; i < n; i++) {
    pthread_create(&workers[i], &attr, worker, (void *) i);
  }
  for (size_t i = 0; i < n; i++) {
    pthread_join(workers[i], NULL);
  }
  double end_time = read_timer();

  printf("Sorted %i elements using %i worker(s)\n", elems, n);
  printf("The execution time is %g sec\n", end_time - start_time);
  return 0;
}

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

task* start_task(arraylist* data) {
  task* rv = malloc(sizeof(task));
  pthread_mutex_init(&rv->lock, NULL);
  rv->sorted = false;
  rv->partitioned = false;
  rv->pivot = 0;
  rv->pequal = 0;
  rv->left = NULL;
  rv->right = NULL;
  rv->list = data;
  return rv;
}

void* worker (void* arg) {
  size_t id = (size_t) arg;
  while(!root->sorted) {
    sort(root, id);
  }
  return NULL;
}

void sort (task* base, size_t id) {
  task *fst, *snd;
  if (pthread_mutex_trylock(&base->lock) == 0) {
    if (!base->partitioned) {
      partition(base);
    }

    if (!base->sorted) {
      if (base->left->sorted && base->right->sorted) {
        merge(base);
      }
    }
    pthread_mutex_unlock(&base->lock);
  }
  if (id % 2 == 0) {
    fst = base->left;
    snd = base->right;
  } else {
    fst = base->right;
    snd = base->left;
  }
  if (fst)
    if (!fst->sorted) {
      sort(fst, id);
    }
  if (snd)
    if (!snd->sorted) {
      sort(snd, id);
    }
}

void partition (task* base) {
  arraylist* list = base->list;

  int pivot = list->array[list->length/2];
  int equal = 0;

  arraylist* lesser = malloc(sizeof(arraylist));
  arraylist* greater = malloc(sizeof(arraylist));
  lesser->array = malloc(sizeof(int)*list->length);
  greater->array = malloc(sizeof(int)*list->length);

  for (int i = 0; i < list->length; i++) {
    int curr = list->array[i];
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

  base->pivot = pivot;
  base->pequal = equal;
  base->left = start_task(lesser);
  base->right = start_task(greater);

  if (lesser->length < 2) {
    base->left->sorted = true;
    base->left->partitioned = true;
  }
  if (greater->length < 2) {
    base->right->sorted = true;
    base->right->partitioned = true;
  }

  base->partitioned = true;
};

void merge (task* base) {
  task* left = base->left;
  task* right = base->right;
  arraylist* list = base->list;
  int equal = base->pequal;

  if (!left->sorted && !right->sorted)
    return;

  int* right_pos = list->array + left->list->length + equal;
  memcpy(list->array, left->list->array, left->list->length*sizeof(int));
  memcpy(right_pos, right->list->array, right->list->length*sizeof(int));

  while (equal > 0) {
    list->array[left->list->length+equal-1] = base->pivot;
    equal--;
  }

  base->sorted = true;
}

void free_task (task* base) {
  if (base) {
    free(base->list->array);
    free(base->list);
    free(base);
  }
}

