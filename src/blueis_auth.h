#ifndef __BLUEIS_AUTH_H__
#define __BLUEIS_AUTH_H__

#include <stdbool.h>

#include "blueis_storage.h"

bool blueis_parse_auth_cmd(const char *cmd, char *password, size_t password_len);

bool blueis_is_authed(BlueisTable *auth_table, int fd);
void blueis_auth(BlueisTable *auth_table, int fd);
void blueis_unauth(BlueisTable *auth_table, int fd);


#endif // __BLUEIS_AUTH_H__
