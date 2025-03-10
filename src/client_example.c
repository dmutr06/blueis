#include "blueis_client.h"
#include "blueis_storage.h"
#include <stdio.h>

int main(int argc, char **argv) {
  if (argc < 4) {
    fprintf(stderr, "Usage: %s <host> <port> <password>\n", argv[0]);
    return 1;
  }

  const char *host = argv[1];
  int port = atoi(argv[2]);
  const char *password = argv[3];

  BlueisClient client;

  if (blueis_client_connect(&client, host, port) < 0) {
    fprintf(stderr, "Failed to connect to %s:%d\n", host, port);
    return 1;
  }

  BlueisResult auth_res = blueis_client_auth(&client, password);

  if (auth_res.status == BLUEIS_ERROR) {
    fprintf(stderr, "Failed to authenticate\n");
    return 1;
  }

  BlueisResult res = blueis_client_get(&client, TO_BLUEIS_VALUE("FOO"));

  if (res.status == BLUEIS_ERROR) {
    fprintf(stderr, "Failed to send GET request\n");
    return 1;
  }

  BlueisValue val = res.value;

  switch (val.kind) {
    case BLUEIS_VALUE_STRING:
      printf("Got string: %s\n", val.as.string);
      break;
    case BLUEIS_VALUE_NUMBER:
      printf("Got num: %lf\n", val.as.number);
      break;
    case BLUEIS_VALUE_NIL:
      printf("Got NIL\n");
      break;
    default:
      printf("Got unknown value\n");
  }

  blueis_free_if_string(&val);

  blueis_client_close(&client);

  return 0;
}
