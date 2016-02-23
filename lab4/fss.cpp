#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define N_VEHICLES 8

#define SUCCESSFUL 1
#define E_NO_FUEL -1
#define E_NO_PUMPS -2
#define E_INVALID -3

typedef struct tank_t {
  int64_t nitrogen;
  int64_t qufl;
  int64_t nitrogen_full;
  int64_t qufl_full;
} tank_t;

class Station;

/* A vehicle which requests or supplies fuel with a fuel space station */
class SpaceVehicle {
  public:
    void init(int64_t, bool, int64_t, int64_t, int64_t, int64_t, Station*);
    void* worker(void*);
    void fill_up(int64_t, int64_t);
  private:
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
  private:
    int64_t pump_count;
    pthread_mutex_t* pump;
    pthread_mutex_t tank_lock;
    tank_t tank;
};

void SpaceVehicle::init(int64_t id,
    bool supply_vehicle, int64_t nitrogen, int64_t qufl,
    int64_t nitrogen_consumption, int64_t qufl_consumption,
    Station* station) {
  this->tank.nitrogen_full = nitrogen;
  this->tank.nitrogen = nitrogen;
  this->tank.qufl_full = qufl;
  this->tank.qufl = qufl;
  this->consumption_nitrogen = nitrogen_consumption;
  this->consumption_qufl = qufl_consumption;
  this->supply_vehicle = supply_vehicle;
  this->station = station;
  this->id = id;
}

void* SpaceVehicle::worker(void*) {
  struct timespec sleeptime;
  int64_t status;
  srand(this->id);
  while (true) {
    sleeptime.tv_nsec = 0;
    if (!this->supply_vehicle){
      sleeptime.tv_sec = rand() % 3;
      status = this->station->request(this, 100, 10);
      nanosleep(&sleeptime, NULL);
    } else {
      printf("[%ld] Supplying to station\n", id);
      sleeptime.tv_sec = rand() % 7;
      status = this->station->supply(this, 1000, 100);
      nanosleep(&sleeptime, NULL); 
    }
    if (status == E_NO_FUEL) {
      printf("Station is out of fuel. Waiting\n");
    }
  }

  return NULL;
};

void SpaceVehicle::fill_up(int64_t nitrogen, int64_t qufl) {
  this->tank.nitrogen += nitrogen;
  this->tank.qufl += qufl;
  printf("[%ld] %ld / %ld nitrogen and %ld / %ld qufl\n", this->id,
      this->tank.nitrogen, this->tank.nitrogen_full,
      this->tank.qufl, this->tank.qufl_full);
};

void Station::init(int64_t pumps, int64_t nitrogen, int64_t qufl) {
  printf("[FSS] Opening with %ld units of nitrogen and %ld units of quantum fluids\n",
      nitrogen, qufl);
  this->tank.nitrogen_full = nitrogen;
  this->tank.nitrogen = nitrogen;
  this->tank.qufl_full = qufl;
  this->tank.qufl = qufl;

  /* The lock for pumping up fuel from the tank into the pump */
  pthread_mutex_init(&this->tank_lock, NULL);

  /* The pumps are represented by n locks */
  this->pump_count = pumps;
  this->pump = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t) * pumps);
  for (int i = 0; i < pumps; i++)
    pthread_mutex_init(&this->pump[i], NULL);

  // TODO Initialize conds
}

int64_t Station::request(SpaceVehicle* vehicle, int64_t nitrogen, int64_t qufl) {
  int64_t pump = -1;

  if (nitrogen < 0 || qufl < 0)
    return E_INVALID;
  for (int p = 0; p < pump_count; p++) {
    if (pthread_mutex_trylock(&this->pump[p]) == 0) {
      pump = p;
      break;
    }
  }
  if (pump == -1) {
    return E_NO_PUMPS;
  }
  pthread_mutex_lock(&tank_lock);
  /* Tell the vehicle to wait until the station is refuel'd */
  if (nitrogen > this->tank.nitrogen || qufl > this->tank.qufl) {
    pthread_mutex_unlock(&tank_lock);
    pthread_mutex_unlock(&this->pump[pump]);
    return E_NO_FUEL;
  } else {
    this->tank.nitrogen -= nitrogen;
    this->tank.qufl -= qufl;
    pthread_mutex_unlock(&tank_lock);
    vehicle->fill_up(nitrogen, qufl);
    printf("[FSS] %ld units of nitrogen and %ld units of quantum fluids\n",
      this->tank.nitrogen, this->tank.qufl);
  }

  pthread_mutex_unlock(&this->pump[pump]);
  return SUCCESSFUL;
};
int64_t Station::supply(SpaceVehicle* vehicle, int64_t nitrogen, int64_t qufl) {
  if (nitrogen < 0 || qufl < 0)
    return E_INVALID;
  pthread_mutex_lock(&tank_lock);
  this->tank.nitrogen += nitrogen;
  this->tank.qufl += qufl;
  pthread_mutex_unlock(&tank_lock);
  return SUCCESSFUL;
};

void* init_vehicle_thread(void* arg) {
  SpaceVehicle* vehicle = (SpaceVehicle*) arg;
  vehicle->worker(arg);
  return NULL;
};

int main(int argc, char* argv[]) {
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
  
  pthread_t vehicle_threads[N_VEHICLES];
  SpaceVehicle* vehicle[N_VEHICLES];

  Station station;
  printf("Starting Fuel Space Station v.1\n");
  station.init(4, 10000, 1000);

  for (int64_t i = 0; i < N_VEHICLES; i++) {
    bool supply_vehicle = false;
    vehicle[i] = (SpaceVehicle*) malloc(sizeof(SpaceVehicle));
    if (i % 5 == 0)
      supply_vehicle = true;
    vehicle[i]->init(i, supply_vehicle, 1000, 100, 100, 10, &station);
    pthread_create(&vehicle_threads[i], &attr, &init_vehicle_thread, vehicle[i]);
  }

  for (int64_t i = 0; i < N_VEHICLES; i++)
    pthread_join(vehicle_threads[i], NULL);
}

