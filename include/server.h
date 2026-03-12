#ifndef SERVER_H_
#define SERVER_H_

#include"player.h"
#include"constants.h"

int s_socket_setup(int port);
int s_epoll_setup(int sfd);
void s_mainloop(int sfd);
/* Returns the head of the list of players */
void s_handle_connection(int epoll_fd, int sfd, player_t **player_list);
void s_handle_client(int epoll_fd, int cfd, player_t **player_list);

int sock_setunblocking(int cfd);

#endif
