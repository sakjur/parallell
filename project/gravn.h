#pragma once
#include <stdint.h>
#include <sys/time.h>

#define TIME_DEFAULT 200
#define BODIES_DEFAULT 120
#define WORKERS_DEFAULT 2

typedef struct point {
	double x;
	double y;
} point;

typedef struct body {
	int64_t id;
	point position;
	point velocity;
	point force;
	double mass;
} body;

// 6.67e-11
#define NEWTON_G (.0000000000667)
#define DELTA_T 1

void row_of_twenty(body*, int64_t);
struct timeval start_timer();
void stop_timer(struct timeval);
void apply_deltav(body*);
void barrier(int64_t total_workers);
