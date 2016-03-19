#pragma once
#include <stdint.h>

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
