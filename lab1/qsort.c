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

// An arraylist is an array which also stores it's length
typedef struct {
  int length;
  int* array;
} arraylist;

// A task is an arraylist which is waiting to be sorted and it's respective
// sublists
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

  // If the application is launched with arguments, read these and use the
  // first as the number of workers to use (0 < n <= MAXWORKERS) and the
  // second as the number of elements to sort (0 < n <= MAXELEMS)
  if (argc > 1) {
    num_workers = atoi(argv[1]);
  }
  if (argc > 2) {
    elems = atoi(argv[2]);
  }

  // Sanity checks for input
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

  // initialize list of elements
  arraylist* data = malloc(sizeof(arraylist));
  data->array = malloc(sizeof(int)*elems);
  data->length = elems;
  for (int i = 0; i < elems; i++) {
    data->array[i] = random() % MAXELEMS;
  }

  // sorting the list of elements to a task and set the root task to that task
  root = start_task(data);

#ifdef DEBUG
  for (int i = 0; i < elems; i++)
    printf("%i\n", data->array[i]);
  printf("EOL\n");
#endif

  double start_time = read_timer();

  // Start num_workers workers
  for (size_t i = 0; i < num_workers; i++) {
    pthread_create(&workers[i], &attr, worker, (void *) i);
  }

  // Wait for all workers to terminate
  for (size_t i = 0; i < num_workers; i++) {
    pthread_join(workers[i], NULL);
  }
  double end_time = read_timer();

#ifdef DEBUG
  for (int i = 0; i < elems; i++)
    printf("%i\n", data->array[i]);
  printf("EOL\n");
#endif

  printf("Sorted %i elements using %i worker(s)\n", elems, num_workers);
  printf("The execution time is %g sec\n", end_time - start_time);
  return 0;
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

/**
 * start_task takes a pointer to an arraylist and creates a task that can be
 * used to determine what actions a thread needs to do with a specific list
 */
task* start_task(arraylist* data) {
  task* rv = malloc(sizeof(task));
  pthread_mutex_init(&rv->lock, NULL);
  rv->sorted = false;
  rv->partitioned = false;
  rv->left = NULL;
  rv->right = NULL;
  rv->list = data;
  return rv;
}

/**
 * worker is used to start a thread and make it recursively partition the
 * list to be sorted
 */
void* worker (void* arg) {
  size_t id = (size_t) arg; // Used to decide traversal path
  while(!root->sorted) {
    sort(root, id);
  }
  return NULL;
}

/**
 * traverse the tree of tasks and pick the first available task where an
 * action is necessary
 */
void sort (task* base, size_t id) {
  task *fst, *snd;
  if (pthread_mutex_trylock(&base->lock) == 0) {
    if (!base->partitioned) {
      partition(base);
    }

    if (!base->sorted) {
      if (base->left->sorted && base->right->sorted) {
        // A list is sorted iff both of it's children are sorted
        base->sorted = true;
      }
    }
    pthread_mutex_unlock(&base->lock);
  }

  /*
   * Decide walking left or right. Used to minimize collisions
   * TODO Smarter algorithm for traversal which divides it into n paths instead
   * of 2 paths
   */
  if (id % 2 == 0) {
    fst = base->left;
    snd = base->right;
  } else {
    fst = base->right;
    snd = base->left;
  }

  /*
   * Recursively call the sort function on both the left and right branches
   */
  if (fst)
    if (!fst->sorted) {
      sort(fst, id);
    }
  if (snd)
    if (!snd->sorted) {
      sort(snd, id);
    }
}

/*
 * Divide the list associated with a task into multiple subtasks with lesser,
 * equal and greater digits in them respectively and mark the task as
 * partitioned.
 *
 * Subtasks with one elements are immediately marked as partitioned and sorted
 */
void partition (task* base) {
  arraylist* list = base->list;

  // Pick pivot from middle of the original list
  // (for better performance on sorted lists than picking the first element)
  int pivot = list->array[list->length/2];
  int equal = 0; // Store the number of appearances of the pivot

  // Initialize lists to store the partitions in
  arraylist* lesser = malloc(sizeof(arraylist));
  arraylist* greater = malloc(sizeof(arraylist));
  lesser->length = 0;
  greater->length = 0;
  // Temporary list to not over-write items while reading them
  int* tmplist = malloc(sizeof(int)*list->length);

  // Iterate over the list of items and position them in the temporary list
  // as convenient
  for (int i = 0; i < list->length; i++) {
    int curr = list->array[i];
    if (curr < pivot) {
      tmplist[lesser->length] = curr;
      lesser->length++;
    } else if (curr > pivot) {
      // Larger items are stored at the back of the temporary list
      greater->length++;
      tmplist[list->length-greater->length] = curr;
    } else if (curr == pivot) {
      equal++;
    }
  }

  // The position of the sublist with greater elements
  int* right_pos = list->array + lesser->length + equal;

  // Position pivot-elements in the middle of the list
  while (equal > 0) {
    tmplist[lesser->length-1+equal] = pivot;
    equal--;
  }

#ifdef DEBUG
  printf("\n Pivot: %i [", pivot);
  for (int i = 0; i < list->length; i++) {
    printf("%i ", tmplist[i]);
  }
  printf("]");
#endif

  // Copy over the temporary list to the actual list
  memcpy(list->array, tmplist, list->length*sizeof(int));
  free(tmplist);

  // Prepare tasks and offsets for the sublists
  lesser->array = list->array;
  greater->array = right_pos;
  task* left = start_task(lesser);
  task* right = start_task(greater);

  // Handle single element lists
  if (lesser->length < 2) {
    left->sorted = true;
    left->partitioned = true;
  }
  if (greater->length < 2) {
    right->sorted = true;
    right->partitioned = true;
  }

  // Update the original task with sub tasks and mark as partitioned
  base->partitioned = true;
  base->left = left;
  base->right = right;
};

