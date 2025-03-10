#ifndef __BLUEIS_H__
#define __BLUEIS_H__

#include <stdbool.h>
#include "blueis_storage.h"

typedef enum {
  BLUEIS_GET_OP,
  BLUEIS_SET_OP,
  BLUEIS_DELETE_OP,
  BLUEIS_INVALID_OP
} BlueisOpKind;

typedef struct {
  BlueisOpKind kind;
  union {
    struct {
      BlueisValue key;
    } get;
    struct {
      BlueisValue key;
      BlueisValue value;
    } set;
    struct {
      BlueisValue key;
    } delete;
  };
} BlueisOp;

typedef enum {
  BLUEIS_OK,
  BLUEIS_ERROR
} BlueisStatus;

typedef struct {
  BlueisStatus status;
  const BlueisValue value;
} BlueisResult;

int str_append_blueis_value(char *buf, size_t buf_size, BlueisValue value);

BlueisOp blueis_op_from_cmd(const char *cmd);
BlueisResult blueis_execute_op(BlueisTable *table, BlueisOp op);
BlueisResult blueis_execute_cmd(BlueisTable *table, const char *cmd);

bool blueis_op_to_cmd(BlueisOp op, char *cmd, size_t len);

BlueisOp blueis_op_get(BlueisValue key);
BlueisOp blueis_op_set(BlueisValue key, BlueisValue value);
BlueisOp blueis_op_delete(BlueisValue key);

#endif // __BLUEIS_H__
