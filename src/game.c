#include<stdlib.h>
#include<stdio.h>

#include"game.h"
#include"constants.h"

/* sets `ship` to NULL if center is invalid */
int init_ship(ship_t *ship, int x, int y, char d) {
  if (!ship) return 0;

  // initialize tiles
  for (int i = 0; i < SHIP_SIZE; i++) {
    int tile_x = (d == '-') ? (x - 2 + i) : x;
    int tile_y = (d == '|') ? (y - 2 + i) : y;

    if (tile_x < BOARD_MIN || tile_x >= BOARD_MAX ||
        tile_y < BOARD_MIN || tile_y >= BOARD_MAX) {
      return 0;
    }

    ship->body[i].x = tile_x;
    ship->body[i].y = tile_y;
    ship->body[i].is_hit = 0;
  }
  return 1;
}

int attack(ship_t *ship, int x, int y) {
  for (int i = 0; i < SHIP_SIZE; i++) {
//    fprintf(stderr, "[attack] ship (%d, %d) -> bomb (%d, %d)\n", ship->body[i].x, ship->body[i].y, x, y);
    if (ship->body[i].x == x && ship->body[i].y == y) {
      ship->body[i].is_hit = 1;
      return 1;
    }
  }
  return 0;
}

int is_sunk(ship_t *ship) {
  for (int i = 0; i < SHIP_SIZE; i++) {
    fprintf(stderr, "[sunk] point (%d, %d): hit? %d\n", ship->body[i].x, ship->body[i].y, ship->body[i].is_hit);
    if (ship->body[i].is_hit == 0) {
      return 0;
    }
  }
  return 1;
}
