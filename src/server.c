#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>

#include "blueis.h"

#define MAX_EVENTS 10

int str_append_blueis_value(char *buf, size_t buf_size, BlueisValue value) {
  size_t len = strlen(buf);

  switch (value.kind) {
    case BLUEIS_VALUE_NUMBER:
      return snprintf(buf + len, buf_size - len, "%g", value.as.number);
    case BLUEIS_VALUE_STRING:
      return snprintf(buf + len, buf_size - len, "\"%s\"", value.as.string);
    case BLUEIS_VALUE_NIL:
      return snprintf(buf + len, buf_size - len, "NIL");
  }

  return 0;
}

int setnonblocking(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags < 0) {
    return -1;
  }

  if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
    return -1;
  }

  return 0;
}

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <port>\n", argv[0]);
    return 1;
  }

  int port = atoi(argv[1]);

  BlueisTable table;
  blueis_table_init(&table);

  int sfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sfd < 0) {
    perror("socket");
    return 1;
  }
  
  if (setnonblocking(sfd) < 0) {
    perror("setnonblocking");
    return 1;
  }

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(sfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("bind");
    return 1;
  }

  if (listen(sfd, 69) < 0) {
    perror("listen");
    return 1;
  }

  int efd = epoll_create1(0);
  if (efd == -1) {
    perror("epoll_create1");
    return 1;
  }

  struct epoll_event event;
  event.events = EPOLLIN;
  event.data.fd = sfd;

  if (epoll_ctl(efd, EPOLL_CTL_ADD, sfd, &event) < 0) {
    perror("epoll_ctl(sfd)");
    return 1;
  }

  struct epoll_event events[MAX_EVENTS];
  
  for (;;) {
    int nfds = epoll_wait(efd, events, MAX_EVENTS, -1);
    if (nfds < 0) {
      perror("epoll_wait");
      return 1;
    }

    for (int i = 0; i < nfds; i++) {
      if (events[i].data.fd == sfd) {
        int cfd = accept(sfd, NULL, NULL);
        if (cfd < 0) {
          perror("accept");
          return 1;
        }

        if (setnonblocking(cfd) < 0) {
          perror("setnonblocking");
          return 1;
        }

        event.events = EPOLLIN | EPOLLET;
        event.data.fd = cfd;

        if (epoll_ctl(efd, EPOLL_CTL_ADD, cfd, &event) < 0) {
          perror("epoll_ctl(cfd)");
          return 1;
        }
      } else {
        int cfd = events[i].data.fd;
        char buf[1024] = {0};
        int n = read(cfd, buf, sizeof(buf));
        if (n < 0) {
          perror("read");
          return 1;
        }

        if (n == 0) {
          close(cfd);
          continue;
        }

        buf[n] = '\0';
        BlueisResult res = blueis_execute_cmd(&table, buf);

        if (res.status == BLUEIS_ERROR) {
          write(cfd, "ERROR\n", 6);
          continue;
        }

        char out[1024] = {0};
        int len = sprintf(out, "OK ");
        len += str_append_blueis_value(out, sizeof(out) - len, res.value);

        len += snprintf(out + len, sizeof(out) - len, "\n");
        write(cfd, out, len);
      }
    }
  }

  return 0;
}
