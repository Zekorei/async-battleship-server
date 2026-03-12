#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<errno.h>

#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<sys/epoll.h>
#include<fcntl.h>

#include"player.h"
#include"server.h"
#include"buffer.h"
#include"parser.h"
#include"constants.h"

int s_socket_setup(int port) {
  int sfd;
  struct sockaddr_in addr;

  if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    exit(1);
  }

  // socket options
  memset(&addr, 0, sizeof(addr));
  addr.sin_family 	= AF_INET;
  addr.sin_port  	= htons(port);
  addr.sin_addr.s_addr 	= INADDR_ANY;

  if (bind(sfd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
    perror("bind");
    close(sfd);
    exit(1);
  }

  if (listen(sfd, MAX_QUEUE_SIZE) == -1) {
    perror("listen");
    close(sfd);
    exit(1);
  }

  return sfd;
}

int s_epoll_setup(int sfd) {
  struct epoll_event event;
  int epoll_fd = epoll_create1(0);
  if (epoll_fd == -1) {
    perror("epoll_create1");
    exit(1);
  }

  // set epoll sfd
  event.events 	= EPOLLIN;
  event.data.fd = sfd;

  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sfd, &event) == -1) {
    perror("epoll_ctl: listen_socket");
    exit(1);
  }
  return epoll_fd;
}

void s_mainloop(int sfd) {
  int			epoll_fd = s_epoll_setup(sfd);
  player_t		*player_list = NULL;
  struct epoll_event	events[MAX_EVENTS];

  while (1) {
    int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
    if (n < 0) {
      perror("epoll_wait");
      break;
    }

    for (int i = 0; i < n; i++) {
      int fd = events[i].data.fd;

      if (fd == sfd) {
        s_handle_connection(epoll_fd, sfd, &player_list);
      } else {
        s_handle_client(epoll_fd, fd, &player_list);
      }
    }
  }

  close(epoll_fd);
  free_player_list(player_list);
}

void s_handle_connection(int epoll_fd, int sfd, player_t **player_list) {
  struct sockaddr_in	client_addr;
  socklen_t 		addrlen = sizeof(client_addr);

  // accept new client
  int cfd = accept(sfd, (struct sockaddr *)&client_addr, &addrlen);
  if (cfd < 0) {
    perror("accept");
    return;
  }

  if (sock_setunblocking(cfd) < 0) {
    close(cfd);
    return;
  }

  // register fd with epoll
  struct epoll_event event = {
    .events = EPOLLIN | EPOLLET,
    .data.fd = cfd
  };

  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, cfd, &event) == -1) {
    perror("epoll_ctl: add client");
    close(cfd);
    return;
  }

  // add player to list
  if (!add_player(player_list, cfd)) {
    fprintf(stderr, "Failed to add player for fd %d\n", cfd);
    close(cfd);
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, cfd, NULL);
    return;
  }

  fprintf(stderr, "New connection from %s:%d (fd=%d)\n",
    inet_ntoa(client_addr.sin_addr),
    ntohs(client_addr.sin_port),
    cfd
  );
}

void s_handle_client(int epoll_fd, int cfd, player_t **player_list) {
  player_t *player = find_player(*player_list, cfd);
  if (!player) {
    fprintf(stderr, "No player found for fd=%d\n", cfd);
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, cfd, NULL);
    close(cfd);
    return;
  }
  if (player->disconnected == 1) {
    if (player->registered == 1) {
      char gg_msg[30];
      snprintf(gg_msg, sizeof(gg_msg), "GG %s\n", player->name);
      broadcast(*player_list, gg_msg);
    }
    remove_player(player_list, cfd, epoll_fd);
    return;
  }

  char read_buf[MAX_MSG_LEN];
  ssize_t bytes_read = read(cfd, read_buf, sizeof(read_buf));

  if (bytes_read <= 0) {
    if (bytes_read == 0) {
      fprintf(stderr, "Client fd=%d disconnected\n", cfd);
    } else {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        return;
      }
      perror("read");
    }

    if (player->registered == 1) {
      char gg_msg[30];
      snprintf(gg_msg, sizeof(gg_msg), "GG %s\n", player->name);
      broadcast(*player_list, gg_msg);
    }
    remove_player(player_list, cfd, epoll_fd);
    return;
  }

  if (append_b(&player->buffer, read_buf, (size_t)bytes_read) > 0 ||
      (player->buffer.size == MAX_MSG_LEN && is_ready(&player->buffer) < 0)) {
    fprintf(stderr, "Client %d buffer overflow. Disconnecting...\n", cfd);
    remove_player(player_list, cfd, epoll_fd);
    return;
  }

  char *cmd;
  if ((cmd = pop_b(&player->buffer)) != NULL) {
    handle_command(player, cmd, *player_list, epoll_fd);
    free(cmd);
  }
}

int sock_setunblocking(int cfd) {
  int flags = fcntl(cfd, F_GETFL, 0);
  if (flags == -1) {
    perror("fcntl get");
    return -1;
  }

  if (fcntl(cfd, F_SETFL, flags | O_NONBLOCK) == -1) {
    perror("fcntl set");
    return -1;
  }
  return 0;
}
