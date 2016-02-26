#include "spacestation.hpp"

/* Set up a space vehicle */
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

/* Calculate how much a space vehicle can supply to the station */
int64_t calculate_supply(int64_t total, int64_t consumption) {
  int64_t value = total - consumption;
  if (value < 0) {
    fprintf(stderr, "[ERROR] Supply Vehicle cannot deliever any fuel\n");
    return 0;
  }
  return value;
}

/* Amount of fuel necessary to refuel a tank */
int64_t calculate_refuel(int64_t full, int64_t current) {
  return full - current;
};

/* Vehicle that supplies fuel to the space station */
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

    /* If the refill tank is empty (i.e. only enough fuel to travel),
     * go away and refill the tank */
    if (supply_nitrogen == 0 && supply_qufl == 0) {
      nanosleep(&sleeptime, NULL); 
      this->tank.nitrogen = this->tank.nitrogen_full;
      this->tank.qufl = this->tank.qufl_full;
      nanosleep(&sleeptime, NULL); 
      continue;
    }

    /* Try to supply nitrogen to the space station if possible */
    if (supply_nitrogen) {
      status = this->station->supply(this, supply_nitrogen, 0);
      if (status == SUCCESSFUL) {
        printf("[%ld] Supplying %ld nitrogen to station\n", id, supply_nitrogen);
        this->tank.nitrogen -= supply_nitrogen;
      } else if (status == E_FULL) {
        printf("[%ld] Cannot supply nitrogen. Station full\n", this->id);
      } else if (status == E_NO_PUMPS) {
        /* All pumps are taken. Signal for waiting */
        printf("[%ld] Waiting for pump...\n", this->id);
        pthread_mutex_lock(&this->station->in_queue);
        pthread_cond_wait(&this->station->pump_wait, &this->station->in_queue);
        pthread_mutex_unlock(&this->station->in_queue);
      }
    }

    /* Try to supply quantum fluids to the space station if possible */
    if (supply_qufl) {
      status = this->station->supply(this, 0, supply_qufl);
      if (status == SUCCESSFUL) {
        printf("[%ld] Supplying %ld qufl to station\n", id, supply_qufl);
        this->tank.qufl -= supply_qufl;
      } else if (status == E_FULL) {
        printf("[%ld] Cannot supply quantum fluids. Station full\n", this->id);
      } else if (status == E_NO_PUMPS) {
        /* All pumps are taken. Signal for waiting */
        printf("[%ld] Waiting for pump...\n", this->id);
        pthread_mutex_lock(&this->station->in_queue);
        pthread_cond_wait(&this->station->pump_wait, &this->station->in_queue);
        pthread_mutex_unlock(&this->station->in_queue);
      }
    }
    nanosleep(&sleeptime, NULL); 
  }
}

/* The consumer which will extract fuel from the station */
void SpaceVehicle::consumer() {
  struct timespec sleeptime;
  int64_t status;
  srand(this->id); /* Seed the prng */

  while(this->arrivals < N_ARRIVALS) {
    sleeptime.tv_nsec = 0;
    sleeptime.tv_sec = rand() % 3;

    /* Calculate the necessary amount of fuel to fill the tank */
    int64_t req_nitrogen, req_qufl;
    req_nitrogen = calculate_refuel(this->tank.nitrogen_full,
        this->tank.nitrogen);
    req_qufl = calculate_refuel(this->tank.qufl_full, this->tank.qufl);

    /* Get the fuel from the station or an error message */
    status = this->station->request(this, req_nitrogen, req_qufl);
    if (status == E_NO_FUEL) {
      /* Wait for refueling of the fuel station */
      printf("[%ld] Station is out of fuel. Waiting\n", this->id);
      pthread_mutex_lock(&this->station->in_queue);
      pthread_cond_wait(&this->station->refill_wait, &this->station->in_queue);
      pthread_mutex_unlock(&this->station->in_queue);
    } else if (status == SUCCESSFUL) {
      /* Increment number of arrivals, sleep for a random time and remove
       * fuel from tank for next iteration */
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
      /* All pumps are taken. Signal for waiting */
      printf("[%ld] Waiting for pump...\n", this->id);
      pthread_mutex_lock(&this->station->in_queue);
      pthread_cond_wait(&this->station->pump_wait, &this->station->in_queue);
      pthread_mutex_unlock(&this->station->in_queue);
    }
  }

  /* Terminate when the vehicle have been at the station N_ARRIVALS time */
  printf("[%ld] Shutting down...\n", this->id);
}

/* Decide what kind of worker this vehicle is */
void SpaceVehicle::worker() {
  if (this->supply_vehicle)
    this->supply_worker();
  else
    this->consumer();
};

/* The station calls this method to refill the fuel tank of the vehicle */
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


