#include "spacestation.hpp"

/* Set initial values for the station's variables */
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

  // queue mutex
  pthread_mutex_init(&this->in_queue, NULL);
  pthread_cond_init(&this->pump_wait, NULL);
  pthread_cond_init(&this->refill_wait, NULL);
}

/* Request the space station to refuel the vehicle */
int64_t Station::request(SpaceVehicle* vehicle, int64_t nitrogen, int64_t qufl) {
  int64_t pump = -1;
  struct timespec pumptime;
  pumptime.tv_nsec = 500000000;
  pumptime.tv_sec = 0;

  /* Use ::supply instead if trying to replenish the station's fuel */
  if (nitrogen < 0 || qufl < 0)
    return E_INVALID;

  /* Try to find and occupy a free pump */
  for (int p = 0; p < pump_count; p++) {
    if (pthread_mutex_trylock(&this->pump[p]) == 0) {
      pump = p;
      break;
    }
  }
  /* Tell the vehicle to wait for a free pump */
  if (pump == -1) {
    return E_NO_PUMPS;
  }
  pthread_mutex_lock(&tank_lock);
  /* Tell the vehicle to wait until the station is refuel'd */
  if (nitrogen > this->tank.nitrogen || qufl > this->tank.qufl) {
    /* Tell the vehicle to wait until the station is replenished by a supply
     * vehicle*/
    pthread_mutex_unlock(&tank_lock);
    pthread_mutex_unlock(&this->pump[pump]);
    return E_NO_FUEL;
  } else {
    /* Pump up fuel into the pump */
    this->tank.nitrogen -= nitrogen;
    this->tank.qufl -= qufl;
    pthread_mutex_unlock(&tank_lock);
    /* Pump the fuel into the vehicle */
    vehicle->fill_up(nitrogen, qufl);
    printf("[FSS] %ld units of nitrogen and %ld units of quantum fluids\n",
      this->tank.nitrogen, this->tank.qufl);
    nanosleep(&pumptime, NULL);
  }

  /* Leave the pump */
  pthread_mutex_unlock(&this->pump[pump]);
  /* Tell next-in-line (unfairly) to tread forward */
  pthread_cond_signal(&pump_wait);
  return SUCCESSFUL;
};

int64_t Station::supply(SpaceVehicle* vehicle, int64_t nitrogen, int64_t qufl) {
  int64_t pump = -1;
  int64_t status = SUCCESSFUL;

  /* Use ::request instead if trying to refuel the vehicle */
  if (nitrogen < 0 || qufl < 0)
    return E_INVALID;

  /* Try to occupate a pump */
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
  if (this->tank.nitrogen + nitrogen > this->tank.nitrogen_full ||
      this->tank.qufl + qufl > this->tank.qufl_full) {
    /* Ask the vehicle to wait until the station needs the supplies */
    status = E_FULL; 
  } else {
    /* Replenish the station */
    this->tank.nitrogen += nitrogen;
    this->tank.qufl += qufl;
  }

  pthread_mutex_unlock(&tank_lock);
  pthread_mutex_unlock(&this->pump[pump]);
  if (status == SUCCESSFUL)
    /* Tell the vehicles in queue that the station has been replenished */
    pthread_cond_broadcast(&this->refill_wait);
  return status;
};
