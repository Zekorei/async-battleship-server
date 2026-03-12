#ifndef _PARSER_H
#define _PARSER_H

#include"player.h"
#include"buffer.h"

typedef enum {
  CMD_UNKNOWN,
  CMD_REG,
  CMD_BOMB
} command_type_t;


typedef struct {
  command_type_t type;
  char name[MAX_NAME_SIZE];
  int x,y;
  char dir;
} command_t;

int validate_name(const char *name);
int parse_command(const char *cmd, command_t *out);
void handle_command(player_t *player, const char *cmd, player_t *player_list, int epoll_fd);

void register_player(player_t *player, command_t cmd, player_t *player_list);
void bomb_player(player_t *player, command_t cmd, player_t *player_list, int epoll_fd);

#endif
