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
  FILE* output;
  body* bodies;
  point** forces;
} worker_info;

void* Worker (void* d);

void calculate_forces(worker_info data) {
  /* Calculate how the forces apply to the different bodies */
  int64_t worker_id, total_workers, count;
  worker_id = data.worker_id;
  total_workers = data.total_workers;
  count = data.count;
  body* vec = data.bodies;
  point* force = data.forces[worker_id];
  /* Decide what bodies this worker applies to. Pattern ABCABCABC is mostly
   * fair and simple and good 'nuf */
  for (int64_t i = worker_id; i < count; i+=total_workers) { 
    for (int64_t j = i + 1; j < count; j++) {
      double distance = sqrt(pow(vec[i].position.x - vec[j].position.x, 2) +
          pow(vec[i].position.y - vec[j].position.y, 2));
      double magnitude = (NEWTON_G*vec[i].mass*vec[j].mass) /
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
  /* Apply the forces calculated in calculate_forces on the bodies */
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
  int time_limit = TIME_DEFAULT;
  int n_bodies = BODIES_DEFAULT;
  int n_workers = WORKERS_DEFAULT;

  /* Get command line arguments */
  if (argc > 1) {
    n_bodies = atoi(argv[1]);
  }
  if (argc > 2) {
    time_limit = atoi(argv[2]);
  }
  if (argc > 3) {
    n_workers = atoi(argv[3]);
    if (n_workers > 64) {
      n_workers = 64;
    }
  }

  /* The thread variables and properties for the workers */
  pthread_t worker_threads[n_workers];
  worker_info workers_data[n_workers];

  /* set global thread attributes */
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

  FILE* output = fopen("output", "w");

  body* bodies = malloc(sizeof(body) * n_bodies);
  memset(bodies, 0, sizeof(body) * n_bodies);
  for (int i = 0; i < n_bodies; i++) {
    row_of_twenty(&bodies[i], i);
  }

  /* Allocate space for the pointers to the worker point-arrays */
  point** forces = malloc(sizeof(point*) * n_workers);
  printf("[simulation] %d bodies over %d time steps with %d workers\n",
      n_bodies, time_limit, n_workers);

  struct timeval start = start_timer();
  for (int w = 0; w < n_workers; w++) {
    /* Iterate over all the workers and create their properties */
    forces[w] = malloc(sizeof(point) * n_bodies);
    memset(forces[w], 0, sizeof(point) * n_bodies);
    workers_data[w].worker_id = w;
    workers_data[w].total_workers = n_workers;
    workers_data[w].forces = forces;
    workers_data[w].count = n_bodies;
    workers_data[w].bodies = bodies;
    workers_data[w].output = output;
    workers_data[w].time_limit = time_limit;
  }
  for (int w = 0; w < n_workers; w++) {
    pthread_create(&worker_threads[w], &attr, Worker, (void*) &workers_data[w]);
  }
  for (int w = 0; w < n_workers; w++) {
    pthread_join(worker_threads[w], NULL);
  }
  stop_timer(start);
}

void* Worker (void* d) {
  /* A worker is a thread which iterates over a predefined set of bodies
   * and calculates their new properties */
  worker_info* data = (worker_info*) d;
  for (int64_t t = 0; t < data->time_limit; t++) {
#ifdef DEBUG_MODE
    for (int64_t i = data->worker_id; i < data->count; i+=data->total_workers) {
      fprintf(data->output, "%ld %ld %lf %lf\n", t, i,
          data->bodies[i].position.x,
          data->bodies[i].position.y);
    };
#endif
    calculate_forces(*data);
    barrier(data->total_workers);
    move_bodies(*data);
    barrier(data->total_workers);
  }
  return NULL;
}
