#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

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

class SpaceVehicle {
  public:
    void* Worker(void*);
    void fill_up(int64_t nitrogen, int64_t qufl) {
      this->tank.nitrogen += nitrogen;
      this->tank.qufl += qufl;
      printf("Filled tank with %ld units of nitrogen and %ld units of quantum fluids\n",
          nitrogen, qufl);
    };
  private:
    bool supply_vehicle;
    /* Amount of fuel used by this vehicle for 1 travel unit */
    int64_t consumption_nitrogen;
    int64_t consumption_qufl;
    /* The fuel tank of the vehicle
     * Should be much, much larger on supply vehicles */
    tank_t tank;
};

class Station {
  public:
    void init(int64_t pumps, int64_t nitrogen, int64_t qufl) {
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
    int64_t request(SpaceVehicle* vehicle, int64_t nitrogen, int64_t qufl) {
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
        return E_NO_FUEL;
      } else {
        this->tank.nitrogen -= nitrogen;
        this->tank.qufl -= qufl;
        pthread_mutex_unlock(&tank_lock);
        vehicle->fill_up(nitrogen, qufl);
      }

      return SUCCESSFUL;
    };


    int64_t supply(SpaceVehicle* vehicle, int64_t nitrogen, int64_t qufl) {
      return E_INVALID;
    };

    pthread_cond_t pump_wait;
    pthread_cond_t refill_wait;
  private:
    int64_t pump_count;
    pthread_mutex_t* pump;
    pthread_mutex_t tank_lock;
    tank_t tank;
};

Station main_station;

int main(int argc, char* argv[]) {
  printf("Starting Fuel Space Station v.1\n");
  main_station.init(4, 10000, 1000);
}

