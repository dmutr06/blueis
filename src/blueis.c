#include "blueis.h"
#include "blueis_storage.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <stddef.h>

typedef enum {
  TOKEN_GET,
  TOKEN_SET,
  TOKEN_DELETE,
  TOKEN_STRING,
  TOKEN_NUMBER,
  TOKEN_ERROR,
  TOKEN_EOF
} BlueisTokenType;

typedef struct {
  BlueisTokenType type;
  char const *start;
  size_t length;
} BlueisToken;


typedef struct {
  char const *src;
  size_t length;
  size_t cur;
} BlueisLexer;

void blueis_lexer_init(BlueisLexer *lexer, const char *src) {
  lexer->src = src;
  lexer->length = strlen(src);
  lexer->cur = 0;
}

char blueis_lexer_peek(BlueisLexer *lexer) {
  return lexer->cur < lexer->length ? lexer->src[lexer->cur] : '\0';
}

char blueis_lexer_advance(BlueisLexer *lexer) {
  return lexer->cur < lexer->length ? lexer->src[lexer->cur++] : '\0';
}

void blueis_lexer_skip_whitespace(BlueisLexer *lexer) {
  while (isspace(blueis_lexer_peek(lexer))) {
    blueis_lexer_advance(lexer);
  }
}

BlueisToken blueis_scan_token(BlueisLexer *lexer) {
  blueis_lexer_skip_whitespace(lexer);
  
  BlueisToken token;
  token.type = TOKEN_ERROR;
  token.start = lexer->src + lexer->cur;
  token.length = 0;
  
  char c = blueis_lexer_peek(lexer);
  if (c == '"') {
    token.type = TOKEN_STRING;
    token.start += 1;
    blueis_lexer_advance(lexer);
    while (blueis_lexer_peek(lexer) != '"' && blueis_lexer_peek(lexer) != '\0') {
      blueis_lexer_advance(lexer);
      token.length += 1;
    }
    if (blueis_lexer_peek(lexer) == '\0') {
      token.type = TOKEN_ERROR;
    } else {
      blueis_lexer_advance(lexer);
    }

    return token;
  }

  if (isdigit(c)) {
    token.type = TOKEN_NUMBER;
    bool has_dot = false;
    while (isdigit(blueis_lexer_peek(lexer)) || blueis_lexer_peek(lexer) == '.') {
      if (blueis_lexer_peek(lexer) == '.') {
        if (has_dot) {
          token.type = TOKEN_ERROR;
          break;
        }
        has_dot = true;
      }
      blueis_lexer_advance(lexer);
      token.length += 1;
    }

    return token;
  }

  if (c == '\0') {
    token.type = TOKEN_EOF;
    return token;
  }

  while (!isspace(blueis_lexer_peek(lexer)) && blueis_lexer_peek(lexer) != '\0') {
    blueis_lexer_advance(lexer);
    token.length += 1;
  }

  if (strncmp(token.start, "GET", token.length) == 0) {
    token.type = TOKEN_GET;
    return token;
  }

  if (strncmp(token.start, "SET", token.length) == 0) {
    token.type = TOKEN_SET;
    return token;
  }

  if (strncmp(token.start, "DELETE", token.length) == 0) {
    token.type = TOKEN_DELETE;
    return token;
  }
 
  return token;
}

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

typedef struct {
  BlueisLexer lexer;
  BlueisToken cur_token;
} BlueisParser;

void blueis_parser_init(BlueisParser *parser, const char *src) {
  blueis_lexer_init(&parser->lexer, src);
  parser->cur_token = blueis_scan_token(&parser->lexer);
}

BlueisValue blueis_value_from_token(BlueisToken token) {
  if (token.type == TOKEN_NUMBER) {
    double number = 0;
    char buf[64] = {0};
    strncpy(buf, token.start, token.length > 63 ? 63 : token.length);
    sscanf(buf, "%lf", &number);
    return TO_BLUEIS_VALUE(number);
  } else if (token.type == TOKEN_STRING) {
    char *buf = strndup(token.start, token.length);
    return TO_BLUEIS_VALUE(buf);
  }

  return TO_BLUEIS_VALUE(NULL);
}

void blueis_parse_op(BlueisParser *parser, BlueisOp *op) {
  switch (parser->cur_token.type) {
    case TOKEN_GET:
    case TOKEN_DELETE: {
      BlueisToken key_token = blueis_scan_token(&parser->lexer);
      if (key_token.type == TOKEN_STRING || key_token.type == TOKEN_NUMBER) {
        op->kind = parser->cur_token.type == TOKEN_GET ? BLUEIS_GET_OP : BLUEIS_DELETE_OP;
        op->get.key = blueis_value_from_token(key_token);
      }
      break;
    }
    case TOKEN_SET: {
      BlueisToken key_token = blueis_scan_token(&parser->lexer);
      BlueisToken value_token = blueis_scan_token(&parser->lexer);

      if ((key_token.type == TOKEN_STRING || key_token.type == TOKEN_NUMBER) &&
          (value_token.type == TOKEN_STRING || value_token.type == TOKEN_NUMBER)) {
        op->kind = BLUEIS_SET_OP;
        op->set.key = blueis_value_from_token(key_token);
        op->set.value = blueis_value_from_token(value_token);
      }

      break;
    }
    default:
      op->kind = BLUEIS_INVALID_OP;
      break;
  }

  parser->cur_token = blueis_scan_token(&parser->lexer);
}

BlueisValue blueis_execute_op(BlueisTable *table, BlueisOp op) {
  switch (op.kind) {
    case BLUEIS_GET_OP: {
      BlueisValue *result = blueis_table_get(table, op.get.key);
      if (result)
        return *result;
      return TO_BLUEIS_VALUE(NULL);
    }
    case BLUEIS_SET_OP:
      blueis_table_insert(table, op.set.key, op.set.value);
      return op.set.value;
    case BLUEIS_DELETE_OP: {
      return blueis_table_delete(table, op.delete.key);
    }
    default:
      return TO_BLUEIS_VALUE(NULL);
  }
}

void blueis_free_op(BlueisOp *op) {
  switch (op->kind) {
    case BLUEIS_GET_OP:
    case BLUEIS_DELETE_OP:
    case BLUEIS_SET_OP:
      blueis_free_if_string(&op->get.key);
      break;
    default:
      break;
  }
}


// TODO: return status with value
BlueisValue blueis_execute_cmd(BlueisTable *table, const char *cmd) {
  BlueisOp op;
  BlueisParser parser;

  blueis_parser_init(&parser, cmd);
  blueis_parse_op(&parser, &op);

  BlueisValue res = blueis_execute_op(table, op);
  blueis_copy_if_string(&res);

  blueis_free_op(&op);

  return res;
}
