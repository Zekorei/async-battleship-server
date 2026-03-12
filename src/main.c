#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<arpa/inet.h>

#include"server.h"

int main(int argc, char **argv) {
  int sfd;
  int port;

  if (argc != 2) {
    fprintf(stderr, "One positional argment expected; none given\n");
    exit(1);
  }

  port = atoi(argv[1]);
  if (port <= 1024 || port > 65535) {
    fprintf(stderr, "main: invalid port: %d\n", port);
    exit(1);
  }

  sfd = s_socket_setup(port);
  fprintf(stderr, "[log] main: listening on port %d...\n", port);

  s_mainloop(sfd);

  close(sfd);
  return 0;
}
