#include<string.h>
#include<stdio.h>
#include<ctype.h>

#include"constants.h"
#include"game.h"
#include"parser.h"
#include"player.h"

int validate_name(const char *name) {
  size_t len = strlen(name);
  if (len == 0 || len > 20) return 0;
  for (size_t i = 0; i < len; i++) {
    char c = name[i];
    if(!isalnum((unsigned char)c) && c != '-') return 0;
  }
  return 1;
}

int parse_command(const char *cmd, command_t *out) {
  if (!cmd || !out) return 0;

  // try REG command
  char name[MAX_NAME_SIZE];
  int x,y;
  char dir;
  if (sscanf(cmd, "REG %20s %d %d %c\n", name, &x, &y, &dir) == 4) {
    out->type = CMD_REG;
    strncpy(out->name, name, MAX_NAME_SIZE - 1);
    out->name[MAX_NAME_SIZE - 1] = '\0';
    out->x = x;
    out->y = y;
    out->dir = dir;
    return 1;
  }

  // try BOMB command
  if (sscanf(cmd, "BOMB %d %d\n", &x, &y) == 2) {
    out->type = CMD_BOMB;
    out->x = x;
    out->y = y;
    return 1;
  }

  out->type = CMD_UNKNOWN;
  return 0;
}


void handle_command(player_t *player, const char *cmd, player_t *player_list, int epoll_fd) {
  command_t command;
  if (!parse_command(cmd, &command)) {
    fprintf(stderr, "handle_command: invalid command from %d\n", player->fd);
    tell(player, MSG_INVALID);
    return;
  }

  switch (command.type) {
    case CMD_REG:
      register_player(player, command, player_list);
      break;

    case CMD_BOMB:
      fprintf(stderr, "handle_command: bombing attempt by %d\n", player->fd);
      bomb_player(player, command, player_list, epoll_fd);
      break;

    default:
      tell(player, MSG_INVALID);
      break;
  }
}

// commands
void register_player(player_t *player, command_t cmd, player_t *player_list) {
  if (player->registered == 1) {
    fprintf(stderr, "register_player: already registered (%d)\n", player->fd);
    tell(player, MSG_INVALID);
    return;
  }

  if (validate_name(cmd.name) == 0) {
    fprintf(stderr, "register_player: invalid name (%d)\n", player->fd);
    tell(player, MSG_INVALID);
    return;
  }

  if (find_player_by_name(player_list, cmd.name)) {
    fprintf(stderr, "register_player: name taken (%d)\n", player->fd);
    tell(player, "TAKEN\n");
    return;
  }

  if (init_ship(&player->ship, cmd.x, cmd.y, cmd.dir) == 0) {
    fprintf(stderr, "register_player: invalid ship coords(%d)\n", player->fd);
    tell(player, MSG_INVALID);
    return;
  }

  setup_player(player, cmd.name);

  char msg_join[30];
  snprintf(msg_join, sizeof(msg_join), "JOIN %s\n", player->name);
  tell(player, "WELCOME\n");
  broadcast(player_list, msg_join);
}

void bomb_player(player_t *player, command_t cmd, player_t *player_list, int epoll_fd) {
  int x = cmd.x;
  int y = cmd.y;
  int is_hit = 0;

  for (player_t *victim = player_list; victim != NULL; victim = victim->next) {
    if (victim->registered == 0) {
      fprintf(stderr, "bomb_player: unregistered player\n");
      continue;
    }

    if (attack(&victim->ship, x, y) == 1) {
      is_hit = 1;

      char hit_msg[60];
      snprintf(hit_msg, sizeof(hit_msg), "HIT %s %d %d %s\n",
        player->name, x, y, victim->name);
      broadcast(player_list, hit_msg);
    }
  }

  if (is_hit == 0) {
    char miss_msg[40];
    snprintf(miss_msg, sizeof(miss_msg), "MISS %s %d %d\n", player->name, x, y);
    broadcast(player_list, miss_msg);
  }

  cleanup_players(&player_list, epoll_fd);
}

