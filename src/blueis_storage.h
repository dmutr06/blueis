#ifndef __BLUEIS_STORAGE_H__
#define __BLUEIS_STORAGE_H__

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define BLUEIS_GET_INIT_VALUE_FUNC(val) \
_Generic((val), \
  short: blueis_init_number, \
  int: blueis_init_number, \
  long: blueis_init_number, \
  long long: blueis_init_number, \
  unsigned: blueis_init_number, \
  unsigned short: blueis_init_number, \
  unsigned long: blueis_init_number, \
  unsigned long long: blueis_init_number, \
  float: blueis_init_number, \
  double: blueis_init_number, \
  char *: blueis_init_string, \
  void *: blueis_init_nil \
)

#define TO_BLUEIS_VALUE(val) \
  BLUEIS_GET_INIT_VALUE_FUNC(val)(val)


#define CTYPE_TO_BLUEIS_VALUE(val) \
_Generic((val), \
  bool: (union BlueisValueAs){ .boolean = val }, \
  double: (union BlueisValueAs){ .number = val }, \
  char *: (union BlueisValueAs){ .string = val }, \
)

typedef enum {
  BLUEIS_VALUE_NIL,
  BLUEIS_VALUE_NUMBER,
  BLUEIS_VALUE_STRING,
} BlueisValueKind;

typedef struct {
  BlueisValueKind kind;
  union BlueisValueAs {
    double number;
    char *string;
  } as;
} BlueisValue;

typedef enum {
  BLUEIS_PAIR_EMPTY,
  BLUEIS_PAIR_DELETED,
  BLUEIS_PAIR_OCCUPIED
} BlueisPairState;

typedef struct {
  BlueisValue key;
  BlueisValue value;
  BlueisPairState state;
} BlueisPair;

typedef struct {
  size_t capacity;
  size_t count;
  BlueisPair *pairs;
} BlueisTable;

void blueis_table_init(BlueisTable *table);
void blueis_table_deinit(BlueisTable *table);
size_t blueis_hash_value(BlueisValue value); 
bool blueis_value_compare(const BlueisValue *a, const BlueisValue *b);
void blueis_table_insert(BlueisTable *table, BlueisValue key, BlueisValue value);
BlueisValue *blueis_table_get(BlueisTable *table, BlueisValue key);
BlueisValue blueis_table_delete(BlueisTable *table, BlueisValue key);

BlueisValue blueis_init_nil(void *value);
BlueisValue blueis_init_number(double value);
BlueisValue blueis_init_string(char *value);

void blueis_free_if_string(BlueisValue *value);
void blueis_copy_if_string(BlueisValue *value);
#endif // __BLUEIS_STORAGE_H__
