#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

#define N_BABY_BIRDS 10
#define N_WORMS 17

int64_t worms;
// Semaphore for waiting for food
sem_t bowl_sem;
// Semaphore for parent bird to keep the hungry baby bird from eating while
// refilling
sem_t refill_sem;
// Semaphore for waking up parent bird
sem_t chirp_sem;

pthread_attr_t attr;

void* baby_bird(void*);
void* parent_bird(void*);

int main(int argc, char* argv[]) {
  /*
   * There are threads for every bird, the baby birds are represented within
   * an array of pthreads and the parent bird by a pthread directly
   */
  pthread_t baby[N_BABY_BIRDS];
  pthread_t parent;

  // initialize global pthread attributes
  pthread_attr_init(&attr);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

  /* Initialize synchronization semaphores */
  sem_init(&bowl_sem, 0, 1);
  sem_init(&refill_sem, 0, 0);
  sem_init(&chirp_sem, 0, 0);

  /* Worms in the bowl from the beginning */
  worms = N_WORMS;

  // Start num_workers workers
  for (int64_t i = 0; i < N_BABY_BIRDS; i++) {
    pthread_create(&baby[i], &attr, baby_bird, (void *) i);
  }
  pthread_create(&parent, &attr, parent_bird, NULL);

  pthread_join(parent, NULL);

  return 0;
}

/* Thread for a baby bird which consumes worms from the food bowl */
void* baby_bird(void* arg) {
  int64_t me = (int64_t) arg;
  int64_t eat_counter = 0;
  struct timespec sleeptime = {0};
  struct sched_param param;

  /* POSIX says that if you set SCHED_FIFO, the longest waiting thread will
   * get the semaphore. Thus we implement fairness
   * Improvement: Potentially alter the priority depending on how many times
   * the bird has eaten to make fairness even fairer. Like siblings are.
   */
  pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);

  while(true) {
    /* Stand in queue */
    sem_wait(&bowl_sem);

    /* Wait for parent to refill bowl if bowl is empty */
    if (worms == 0) {
      printf("[Baby %lu] CHIRP!\n", me);
      sem_post(&chirp_sem);
      sem_wait(&refill_sem);
    }

    /* Eat... */
    worms--;
    eat_counter++;
    printf("[Baby %lu] Eating. %ld worms left. Eaten %ld times\n",
        me, worms, eat_counter);
    /* ...for 1/3 seconds */
    sleeptime.tv_sec = 0;
    sleeptime.tv_nsec = 300000000;
    nanosleep(&sleeptime, NULL);

    sem_post(&bowl_sem);

    /* Do not wait for food for 1 second */
    sleeptime.tv_sec = 1;
    sleeptime.tv_nsec = 0;
    nanosleep(&sleeptime, NULL);
  }
}

/* Thread for a parent bird which produces worms for the food bowl */
void* parent_bird(void* arg) {
  /* It takes the parent two seconds to refill the bowl */
  struct timespec sleeptime = {.tv_sec = 2, .tv_nsec = 0};
  while(true) {
    /* Sleep until awakened by a child chirping */
    sem_wait(&chirp_sem);
    printf("[parent] Flying away to refill bowl\n");
    nanosleep(&sleeptime, NULL);
    /* Refill bowl */
    worms = N_WORMS;
    printf("[parent] Bowl refilled\n");
    /* Tell child the bowl is refilled */
    sem_post(&refill_sem);
  }
}

