#include "gravn.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/time.h>

void calculate_forces(int64_t count, body* vec) {
  /* Calculate the graviational forces between the bodies in the 'verse */
  for (int64_t i = 0; i < count; i++) {
    for (int64_t j = i + 1; j < count; j++) {
      double distance = sqrt(pow(vec[i].position.x - vec[j].position.x, 2) +
          pow(vec[i].position.y - vec[j].position.y, 2));
      double magnitude = (NEWTON_G*vec[i].mass*vec[j].mass) /
        (pow(distance, 2));
      point direction;
      direction.x = vec[j].position.x - vec[i].position.x;
      direction.y = vec[j].position.y - vec[i].position.y;

      vec[i].force.x = vec[i].force.x + magnitude*direction.x/distance;
      vec[j].force.x = vec[j].force.x - magnitude*direction.x/distance;
      vec[i].force.y = vec[i].force.y + magnitude*direction.y/distance;
      vec[j].force.y = vec[j].force.y - magnitude*direction.y/distance;
    }
  }
}

void move_bodies(int64_t count, body* vec) {
  /* Apply the forces of the bodies using the common apply_deltav function */
  for (int64_t i = 0; i < count; i++){
    apply_deltav(&vec[i]);
  }
}

int main (int argc, char* argv[]) {
  /* Run the simulation */
  int time_limit = TIME_DEFAULT;
  int n_bodies = BODIES_DEFAULT;

  /* Command line arguments */
  if (argc > 1) {
    n_bodies = atoi(argv[1]);
  }
  if (argc > 2) {
    time_limit = atoi(argv[2]);
  }

  /* Initialize the bodies at their first position */
  body bodies[n_bodies];
  memset(bodies, 0, sizeof(body) * n_bodies);
  for (int i = 0; i < n_bodies; i++) {
    row_of_twenty(&bodies[i], i);
  }

  printf("[simulation] %d bodies over %d time steps\n", n_bodies, time_limit);
  struct timeval start = start_timer();
#ifdef DEBUG_MODE
  FILE* output = fopen("output", "w");
#endif
  /* Do simulation */
  for (int64_t t = 0; t < time_limit; t++) {
    calculate_forces(n_bodies, bodies);
    move_bodies(n_bodies, bodies);
#ifdef DEBUG_MODE
    /* Avoid I/O unless debug-mode is activated */
    for (int64_t i = 0; i < n_bodies; i++) {
      fprintf(output, "%ld %ld %lf %lf\n", t, i, bodies[i].position.x,
        bodies[i].position.y);
    };
#endif
  }
#ifdef DEBUG_MODE
  fclose(output);
#endif
  stop_timer(start);
}

