#include "spacestation.hpp"

int main(int argc, char* argv[]) {
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
  
  int n_supply_vehicles = 3;

  pthread_t vehicle_threads[N_VEHICLES];
  SpaceVehicle* vehicle[N_VEHICLES];
  pthread_t supply_vehicle_threads[n_supply_vehicles];
  SpaceVehicle* supply_vehicle[n_supply_vehicles];

  Station station;
  printf("Starting Fuel Space Station v.1\n");
  station.init(2, 10000, 1000);

  for (int64_t i = 0; i < n_supply_vehicles; i++) {
    supply_vehicle[i] = (SpaceVehicle*) malloc(sizeof(SpaceVehicle));
  }

  supply_vehicle[0]->init(100, true, 3000, 0, 35, 0, &station);
  supply_vehicle[1]->init(101, true, 0, 500, 0, 10, &station);
  supply_vehicle[2]->init(102, true, 2000, 500, 100, 70, &station);

  for (int64_t i = 0; i < n_supply_vehicles; i++) {
    pthread_create(&supply_vehicle_threads[i], &attr, &init_vehicle_thread,
        supply_vehicle[i]);
  }

  for (int64_t i = 0; i < N_VEHICLES; i++) {
    int64_t nitrogen, qufl, qufl_consumption, nitrogen_consumption;
    int vehicle_die = rand() % 5;
    vehicle[i] = (SpaceVehicle*) malloc(sizeof(SpaceVehicle));
    if (vehicle_die == 0) {
      nitrogen = 0;
      nitrogen_consumption = 0;
    } else {
      nitrogen = rand() % 1000;
      nitrogen_consumption = rand() % nitrogen;
    }
    if (vehicle_die == 1) {
      qufl = 0;
      qufl_consumption = 0;
    } else {
      qufl = rand() % 100;
      qufl_consumption = rand() % qufl;
    }

    vehicle[i]->init(i, false, nitrogen, qufl, nitrogen_consumption,
        qufl_consumption, &station);
    pthread_create(&vehicle_threads[i], &attr, &init_vehicle_thread, vehicle[i]);
  }

  for (int64_t i = 0; i < N_VEHICLES; i++)
    pthread_join(vehicle_threads[i], NULL);
}

