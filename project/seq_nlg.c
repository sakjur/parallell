#include "gravn.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#define CUTOFF_DISTANCE_DEFAULT 2
#define QUADS_MAX_ELEMENTS 5

typedef struct quads {
  int64_t id;
  struct quads* children[4];
  body** bodies;
  int64_t child_count;
  double sum_mass;
  point center_of_mass;
  /* Corners for the square */
  point nw;
  point se;
} quads;

void divide(int64_t, quads*);

/*
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
*/

void clean_tree(quads* root, int64_t level) {
  if (level == 0) {
    for (int i = 0; i < 4; i++) {
      clean_tree(root->children[i], level+1);
    }
  }
  else if (root->child_count > QUADS_MAX_ELEMENTS) {
    for (int i = 0; i < 4; i++) {
      clean_tree(root->children[i], level+1);
    }
    free(root->bodies);
    free(root);
  } else {
    free(root->bodies);
    free(root);
  }
}

void calculate_forces(int64_t count, quads* root, double cutoff_distance) {
  body** vec = root->bodies;
  for (int64_t i = 0; i < count; i++) {
    for (int64_t j = i + 1; j < count; j++) {
      double distance = sqrt(pow(vec[i]->position.x - vec[j]->position.x, 2) +
          pow(vec[i]->position.y - vec[j]->position.y, 2));
      double magnitude = (NEWTON_G*vec[i]->mass*vec[i]->mass) /
        (pow(distance, 2));
      point direction;
      direction.x = vec[j]->position.x - vec[i]->position.x;
      direction.y = vec[j]->position.y - vec[i]->position.y;
      
      vec[i]->force.x = vec[i]->force.x + magnitude*direction.x/distance;
      vec[j]->force.x = vec[j]->force.x - magnitude*direction.x/distance;
      vec[i]->force.y = vec[i]->force.y + magnitude*direction.y/distance;
      vec[j]->force.y = vec[j]->force.y - magnitude*direction.y/distance;
    }
  }
}

void move_bodies(int64_t count, quads* root) {
  /* Apply the forces of the bodies using the common apply_deltav function */
  body** vec = root->bodies;
  for (int64_t i = 0; i < count; i++){
    apply_deltav(vec[i]);
  }
}

void insert_body(quads* quad, body* o) {
  quad->bodies[quad->child_count] = o;
  quad->child_count++;

  if (quad->nw.x > o->position.x) {
    quad->nw.x = o->position.x;
  }
  if (quad->nw.y < o->position.y) {
    quad->nw.y = o->position.y;
  }
  if (quad->se.x < o->position.x) {
    quad->se.x = o->position.x;
  }
  if (quad->se.y > o->position.y) {
    quad->se.y = o->position.y;
  }


}

void inner_divide(quads* quad) {
  if (quad->child_count > QUADS_MAX_ELEMENTS) {
    divide(quad->child_count, quad);

    point mass_position_sum;
    mass_position_sum.x = 0;
    mass_position_sum.y = 0;

    for (int i = 0; i < 4; i++) {
      mass_position_sum.x += quad->children[i]->center_of_mass.x;
      mass_position_sum.y += quad->children[i]->center_of_mass.y;
    }

    quad->center_of_mass.x = mass_position_sum.x / quad->sum_mass;
    quad->center_of_mass.y = mass_position_sum.y / quad->sum_mass;
  } else {
    point mass_position_sum;
    mass_position_sum.x = 0;
    mass_position_sum.y = 0;

    for (int child = 0; child < quad->child_count; child++) {
      mass_position_sum.x += quad->bodies[child]->position.x *
        quad->bodies[child]->mass;
      mass_position_sum.y += quad->bodies[child]->position.y *
        quad->bodies[child]->mass;
      quad->sum_mass += quad->bodies[child]->mass;
    }

    quad->center_of_mass.x = mass_position_sum.x / quad->sum_mass;
    quad->center_of_mass.y = mass_position_sum.y / quad->sum_mass;
  }
}

quads* init_child(int id, point middle, int64_t parent_count) {
    quads* child = malloc(sizeof(quads));
    child->id = id;
    child->child_count = 0;
    child->sum_mass = 0;
    child->bodies = malloc(sizeof(body)*parent_count);
    child->se.x = middle.x;
    child->se.y = middle.y;
    child->nw.x = middle.x;
    child->nw.y = middle.y;
    return child;
}

point get_middle(int64_t count, body* vec) {
  point middle;
  middle.x = vec[0].position.x;
  middle.y = vec[0].position.y;
  for (int64_t i = 1; i < count; i++) {
    middle.x += vec[i].position.x;
    middle.y += vec[i].position.y;
  }
  middle.x = middle.x / count;
  middle.y = middle.y / count;
  return middle;
}

void divide(int64_t count, quads* root) {
  point middle = get_middle(count, *root->bodies);
  root->sum_mass = 0;
  for (int i = 0; i < 4; i++) {
    root->children[i] = init_child(i, middle, count);
  }

  body* vec = *root->bodies;
  for (int64_t i = 0; i < count; i++) {
    if (vec[i].position.y > middle.y) { // N
      root->sum_mass += vec[i].mass;
      if (vec[i].position.x > middle.x) { // NE
        insert_body(root->children[0], &vec[i]);
      } else if (vec[i].position.x <= middle.x) { // NW
        insert_body(root->children[1], &vec[i]);
      }
    } else if (vec[i].position.y <= middle.y) { // S
      if (vec[i].position.x < middle.x) { // SW
        insert_body(root->children[2], &vec[i]);
      } else if (vec[i].position.x >= middle.x){ // SE
        insert_body(root->children[3], &vec[i]);
      }
    } else {
      printf("Error! x %lf y %lf\n", vec[i].position.x, vec[i].position.y);
    }
  }
  for (int i = 0; i < 4; i++) {
    inner_divide(root->children[i]);
  }
}

int main(int argc, char* argv[]) {
  /* Run the simulation */
  int time_limit = TIME_DEFAULT;
  int n_bodies = BODIES_DEFAULT;
  double cutoff_distance = CUTOFF_DISTANCE_DEFAULT;

  /* Command line arguments */
  if (argc > 1) {
    n_bodies = atoi(argv[1]);
  }
  if (argc > 2) {
    time_limit = atoi(argv[2]);
  }

  /* Initialize the bodies at their first position */
  quads* root = malloc(sizeof(quads));
  root->bodies = malloc(sizeof(body*)*n_bodies);
  memset(root->bodies, 0, sizeof(body) * n_bodies);
  for (int i = 0; i < n_bodies; i++) {
    root->bodies[i] = malloc(sizeof(body));
    row_of_twenty(root->bodies[i], i);
  }


  printf("[simulation] %d bodies over %d time steps -- nlogn\n", n_bodies, time_limit);
  struct timeval start = start_timer();
#ifdef DEBUG_MODE
  FILE* output = fopen("output", "w");
#endif
  /* Do simulation */
  for (int64_t t = 0; t < time_limit; t++) {
    divide(n_bodies, root);
    calculate_forces(n_bodies, root, cutoff_distance);
    move_bodies(n_bodies, root);
    clean_tree(root, 0);
#ifdef DEBUG_MODE
    /* Avoid I/O unless debug-mode is activated */
    for (int64_t i = 0; i < n_bodies; i++) {
      fprintf(output, "%ld %ld %lf %lf\n", t, i, root->bodies[i]->position.x,
         root->bodies[i]->position.y);
    };
#endif
  }
  stop_timer(start);
#ifdef DEBUG_MODE
  fclose(output);
#endif

}
