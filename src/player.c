#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>

#include<sys/socket.h>
#include<sys/epoll.h>
#include<errno.h>

#include"game.h"
#include"player.h"
#include"buffer.h"
#include"constants.h"

/* -memory management- */
player_t *add_player(player_t **head, int fd) {
  player_t *p = malloc(sizeof(*p));
  if (!p) {
    close(fd);
    return NULL;
  }

  p->fd = fd;
  memset(p->name, 0, MAX_NAME_SIZE);
  init_buffer(&p->buffer);
  p->registered = 0;
  p->disconnected = 0;
  p->next = *head;

  *head = p;
  return p;
}

void free_player(player_t *player) {
  close(player->fd);
  free(player);
}

void free_player_list(player_t *head) {
  player_t *temp;
  while (head) {
    temp = head;
    head = head->next;
    free_player(temp);
  }
}

void remove_player(player_t **head, int fd, int epoll_fd) {
  player_t *curr = *head;
  player_t *prev = NULL;

  while (curr) {
    if (curr->fd == fd) {
      if (prev) {
        prev->next = curr->next;
      } else {
        *head = curr->next;
      }

      if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL) == -1) {
        perror("epoll_ctl: delete");
      }

      close(fd);

      free_player(curr);
      return;
    }

    prev = curr;
    curr = curr->next;
  }
}

void cleanup_players(player_t **head, int epoll_fd) {
  player_t *curr = *head;
  player_t *prev = NULL;
  while (curr != NULL) {
    if (curr->registered == 1 &&
        (is_sunk(&curr->ship) == 1 || curr->disconnected == 1)) {
      player_t *remove = curr;

      fprintf(stderr, "cleaup_players: removing playing %s\n", remove->name);
      char gg_msg[32];
      snprintf(gg_msg, sizeof(gg_msg), "GG %s\n", remove->name);
      broadcast(*head, gg_msg);
      epoll_ctl(epoll_fd, EPOLL_CTL_DEL, remove->fd, NULL);
      close(remove->fd);
      free(remove);

      if (prev == NULL) {
        *head = curr->next;
        curr = *head;
      } else {
        prev->next = curr->next;
        curr = curr->next;
      }
    } else {
      prev = curr;
      curr = curr->next;
    }
  }
}

/* -utility functions- */
void setup_player(player_t *player, char *name) {
  strncpy(player->name, name, MAX_NAME_SIZE);
  player->registered = 1;
}

int players_equal(player_t *player1, player_t *player2) {
  if (player1->fd == player2->fd) {
    return 1;
  }
  return 0;
}

player_t *find_player(player_t *head, int fd) {
  for (player_t *tr = head; tr != NULL; tr = tr->next) {
    if (tr->fd == fd) {
      return tr;
    }
  }

  return NULL;
}

player_t *find_player_by_name(player_t *head, const char *name) {
  for (player_t *tr = head; tr != NULL; tr = tr->next) {
    if (strcmp(name, tr->name) == 0) {
      return tr;
    }
  }

  return NULL;
}

/* -I/O- */
void tell(player_t *player, const char *msg) {
  ssize_t n = send(player->fd, msg, strlen(msg), MSG_NOSIGNAL);

  if (n < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EPIPE) {
      player->disconnected = 1;
    } else {
      perror("send");
    }
  }
}

void broadcast(player_t *head, const char *msg) {
  while (head) {
    tell(head, msg);
    head = head->next;
  }
}
