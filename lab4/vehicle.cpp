#include "spacestation.hpp"

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

int64_t calculate_supply(int64_t total, int64_t consumption) {
  int64_t value = total - consumption;
  if (value < 0) {
    fprintf(stderr, "[ERROR] Supply Vehicle cannot deliever any fuel\n");
    return 0;
  }
  return value;
}

int64_t calculate_refuel(int64_t full, int64_t current) {
  return full - current;
};

void SpaceVehicle::supply_worker() {
  struct timespec sleeptime;
  int64_t status;
  srand(this->id);
  while (true) {
    sleeptime.tv_nsec = 0;
    int64_t supply_nitrogen, supply_qufl;
    sleeptime.tv_sec = rand() % 7;
    supply_nitrogen = calculate_supply(this->tank.nitrogen,
        this->consumption_nitrogen);
    supply_qufl = calculate_supply(this->tank.qufl,
        this->consumption_qufl);
    if (supply_nitrogen == 0 && supply_qufl == 0) {
      nanosleep(&sleeptime, NULL); 
      this->tank.nitrogen = this->tank.nitrogen_full;
      this->tank.qufl = this->tank.qufl_full;
      nanosleep(&sleeptime, NULL); 
      continue;
    }
    if (supply_nitrogen) {
      status = this->station->supply(this, supply_nitrogen, 0);
      if (status == SUCCESSFUL) {
        printf("[%ld] Supplying %ld nitrogen to station\n", id, supply_nitrogen);
        this->tank.nitrogen -= supply_nitrogen;
      } else if (status == E_FULL) {
        printf("[%ld] Cannot supply nitrogen. Station full\n", this->id);
      }
    }
    if (supply_qufl) {
      status = this->station->supply(this, 0, supply_qufl);
      if (status == SUCCESSFUL) {
        printf("[%ld] Supplying %ld qufl to station\n", id, supply_qufl);
        this->tank.qufl -= supply_qufl;
      } else if (status == E_FULL) {
        printf("[%ld] Cannot supply quantum fluids. Station full\n", this->id);
      }
    }
    nanosleep(&sleeptime, NULL); 
  }
}

void SpaceVehicle::consumer() {
  struct timespec sleeptime;
  int64_t status;
  srand(this->id);
  while(this->arrivals < N_ARRIVALS) {
    sleeptime.tv_nsec = 0;
    sleeptime.tv_sec = rand() % 3;
    int64_t req_nitrogen, req_qufl;
    req_nitrogen = calculate_refuel(this->tank.nitrogen_full,
        this->tank.nitrogen);
    req_qufl = calculate_refuel(this->tank.qufl_full, this->tank.qufl);
    status = this->station->request(this, req_nitrogen, req_qufl);
    if (status == E_NO_FUEL) {
      printf("[%ld] Station is out of fuel. Waiting\n", this->id);
      pthread_mutex_lock(&this->station->in_queue);
      pthread_cond_wait(&this->station->refill_wait, &this->station->in_queue);
      pthread_mutex_unlock(&this->station->in_queue);
    } else if (status == SUCCESSFUL) {
      this->arrivals++;
      for (int trips = rand() % 4 + 1; trips > 0; trips--) {
        sleeptime.tv_sec = 1;
        sleeptime.tv_nsec = rand() % 1000000000;
        nanosleep(&sleeptime, NULL);
        if (this->tank.nitrogen < this->consumption_nitrogen ||
            this->tank.qufl < this->consumption_qufl) {
          this->tank.nitrogen = 0;
          this->tank.qufl = 0;
          break;
        } else {
          this->tank.nitrogen -= this->consumption_nitrogen;
          this->tank.qufl -= this->consumption_qufl;
        }
      }
    } else if (status == E_NO_PUMPS) {
      printf("[%ld] Waiting for pump...\n", this->id);
      pthread_mutex_lock(&this->station->in_queue);
      pthread_cond_wait(&this->station->pump_wait, &this->station->in_queue);
      pthread_mutex_unlock(&this->station->in_queue);
    }
  }

  printf("[%ld] Shutting down...\n", this->id);
}

void SpaceVehicle::worker() {
  if (this->supply_vehicle)
    this->supply_worker();
  else
    this->consumer();
};

void SpaceVehicle::fill_up(int64_t nitrogen, int64_t qufl) {
  this->tank.nitrogen += nitrogen;
  this->tank.qufl += qufl;
  printf("[%ld] %ld nitrogen and %ld qufl\n", this->id,
      this->tank.nitrogen, this->tank.qufl);
};


void* init_vehicle_thread(void* arg) {
  SpaceVehicle* vehicle = (SpaceVehicle*) arg;
  vehicle->worker();
  return NULL;
};


