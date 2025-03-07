#include "blueis_client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int blueis_client_connect(BlueisClient *client, const char *host, int port) {
  client->fd = socket(AF_INET, SOCK_STREAM, 0);
  if (client->fd < 0) {
    return -1;
  }

  client->addr.sin_family = AF_INET;
  client->addr.sin_port = htons(port);
  if (inet_pton(AF_INET, host, &client->addr.sin_addr) <= 0) {
    close(client->fd);
    return -1;
  }

  if (connect(client->fd, (struct sockaddr *)&client->addr, sizeof(client->addr)) < 0) {
    close(client->fd);
    return -1;
  }

  return 0;
}

int blueis_client_send_raw(BlueisClient *client, const char *cmd, size_t cmd_len, char *buf, size_t buf_size) {
  if (write(client->fd, cmd, cmd_len) < 0) {
    return -1;
  }

  int n = read(client->fd, buf, buf_size);
  if (n < 0) {
    return -1;
  }

  return n;
}

BlueisResult blueis_client_send_op(BlueisClient *client, BlueisOp op) {
  char cmd[1024];
  if (!blueis_op_to_cmd(op, cmd, sizeof(cmd))) {
    return (BlueisResult) { .status = BLUEIS_ERROR };
  }

  char buf[1024];

  int res = blueis_client_send_raw(client, cmd, strlen(cmd), buf, sizeof(buf));

  if (res < 0) return (BlueisResult) { .status = BLUEIS_ERROR };

  char raw_status[32];

  if (sscanf(buf, "%s", raw_status) != 1) {
    return (BlueisResult) { .status = BLUEIS_ERROR };
  }

  if (strcmp(raw_status, "ERROR") == 0) {
    return (BlueisResult) { .status = BLUEIS_ERROR };
  }

  BlueisValue value;

  if (sscanf(buf, "%*s %lf", &value.as.number) == 1) {
    value.kind = BLUEIS_VALUE_NUMBER;
  } else {
    char str[1024];
    if (sscanf(buf, "%*s \"%[^\"]\"", str) == 1) {
      value.kind = BLUEIS_VALUE_STRING;
      value.as.string = strdup(str);
    } else {
      value.kind = BLUEIS_VALUE_NIL;
    }
  }

  return (BlueisResult) { .status = BLUEIS_OK, .value = value };
}

BlueisResult blueis_client_get(BlueisClient *client, BlueisValue key) {
  BlueisOp op = blueis_op_get(key);
  return blueis_client_send_op(client, op);
}

BlueisResult blueis_client_set(BlueisClient *client, BlueisValue key, BlueisValue value) {
  BlueisOp op = blueis_op_set(key, value);
  return blueis_client_send_op(client, op);
}

BlueisResult blueis_client_delete(BlueisClient *client, BlueisValue key) {
  BlueisOp op = blueis_op_delete(key);
  return blueis_client_send_op(client, op);
}

void blueis_client_close(BlueisClient *client) {
  close(client->fd);
}
