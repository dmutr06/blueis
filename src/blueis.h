#ifndef __BLUEIS_H__
#define __BLUEIS_H__

#include <stdbool.h>
#include "blueis_storage.h"

BlueisValue blueis_execute_cmd(BlueisTable *table, const char *cmd);

#endif // __BLUEIS_H__
