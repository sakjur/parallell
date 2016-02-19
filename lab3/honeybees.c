#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

#define N_BEES 10
#define JAR_FULL 64
#define TENTH_SECOND 100000000
#define HALF_SECOND (5*TENTH_SECOND)

// Global pthread attributes
pthread_attr_t attr;

sem_t jar_sem;
sem_t full_sem;
sem_t empty_sem;

int64_t jar;

void* workerbee(void*);
void* bearthread(void*);

int main (int argc, char* argv[]) {
  /*
   * The honeybees stored within an array and the bear stored as a variable
   */
  pthread_t honeybees[N_BEES];
  pthread_t bear_id;

  // initialize global pthread attributes
  pthread_attr_init(&attr);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

  /* Initialize synchronization semaphores */
  sem_init(&jar_sem, 0, 1);
  sem_init(&full_sem, 0, 0);
  sem_init(&empty_sem, 0, 0);

  /* Begin with an empty honey jar */
  jar = 0;

  // Start num_workers workers
  for (int64_t i = 0; i < N_BEES; i++) {
    pthread_create(&honeybees[i], &attr, workerbee, (void *) i);
  }
  pthread_create(&bear_id, &attr, bearthread, NULL);

  // Do not terminate (unless the bear terminates)
  pthread_join(bear_id, NULL);

  return 0;
}

/* The bees who ŕefills the honeyjar one by one */
void* workerbee(void* arg) {
  int64_t id = (int64_t) arg;
  int64_t refillcounter = 0; // How many times has this bee added honey?
  struct timespec sleeptime = {0};

  /* Fair queueing using FIFO scheduling */
  struct sched_param param;
  pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);

  while(true) {
    sem_wait(&jar_sem);
    jar++;
    refillcounter++;
    printf("[%ld] Delivered honey for the %ldth time. (%ld/%d)\n",
        id, refillcounter, jar, JAR_FULL);
    /* Wake up bear when jar is filled */
    if (jar >= JAR_FULL) {
      printf("[%ld] Waking up bear\n", id);
      sem_post(&full_sem);
      sem_wait(&empty_sem);
    }
    /* Drop-off takes .2 seconds */
    sleeptime.tv_sec = 0;
    sleeptime.tv_nsec=2*TENTH_SECOND;
    nanosleep(&sleeptime, NULL);
    sem_post(&jar_sem);
    sleeptime.tv_sec = 2;
    sleeptime.tv_nsec = 0;
    nanosleep(&sleeptime, NULL);
  }
}

/* The bear which consumes the "honey" */
void* bearthread(void* arg) {
  int64_t eatcounter = 1;
  struct timespec sleeptime = {.tv_sec=0, .tv_nsec=TENTH_SECOND*3};
  while(true) {
    sem_wait(&full_sem);
    printf("[BEAR] Yawn. Waking up\n");
    /* Simulate eating */
    while (jar > 0) {
      jar = jar - 10;
      if (jar < 0) /* Silly bear cannot eat more honey that there is */
        jar = 0;
      printf("[BEAR] Eating #%ld (%ld/%d)\n", eatcounter, jar, JAR_FULL);
      nanosleep(&sleeptime, NULL);
    }
    printf("[BEAR] Going to sleep\n");
    sem_post(&empty_sem);
    eatcounter++;
  }
}

