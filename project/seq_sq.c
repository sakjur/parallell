#include "gravn.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/time.h>

void calculate_forces(int64_t count, body* vec) {
  for (int64_t i = 0; i < count; i++) {
    for (int64_t j = i + 1; j < count; j++) {
      double distance = sqrt(pow(vec[i].position.x - vec[j].position.x, 2) +
          pow(vec[i].position.y - vec[j].position.y, 2));
      double magnitude = (NEWTON_G*vec[i].mass*vec[i].mass) /
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
  for (int64_t i = 0; i < count; i++){
    apply_deltav(&vec[i]);
  }
}

int main (int argc, char* argv[]) {
  int time_limit = TIME_DEFAULT;
  int n_bodies = BODIES_DEFAULT;

  if (argc > 1) {
    n_bodies = atoi(argv[1]);
  }
  if (argc > 2) {
    time_limit = atoi(argv[2]);
  }

  body bodies[n_bodies];
  memset(bodies, 0, sizeof(body) * n_bodies);
  for (int i = 0; i < n_bodies; i++) {
    row_of_twenty(&bodies[i], i);
  }

  printf("[simulation] %d bodies over %d time steps\n", n_bodies, time_limit);
  struct timeval start = start_timer();
  for (int64_t t = 0; t < time_limit; t++) {
    calculate_forces(n_bodies, bodies);
    move_bodies(n_bodies, bodies);
#ifdef DEBUG_MODE
    FILE* output = fopen("output", "w");
    for (int64_t i = 0; i < n_bodies; i++) {
      fprintf(output, "%ld %ld %lf %lf\n", t, i, bodies[i].position.x,
        bodies[i].position.y);
    };
#endif
  }
  stop_timer(start);
}

