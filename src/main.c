#include <stdio.h>
#include <stdbool.h>

#include "blueis.h"
#include "blueis_storage.h"

void print_blueis_value(BlueisValue value) {
  switch (value.kind) {
    case BLUEIS_VALUE_NUMBER:
      printf("%g", value.as.number);
      break;
    case BLUEIS_VALUE_STRING:
      printf("\"%s\"", value.as.string);
      break;
    case BLUEIS_VALUE_NIL:
      printf("NIL");
      break;
    default:
      break;
  }
}

int main() {
  BlueisTable table;
  blueis_table_init(&table);
  
  BlueisOp op = blueis_op_set(TO_BLUEIS_VALUE("foo"), TO_BLUEIS_VALUE("bar"));
  char cmd[1024];
  blueis_op_to_cmd(op, cmd, sizeof(cmd));
  blueis_execute_cmd(&table, cmd);

  char buf[1024];
  while (1) {
    printf("> ");
    if (!fgets(buf, sizeof(buf), stdin)) {
      break;
    }

    if (strcmp(buf, "exit\n") == 0) {
      break;
    }

    BlueisResult res = blueis_execute_cmd(&table, buf);

    if (res.status == BLUEIS_ERROR) {
      printf("ERROR\n");
      continue;
    }

    printf("OK ");
    print_blueis_value(res.value);
    printf("\n");
  }

  blueis_table_deinit(&table);
  return 0;
}
