#include <stdio.h>
#include <stdbool.h>

#include "blueis.h"

void add_to_table(BlueisTable *table) {
  BlueisValue key = TO_BLUEIS_VALUE("Hello");
  BlueisValue value = TO_BLUEIS_VALUE("World");

  blueis_table_insert(table, key, value);
}

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

  add_to_table(&table);

  BlueisValue get_res = blueis_execute_cmd(&table, "GET \"Hello\"");
  BlueisValue set_res = blueis_execute_cmd(&table, "SET \"Hello\" \"it works\"");
  
  print_blueis_value(get_res);
  printf("\n");
  print_blueis_value(set_res);
  printf("\n");

  blueis_table_deinit(&table);
  return 0;
}
