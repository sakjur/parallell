#include <limits.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/time.h>
#include <time.h>
#define MAXELEMS (20*1024*1024)
#define MAXWORKERS 30

typedef struct {
  int length;
  int* array;
} arraylist;

typedef struct task {
  pthread_mutex_t lock;
  arraylist* list;
  bool sorted;
  bool partitioned;
  struct task* left;
  struct task* right;
} task;

double read_timer();
task* start_task(arraylist* data);
void* worker(void*);
void sort (task*, size_t);
void partition (task*);

pthread_attr_t attr;
task* root = NULL;

int main(int argc, char *argv[]) {
  int num_workers = 4;
  int elems = MAXELEMS;

  if (argc > 1) {
    num_workers = atoi(argv[1]);
  }
  if (argc > 2) {
    elems = atoi(argv[2]);
  }

  if (elems == 0 || elems > MAXELEMS) {
    printf("The number of elements must be an integer between 0 and %i\n",
        MAXELEMS);
    return -1;
  }
  if (num_workers == 0 || num_workers > MAXWORKERS) {
    printf("The number of workers must be an integer between 0 and %i\n",
        MAXWORKERS);
    return -1;
  }

  // pthread variables
  pthread_t workers[num_workers];

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
  for (size_t i = 0; i < num_workers; i++) {
    pthread_create(&workers[i], &attr, worker, (void *) i);
  }
  for (size_t i = 0; i < num_workers; i++) {
    pthread_join(workers[i], NULL);
  }
  double end_time = read_timer();

  printf("Sorted %i elements using %i worker(s)\n", elems, num_workers);
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
  rv->gc = false;
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
        base->sorted = true;
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
  lesser->length = 0;
  greater->length = 0;
  int* tmplist = malloc(sizeof(int)*list->length);

  for (int i = 0; i < list->length; i++) {
    int curr = list->array[i];
    if (curr < pivot) {
      tmplist[lesser->length] = curr;
      lesser->length++;
    } else if (curr > pivot) {
      tmplist[list->length-(greater->length+1)] = curr;
      greater->length++;
    } else if (curr == pivot) {
      equal++;
    }
  }

  while (equal > 0) {
    tmplist[lesser->length+equal-1] = pivot;
    equal--;
  }

  memcpy(list->array, tmplist, list->length*sizeof(int));
  free(tmplist);

  int* right_pos = list->array + lesser->length + equal;
  lesser->array = list->array;
  greater->array = right_pos;
  task* left = start_task(lesser);
  task* right = start_task(greater);

  if (lesser->length < 2) {
    left->sorted = true;
    left->partitioned = true;
  }
  if (greater->length < 2) {
    right->sorted = true;
    right->partitioned = true;
  }

  base->partitioned = true;
  base->left = left;
  base->right = right;
};

