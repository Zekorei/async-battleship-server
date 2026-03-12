#ifndef PLAYER_H_
#define PLAYER_H_

#include"game.h"
#include"buffer.h"
#include"constants.h"

typedef struct player_node {
  int			fd;
  char			name[MAX_NAME_SIZE];
  buffer_t		buffer;
  ship_t		ship;
  int			registered;
  int			disconnected;
  struct player_node	*next;
} player_t;

/* -memory management- */
player_t *	add_player(player_t **head, int fd);
void		free_player(player_t *player);
void		free_player_list(player_t *head);
void		remove_player(player_t **head, int fd, int epoll_fd);
void		cleanup_players(player_t **head, int epoll_fd);

/* -utility functions- */
void		setup_player(player_t *player, char *name);
int		players_equal(player_t *player1, player_t *player2);
player_t * 	find_player(player_t *head, int fd);
player_t *	find_player_by_name(player_t *head, const char *name);

/* -I/O- */
void		tell(player_t *player, const char *msg);
void		broadcast(player_t *head, const char *msg);

#endif
