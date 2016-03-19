#include "gravn.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

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
    point deltav;
    point deltap;
    deltav.x = vec[i].force.x/vec[i].mass * DELTA_T;
    deltav.y = vec[i].force.y/vec[i].mass * DELTA_T;

    deltap.x = vec[i].velocity.x + deltav.x/2 * DELTA_T;
    deltap.y = vec[i].velocity.y + deltav.y/2 * DELTA_T;

    vec[i].velocity.x = vec[i].velocity.x + deltav.x;
    vec[i].velocity.y = vec[i].velocity.y + deltav.y;
    vec[i].position.x = vec[i].position.x + deltap.x;
    vec[i].position.y = vec[i].position.y + deltap.y;
    vec[i].force.x = 0;
    vec[i].force.y = 0;
  }
}

void print_body(body b, bool more_items, FILE* output) {
  char comma = ' ';
  if (more_items) comma = ',';
  fprintf(output, "    \"%ld\": {",
      b.id);
  fprintf(output, "\"x\": %lf, \"y\": %lf,", b.position.x, b.position.y);
  fprintf(output, "\"vx\": %lf, \"vy\": %lf", b.velocity.x, b.velocity.y);
  fprintf(output, "}%c\n", comma);
}

int main (int argc, char* argv[]) {
  int time_limit = 2000;
  int n_bodies = 120;
  FILE* output = fopen("output.json", "w");

  body bodies[n_bodies];
  memset(bodies, 0, sizeof(body) * n_bodies);
  for (int i = 0; i < n_bodies; i++) {
    bodies[i].id = i;
    bodies[i].position.x = i*0.05;
    bodies[i].position.y = i*0.05;
    bodies[i].mass = 10000;
  }
  fprintf(output, "{");
  for (int64_t t = 0; t < time_limit; t++) {
    calculate_forces(n_bodies, bodies);
    move_bodies(n_bodies, bodies);
    fprintf(output, "  \"%ld\": {\n", t);
    for (int64_t i = 0; i < n_bodies; i++) {
      if (i == n_bodies-1)
        print_body(bodies[i], false, output);
      else
        print_body(bodies[i], true, output);
    };
    if (t == time_limit-1) 
      fprintf(output, "  }\n");
    else
      fprintf(output, "  },\n");
  }
  fprintf(output, "}\n");
}

