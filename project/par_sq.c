#include "gravn.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

typedef struct worker_info {
  int64_t worker_id;
  int64_t total_workers;
  int64_t count;
  int64_t time_limit;
  body* bodies;
  point** forces;
} worker_info;

void* Worker (void* d);

void calculate_forces(worker_info data) {
  int64_t worker_id, total_workers, count;
  worker_id = data.worker_id;
  total_workers = data.total_workers;
  count = data.count;
  body* vec = data.bodies;
  point* force = data.forces[worker_id];
  for (int64_t i = worker_id; i < count; i+=total_workers) {
    for (int64_t j = i + 1; j < count; j++) {
      double distance = sqrt(pow(vec[i].position.x - vec[j].position.x, 2) +
          pow(vec[i].position.y - vec[j].position.y, 2));
      double magnitude = (NEWTON_G*vec[i].mass*vec[i].mass) /
        (pow(distance, 2));
      point direction;
      direction.x = vec[j].position.x - vec[i].position.x;
      direction.y = vec[j].position.y - vec[i].position.y;

      force[i].x = force[i].x + magnitude*direction.x/distance;
      force[j].x = force[j].x - magnitude*direction.x/distance;
      force[i].y = force[i].y + magnitude*direction.y/distance;
      force[j].y = force[j].y - magnitude*direction.y/distance;
    }
  }
}

void move_bodies(worker_info data) {
  point force;
  force.x = 0.0;
  force.y = 0.0;

  for (int64_t i = data.worker_id; i < data.count; i+=data.total_workers) {
    for (int64_t k = 0; k < data.total_workers; k++) {
      force.x = force.x + data.forces[k][i].x;
      force.y = force.y + data.forces[k][i].y;
      data.forces[k][i].x = 0;
      data.forces[k][i].y = 0;
    }
    data.bodies[i].force.x = force.x;
    data.bodies[i].force.y = force.y;
    apply_deltav(&data.bodies[i]);
    force.x = 0;
    force.y = 0;
  }
}

int main(int argc, char* argv[]) {
  int time_limit = 350;
  int n_bodies = 240;
  int n_workers = 1;
  pthread_t worker_threads[n_workers];
  pthread_attr_t attr;
  /* set global thread attributes */
  pthread_attr_init(&attr);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

  FILE* output = fopen("output.json", "w");

  body* bodies = malloc(sizeof(body) * n_bodies);
  memset(bodies, 0, sizeof(body) * n_bodies);
  for (int i = 0; i < n_bodies; i++) {
    row_of_twenty(&bodies[i], i);
  }

  point** forces = malloc(sizeof(point*) * n_workers);
  struct timeval start = start_timer();
  for (int w = 0; w < n_workers; w++) {
    worker_info workers_data;
    forces[w] = malloc(sizeof(point) * n_bodies);
    memset(bodies, 0, sizeof(point) * n_bodies);
    workers_data.worker_id = w;
    workers_data.total_workers = n_workers;
    workers_data.forces = forces;
    workers_data.count = n_bodies;
    workers_data.bodies = bodies;
    workers_data.time_limit = time_limit;
    pthread_create(&worker_threads[w], &attr, Worker, (void*) &workers_data);
  } 
  for (int w = 0; w < n_workers; w++) {
    pthread_join(worker_threads[w], NULL);
  }
  stop_timer(start);
}

void* Worker (void* d) {
  worker_info* data = (worker_info*) d;
  for (int64_t t = 0; t < data->time_limit; t++) {
    calculate_forces(*data);
    barrier(data->total_workers);
    move_bodies(*data);
    barrier(data->total_workers);
    for (int64_t i = data->worker_id; i < data->count; i+=data->total_workers) {
      printf("%ld %ld x %lf y %lf\n", t, i, data->bodies[i].position.x,
          data->bodies[i].position.y);
    };
  }
  return NULL;
}
