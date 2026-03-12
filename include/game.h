#ifndef GAME_H_
#define GAME_H_

#include"constants.h"

typedef struct tile {
  int x;
  int y;
  int is_hit;
} tile_t;

typedef struct ship {
  tile_t body[SHIP_SIZE];
} ship_t;

// ship
int init_ship(ship_t *ship, int x, int y, char dir);
int attack(ship_t *ship, int x, int y);
int is_sunk(ship_t *ship);

#endif
