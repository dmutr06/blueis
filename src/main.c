#include <stdio.h>
#include <stdbool.h>

#include "blueis_storage.h"

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

void add_to_table(BlueisTable *table) {
  float a = 12;
  BlueisValue key = TO_BLUEIS_VALUE("Hello");
  BlueisValue value = TO_BLUEIS_VALUE(a);

  blueis_table_insert(table, key, value);
}

int main() {
  BlueisTable table;
  blueis_table_init(&table);

  add_to_table(&table);

  BlueisValue deleted = blueis_table_delete(&table, TO_BLUEIS_VALUE("Hello"));

  printf("Deleted ");
  print_blueis_value(deleted);
  printf("\n");;
  blueis_free_if_string(&deleted);

  blueis_table_insert(&table, TO_BLUEIS_VALUE("Hello"), TO_BLUEIS_VALUE("World"));

  BlueisValue *result = blueis_table_get(&table, TO_BLUEIS_VALUE("Hello"));

  if (result) {
    printf("Found ");
    print_blueis_value(*result);
    printf("\n");;
  } else {
    printf("Nil\n");
  }

  blueis_table_deinit(&table);
  return 0;
}
