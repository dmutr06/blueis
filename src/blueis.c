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

typedef struct {
  BlueisLexer lexer;
  BlueisToken cur_token;
} BlueisParser;

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


// TODO: implement a proper lexer

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

  if (isdigit(c) || c == '-') {
    token.type = TOKEN_NUMBER;
    token.length += 1;
    blueis_lexer_advance(lexer);
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
  op->kind = BLUEIS_INVALID_OP;
  switch (parser->cur_token.type) {
    case TOKEN_GET:
    case TOKEN_DELETE: {
      BlueisToken key_token = blueis_scan_token(&parser->lexer);
      if (key_token.type == TOKEN_STRING || key_token.type == TOKEN_NUMBER) {
        op->kind = (parser->cur_token.type == TOKEN_GET) ? BLUEIS_GET_OP : BLUEIS_DELETE_OP;
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

BlueisResult blueis_execute_op(BlueisTable *table, BlueisOp op) {
  switch (op.kind) {
    case BLUEIS_GET_OP: {
      BlueisValue val = blueis_table_get(table, op.get.key);
      return (BlueisResult){ .status = BLUEIS_OK, .value = val };
    }
    case BLUEIS_SET_OP: {
      BlueisValue val = blueis_table_insert(table, op.set.key, op.set.value);
      return (BlueisResult){ .status = BLUEIS_OK, .value = val };
    }
    case BLUEIS_DELETE_OP: {
      BlueisValue val = blueis_table_delete(table, op.delete.key);
      return (BlueisResult){ .status = BLUEIS_OK, .value = val };
    }
    default:
      return (BlueisResult) {
        .status = BLUEIS_ERROR,
        .value = TO_BLUEIS_VALUE(NULL)
      };
  }
}

BlueisOp blueis_op_from_cmd(const char *cmd) {
  BlueisOp op;
  BlueisParser parser;

  blueis_parser_init(&parser, cmd);
  blueis_parse_op(&parser, &op);

  return op;
}

BlueisResult blueis_execute_cmd(BlueisTable *table, const char *cmd) {
  BlueisOp op = blueis_op_from_cmd(cmd);
  return blueis_execute_op(table, op);
}
