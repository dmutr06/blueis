#include "blueis_auth.h"
#include "blueis_storage.h"

#include <string.h>
#include <stdbool.h>
#include <stdio.h>

bool blueis_parse_auth_cmd(const char *cmd, char *password, size_t password_len) {
  if (strncmp(cmd, "AUTH ", 5) != 0) {
    return false;
  }

  size_t len = strlen(cmd + 5);
  if (len >= password_len) {
    return false;
  }

  sscanf(cmd + 5, "%s", password);

  return true;
}

bool blueis_is_authed(BlueisTable *auth_table, int fd) {
  return blueis_value_compare(
    blueis_table_get(auth_table, TO_BLUEIS_VALUE(fd)),
    TO_BLUEIS_VALUE(1)
  );
}

void blueis_auth(BlueisTable *auth_table, int fd) {
  blueis_table_insert(auth_table, TO_BLUEIS_VALUE(fd), TO_BLUEIS_VALUE(1));
}

void blueis_unauth(BlueisTable *auth_table, int fd) {
  blueis_table_delete(auth_table, TO_BLUEIS_VALUE(fd));
}
