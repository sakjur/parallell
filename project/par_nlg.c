#include "gravn.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <pthread.h>

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
  /* Parallelism */
  bool done;
  pthread_mutex_t lock;
} quads;

typedef struct worker_info {
  int64_t worker_id;
  int64_t total_workers;
  int64_t count;
  int64_t time_limit;
  double cutoff_distance;
  FILE* output;
  quads* root;
} worker_info;

void divide(int64_t, quads*);
void* Worker (void*);

double point_distance(point a, point b) {
  double rv = sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
  return rv;
}

point point_direction(point a, point b) {
  point direction;
  direction.x = b.x - a.x;
  direction.y = b.y - a.y;
  return direction;
}

double point_magnitude(double mass_a, double mass_b, double distance) {
  return ((NEWTON_G*mass_a*mass_b) / pow(distance, 2));
}

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
    pthread_mutex_destroy(&root->lock);
    free(root);
  } else {
    free(root->bodies);
    pthread_mutex_destroy(&root->lock);
    free(root);
  }
}

double distance_to_quad(point* origin, quads* target) {
  /* Calculates the distance to the closes point at the quad target from the
   * point origin */
  
  // FIXME Every point should be within one of the quads
  double deltax = 0, deltay = 0;

  if (origin->x > target->se.x) {
    deltax = origin->x - target->se.x;
  } else if (origin->x < target->nw.x) {
    deltax = target->nw.x - origin->x;
  }

  if (origin->y < target->se.y) {
    deltay = target->se.y - origin->y;
  } else if (origin->y > target->nw.y) {
    deltay = origin->y - target->nw.y;
  }

  if (deltay == 0 && deltax == 0) {
    return 0.0; // The point is within the quad
  } else {
    return sqrt(pow(deltay, 2) + pow(deltax, 2)); // Pythagoras
  }
}

int64_t relevant_forces(body* vec, double cutoff_distance, quads* root) {
  point origin = vec->position;
  int64_t counter = 0;
  for (int i = 0; i < 4; i++) {
    double distance = distance_to_quad(&origin, root->children[i]);
    if (distance > cutoff_distance && root->children[i]->child_count) {
      quads* target = root->children[i];
      double pdistance = point_distance(vec->position, target->center_of_mass);
      double magnitude = point_magnitude(vec->mass, target->sum_mass, pdistance);
      point direction = point_direction(vec->position, target->center_of_mass);
      vec->force.x = vec->force.x + magnitude*direction.x/pdistance;
      vec->force.y = vec->force.y + magnitude*direction.y/pdistance;
      counter++;
    } else if (root->children[i]->child_count > QUADS_MAX_ELEMENTS) {
      counter += relevant_forces(vec, cutoff_distance, root->children[i]);
    } else if (root->children[i]->child_count == 0) {
      continue;
    } else {
      for (int j = 0; j < root->children[i]->child_count; j++) {
        body* target = root->children[i]->bodies[j];
        double pdistance = point_distance(vec->position, target->position);
        if (pdistance == 0) {
          continue;
        }
        double magnitude = point_magnitude(vec->mass, target->mass, pdistance);
        point direction = point_direction(vec->position, target->position);
        vec->force.x = vec->force.x + magnitude*direction.x/pdistance;
        vec->force.y = vec->force.y + magnitude*direction.y/pdistance;
        counter++;
      }
    }
  }
  return counter;
}

void calculate_forces(worker_info* data) {
  int64_t count = data->count;
  quads* root = data->root;
  double cutoff_distance = data->cutoff_distance;
  int64_t comparisons = 0;
  for (int64_t i = data->worker_id; i < count; i+=data->total_workers) {
    comparisons += relevant_forces(root->bodies[i], cutoff_distance, root);
  }
#ifdef DEBUG_MODE
  int64_t naive_approx = (count*count)/2;
  printf("%ld/%ld comparisons, %lf%% saved\n", comparisons, naive_approx,
      100*(1-((1.0*comparisons)/naive_approx)));
#endif
}

void move_bodies(worker_info* data) {
  /* Apply the forces of the bodies using the common apply_deltav function */
  body** vec = data->root->bodies;
  for (int64_t i = data->worker_id; i < data->count; i+=data->total_workers){
    apply_deltav(vec[i]);
  }
}

void insert_body(quads* quad, body* o) {
  quad->bodies[quad->child_count] = o;
  quad->child_count++;

  quad->nw.x = min(quad->nw.x, o->position.x);
  quad->nw.y = max(quad->nw.y, o->position.y);
  quad->se.x = max(quad->se.x, o->position.x);
  quad->se.y = min(quad->se.y, o->position.y);
}

void inner_divide(quads* quad) {
  if (quad->child_count > QUADS_MAX_ELEMENTS) {
    divide(quad->child_count, quad);

    point mass_position_sum;
    mass_position_sum.x = 0;
    mass_position_sum.y = 0;

    for (int i = 0; i < 4; i++) {
      mass_position_sum.x += quad->children[i]->center_of_mass.x
        * quad->children[i]->sum_mass;
      mass_position_sum.y += quad->children[i]->center_of_mass.y
        * quad->children[i]->sum_mass;
    }

    if (quad->sum_mass != 0) {
      quad->center_of_mass.x = mass_position_sum.x / quad->sum_mass;
      quad->center_of_mass.y = mass_position_sum.y / quad->sum_mass;
    }
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

    if (quad->sum_mass != 0) {
      quad->center_of_mass.x = mass_position_sum.x / quad->sum_mass;
      quad->center_of_mass.y = mass_position_sum.y / quad->sum_mass;
    }
  }
}

quads* init_child(int id, point middle, int64_t parent_count) {
    quads* child = malloc(sizeof(quads));
    child->id = id;
    child->child_count = 0;
    child->sum_mass = 0;
    child->center_of_mass.x = 0;
    child->center_of_mass.y = 0;
    child->bodies = malloc(sizeof(body*)*parent_count);
    child->se.x = middle.x;
    child->se.y = middle.y;
    child->nw.x = middle.x;
    child->nw.y = middle.y;
    child->done = false;
    pthread_mutex_init(&child->lock, NULL);
    return child;
}

point get_middle(int64_t count, body** vec) {
  point middle;
  middle.x = vec[0]->position.x;
  middle.y = vec[0]->position.y;
  for (int64_t i = 1; i < count; i++) {
    middle.x += vec[i]->position.x;
    middle.y += vec[i]->position.y;
  }
  middle.x = middle.x / count;
  middle.y = middle.y / count;
  return middle;
}

void divide(int64_t count, quads* root) {
  body** vec = root->bodies;
  point middle = get_middle(count, vec);
  root->sum_mass = 0;
  for (int i = 0; i < 4; i++) {
    root->children[i] = init_child(i, middle, count);
  }

  for (int64_t i = 0; i < count; i++) {
    root->sum_mass += vec[i]->mass;
    if (vec[i]->position.y > middle.y) { // N
      if (vec[i]->position.x > middle.x) { // NE
        insert_body(root->children[0], vec[i]);
      } else if (vec[i]->position.x <= middle.x) { // NW
        insert_body(root->children[1], vec[i]);
      }
    } else if (vec[i]->position.y <= middle.y) { // S
      if (vec[i]->position.x < middle.x) { // SW
        insert_body(root->children[2], vec[i]);
      } else if (vec[i]->position.x >= middle.x){ // SE
        insert_body(root->children[3], vec[i]);
      }
    } else {
      printf("Error! x %lf y %lf\n", vec[i]->position.x, vec[i]->position.y);
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
  int n_workers = WORKERS_DEFAULT;

  /* Command line arguments */
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
  if (argc > 4) {
    cutoff_distance = atof(argv[4]);
  }

  /* The thread variables and properties for the workers */
  pthread_t worker_threads[n_workers];
  worker_info workers_data[n_workers];

  /* set global thread attributes */
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);


  /* Initialize the bodies at their first position */

  point origo;
  origo.x = 0;
  origo.y = 0;
  quads* root = init_child(0, origo, n_bodies);
#ifdef DEBUG_MODE
  FILE* output = fopen("output", "w");
#endif

  printf("[simulation] %d bodies over %d time steps with %d workers -- nlogn\n",
      n_bodies, time_limit, n_workers);
  struct timeval start = start_timer();
  for (int w = 0; w < n_workers; w++) {
    /* Iterate over all the workers and create their properties */
    workers_data[w].worker_id = w;
    workers_data[w].total_workers = n_workers;
    workers_data[w].count = n_bodies;
    workers_data[w].root = root;
#ifdef DEBUG_MODE
    workers_data[w].output = output;
#endif
    workers_data[w].time_limit = time_limit;
    workers_data[w].cutoff_distance = cutoff_distance;
  }
  for (int w = 0; w < n_workers; w++) {
    pthread_create(&worker_threads[w], &attr, Worker, (void*) &workers_data[w]);
  }
  for (int w = 0; w < n_workers; w++) {
    pthread_join(worker_threads[w], NULL);
  }
  stop_timer(start);
#ifdef DEBUG_MODE
  fclose(output);
#endif

}

void* Worker (void* in) {
  worker_info* data = (worker_info*) in;
  quads* root = data->root;

  for (int i = data->worker_id; i < data->count; i+=data->total_workers) {
    root->bodies[i] = malloc(sizeof(body));
    row_of_twenty(root->bodies[i], i);
    root->nw.x = min(root->nw.x, root->bodies[i]->position.x);
    root->nw.y = max(root->nw.y, root->bodies[i]->position.y);
    root->se.x = max(root->se.x, root->bodies[i]->position.x);
    root->se.y = min(root->se.y, root->bodies[i]->position.y);
  }
  barrier(data->total_workers);

  /* Do simulation */
  for (int64_t t = 0; t < data->time_limit; t++) {
    if (data->worker_id == 0)
      divide(data->count, root);
    barrier(data->total_workers);
    calculate_forces(data);
    barrier(data->total_workers);
    move_bodies(data);
    barrier(data->total_workers);
    if (data->worker_id == 0)
      clean_tree(root, 0);
    barrier(data->total_workers);
#ifdef DEBUG_MODE
    /* Avoid I/O unless debug-mode is activated */
    for (int64_t i = 0; i < data->count; i++) {
      fprintf(data->output, "%ld %ld %lf %lf\n", t, i, root->bodies[i]->position.x,
         root->bodies[i]->position.y);
    };
#endif
  }
  return NULL;
}
