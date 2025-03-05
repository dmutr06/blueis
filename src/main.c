#include <stdio.h>
#include <stdbool.h>

#include "blueis.h"

void print_blueis_value(BlueisValue value) {
  switch (value.kind) {
    case BLUEIS_VALUE_NUMBER:
      printf("%g", value.as.number);
      break;
    case BLUEIS_VALUE_STRING:
      printf("%s", value.as.string);
      break;
    case BLUEIS_VALUE_NIL:
      printf("Nil");
      break;
    default:
      break;
  }
}

int main() {
  BlueisTable table;
  blueis_table_init(&table);

  char buf[1024];
  while (1) {
    printf("> ");
    if (!fgets(buf, sizeof(buf), stdin)) {
      break;
    }

    if (strcmp(buf, "exit\n") == 0) {
      break;
    }

    BlueisValue res = blueis_execute_cmd(&table, buf);
    print_blueis_value(res);
    printf("\n");
  }

  blueis_table_deinit(&table);
  return 0;
}
