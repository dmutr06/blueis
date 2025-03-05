#include "blueis_storage.h"

#define BLUEIS_TABLE_INITIAL_CAPACITY 64
#define BLUEIS_TABLE_LOAD_FACTOR 0.5

void blueis_copy_if_string(BlueisValue *value) {
  if (value->kind == BLUEIS_VALUE_STRING) {
    value->as.string = strdup(value->as.string);
  }
}

void blueis_free_if_string(BlueisValue *value) {
  if (value->kind == BLUEIS_VALUE_STRING) {
    free(value->as.string);
  }
}

void blueis_table_init(BlueisTable *table) {
  table->capacity = BLUEIS_TABLE_INITIAL_CAPACITY;
  table->count = 0;
  table->pairs = malloc(sizeof(BlueisPair) * table->capacity);
}

void blueis_table_deinit(BlueisTable *table) {
  for (size_t i = 0; i < table->capacity; i++) {
    if (table->pairs[i].state == BLUEIS_PAIR_OCCUPIED) {
      blueis_free_if_string(&table->pairs[i].key);
      blueis_free_if_string(&table->pairs[i].value);
    } else if (table->pairs[i].state == BLUEIS_PAIR_DELETED) {
      blueis_free_if_string(&table->pairs[i].key);
    }
  }
  free(table->pairs);
}

size_t hash(BlueisValue value) {
  switch (value.kind) {
    case BLUEIS_VALUE_NUMBER:
      return (size_t) value.as.number;
    case BLUEIS_VALUE_STRING: {
      size_t payload = 5381;
      for (size_t i = 0; i < strlen(value.as.string); i++) {
        payload = ((payload << 5) + payload) + value.as.string[i];
      }
      return payload;
    }
    default:
      return 0;
  }
}

bool blueis_value_compare(const BlueisValue *a, const BlueisValue *b) {
  if (a->kind != b->kind) return false;

  switch (a->kind) {
    case BLUEIS_VALUE_NUMBER:
      return a->as.number == b->as.number;
    case BLUEIS_VALUE_STRING:
      return strcmp(a->as.string, b->as.string) == 0;
    case BLUEIS_VALUE_NIL:
      return true;
    default:
      return false;
  }
}


BlueisValue blueis_table_internal_insert(BlueisTable *table, BlueisValue key, BlueisValue value) {
  size_t index = hash(key);

  for (size_t i = 0; i < table->capacity; i++) {
    size_t probe = (index + i) & (table->capacity - 1);
    BlueisPair *cur = &table->pairs[probe];
    if (cur->state == BLUEIS_PAIR_EMPTY || cur->state == BLUEIS_PAIR_DELETED) {
      if (cur->state == BLUEIS_PAIR_DELETED) {
        blueis_free_if_string(&cur->key);
        blueis_free_if_string(&cur->value);
      }
      cur->state = BLUEIS_PAIR_OCCUPIED;
      cur->key = key;
      blueis_copy_if_string(&cur->key);
      cur->value = value;
      blueis_copy_if_string(&cur->value);
      table->count += 1;
      return cur->value;
    }
    if (cur->state == BLUEIS_PAIR_OCCUPIED && blueis_value_compare(&cur->key, &key)) {
      cur->value = value;
      return cur->value;
    }
  }

  return TO_BLUEIS_VALUE(NULL);
}

void blueis_table_resize(BlueisTable *table, size_t new_capacity) {
  BlueisPair *old_pairs = table->pairs;
  size_t old_capacity = table->capacity;

  table->capacity = new_capacity;
  table->count = 0;
  table->pairs = malloc(sizeof(BlueisPair) * table->capacity);
  memset(table->pairs, 0, sizeof(BlueisPair) * table->capacity);

  for (size_t i = 0; i < old_capacity; i++) {
    if (old_pairs[i].state == BLUEIS_PAIR_OCCUPIED) {
      blueis_table_internal_insert(table, old_pairs[i].key, old_pairs[i].value);
    }
  }

  free(old_pairs);
}

BlueisValue blueis_table_insert(BlueisTable *table, BlueisValue key, BlueisValue value) {
  if (table->count >= table->capacity * BLUEIS_TABLE_LOAD_FACTOR)
    blueis_table_resize(table, table->capacity * 2);
  
  return blueis_table_internal_insert(table, key, value);
}

BlueisValue blueis_table_get(BlueisTable *table, BlueisValue key) {
  size_t index = hash(key);

  for (size_t i = 0; i < table->capacity; i++) {
    size_t probe = (index + i) & (table->capacity - 1);
    if (table->pairs[probe].state == BLUEIS_PAIR_EMPTY) continue;
    if (table->pairs[probe].state == BLUEIS_PAIR_OCCUPIED) {
      if (table->pairs[probe].key.kind != key.kind) continue;
      if (blueis_value_compare(&table->pairs[probe].key, &key)) {
        return table->pairs[probe].value;
      }
    }
    if (
      table->pairs[probe].state == BLUEIS_PAIR_DELETED &&
      blueis_value_compare(&table->pairs[probe].key, &key)
    ) break;
  }

  return TO_BLUEIS_VALUE(NULL);
}

BlueisValue blueis_table_delete(BlueisTable *table, BlueisValue key) {
  size_t index = hash(key);

  for (size_t i = 0; i < table->capacity; i++) {
    size_t probe = (index + i) & (table->capacity - 1);
    BlueisPair *cur = &table->pairs[probe];
    if (cur->state == BLUEIS_PAIR_EMPTY) continue;
    if (cur->state == BLUEIS_PAIR_OCCUPIED) {
      if (cur->key.kind != key.kind) continue;
      if (blueis_value_compare(&cur->key, &key)) {
        cur->state = BLUEIS_PAIR_DELETED;
        table->count -= 1;
        return cur->value;
      }
    }
  }

  return blueis_init_nil(NULL);
}

BlueisValue blueis_init_nil(void *value) {
  return (BlueisValue) {
    .kind = BLUEIS_VALUE_NIL,
  };
}

BlueisValue blueis_init_number(double value) {
  return (BlueisValue) {
    .kind = BLUEIS_VALUE_NUMBER,
    .as = { .number = value }
  };
}

BlueisValue blueis_init_string(char *value) {
  return (BlueisValue) {
    .kind = BLUEIS_VALUE_STRING,
    .as = { .string = value }
  };
}
