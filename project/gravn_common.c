#include "gravn.h"
#include <sys/time.h>
#include <stdio.h>
#include <pthread.h>

void row_of_twenty(body* o, int64_t i) {
  /* Simple initializer for the bodies where the bodies are stacked up in
   * columns containing 20 elemens each */
  o->id = i;
  o->position.x = i / 20;
  o->position.y = i % 20;
  o->mass = 100000;
}

struct timeval start_timer() {
  /* Return the current time. Wrapper for gettimeofday */
  struct timeval time;
  gettimeofday(&time, NULL);
  return time;
}

void stop_timer(struct timeval start_time) {
  /* Calculate the difference between start_time and current time and print
   * it to stdout */
  struct timeval stop_time;
  gettimeofday(&stop_time, NULL);

  int64_t seconds_total = stop_time.tv_sec - start_time.tv_sec;
  int64_t microseconds_total = stop_time.tv_usec - start_time.tv_usec;
  if (microseconds_total < 0) {
    seconds_total--;
    microseconds_total = 1000000 + microseconds_total;
  }

  printf("[simulation_time] %ld.%4ld seconds\n", seconds_total, microseconds_total);
}

void apply_deltav(body* o) {
  /* Move the bodies according to deltav and adjust their velocity property */
  point deltav, deltap;
  deltav.x = o->force.x/o->mass * DELTA_T;
  deltav.y = o->force.y/o->mass * DELTA_T;

  deltap.x = (o->velocity.x + deltav.x/2) * DELTA_T;
  deltap.y = (o->velocity.y + deltav.y/2) * DELTA_T;

  o->velocity.x = o->velocity.x + deltav.x;
  o->velocity.y = o->velocity.y + deltav.y;
  o->position.x = o->position.x + deltap.x;
  o->position.y = o->position.y + deltap.y;
  o->force.x = 0;
  o->force.y = 0;
}

int64_t num_arrived = 0;
pthread_mutex_t barrier_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t go = PTHREAD_COND_INITIALIZER;

/*
 * from the matrix sum code from homework 1
 */
void barrier(int64_t total_workers) {
  pthread_mutex_lock(&barrier_mutex);
  num_arrived++;
  if (num_arrived == total_workers) {
    num_arrived = 0;
    pthread_cond_broadcast(&go);
  } else {
    pthread_cond_wait(&go, &barrier_mutex);
  }
  pthread_mutex_unlock(&barrier_mutex);
}
