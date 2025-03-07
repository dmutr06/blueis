#ifndef __BLUEIS_CLIENT_H__
#define __BLUEIS_CLIENT_H__

#include "blueis.h"
#include <netinet/in.h>

typedef struct {
  int fd;
  struct sockaddr_in addr;
} BlueisClient;

int blueis_client_connect(BlueisClient *client, const char *host, int port);
void blueis_client_close(BlueisClient *client);

int blueis_client_send_raw(BlueisClient *client, const char *cmd, size_t cmd_len, char *buf, size_t buf_size);
BlueisResult blueis_client_send_op(BlueisClient *client, BlueisOp op);

BlueisResult blueis_client_get(BlueisClient *client, BlueisValue key);
BlueisResult blueis_client_set(BlueisClient *client, BlueisValue key, BlueisValue value);
BlueisResult blueis_client_delete(BlueisClient *client, BlueisValue key);

BlueisResult blueis_client_auth(BlueisClient *client, const char *password);

#endif // __BLUEIS_CLIENT_H__
