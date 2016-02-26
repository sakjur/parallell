#pragma once

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* CONSTANTS! */
#define N_VEHICLES 8
#define N_ARRIVALS 10

#define SUCCESSFUL 0
#define E_NO_FUEL -1
#define E_NO_PUMPS -2
#define E_INVALID -3
#define E_FULL -4

/* The tank_t type is used to describe a fuel tank */
typedef struct tank_t {
  int64_t nitrogen;
  int64_t qufl;
  int64_t nitrogen_full;
  int64_t qufl_full;
} tank_t;

/* Pre-define the Station class since it's used by SpaceVehicle */
class Station;

/* A vehicle which requests or supplies fuel with a fuel space station */
class SpaceVehicle {
  public:
    void init(int64_t, bool, int64_t, int64_t, int64_t, int64_t, Station*);
    void worker();
    void fill_up(int64_t, int64_t);
  private:
    int64_t arrivals = 0;
    void supply_worker();
    void consumer();
    int64_t id;
    Station* station;
    bool supply_vehicle;
    /* Amount of fuel used by this vehicle for 1 travel unit */
    int64_t consumption_nitrogen;
    int64_t consumption_qufl;
    /* The fuel tank of the vehicle
     * Should be much, much larger on supply vehicles */
    tank_t tank;
};

/* Station Class which handles a fuel space station with n pumps */
class Station {
  public:
    void init(int64_t, int64_t, int64_t);
    int64_t request(SpaceVehicle*, int64_t, int64_t);
    int64_t supply(SpaceVehicle*, int64_t, int64_t);
    pthread_cond_t pump_wait;
    pthread_cond_t refill_wait;
    pthread_cond_t supply_wait;
    pthread_mutex_t in_queue;
  private:
    int64_t pump_count;
    pthread_mutex_t* pump;
    pthread_mutex_t tank_lock;
    tank_t tank;
};

/* Wrapper to start a vehicle worker (since method pointers are not legal) */
void* init_vehicle_thread(void* arg);
