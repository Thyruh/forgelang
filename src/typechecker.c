#include "./typechecker.h"
#include "./common/tokenhandlers.h"

static inline bool is_int_type(TokenType t) {
  return (t >= i8_ && t <= u64_) || t == usize;
}

static inline bool is_float_type(TokenType t) {
  return t >= f32_ && t <= f64_;
}

static inline bool is_numeric(TokenType t) {
  return is_int_type(t) || is_float_type(t);
}

static inline bool is_string_type(TokenType t) {
  return t == string;
}

static inline bool is_char_type(TokenType t) {
  return t == char_ || t == uchar_;
}

static inline void checker_line_error(Tokens* tokens, TokenPos pos, char* err_msg) {
  printf(ANSI_COLOR_RED "[typechecker]: %s at line %zu in:\n" ANSI_COLOR_RESET,
      err_msg, pos.line);

  size_t i = 0;
  while (i < tokens->size && tokens->items[i].pos.line < pos.line) {
    i++;
  }

  printf("\t" ANSI_COLOR_YELLOW);

  while (i < tokens->size && tokens->items[i].pos.line == pos.line) {
    const char* s = token_repr(tokens->items[i]);

    if (tokens->items[i].type == char_lit) {
      printf("'%s' ", s);
    } else if (tokens->items[i].type == string_lit) {
      printf("\"%s\" ", s);
    } else if (s[0] != ';') {
      printf("%s ", s);
    } else {
      printf(";");
    }
    i++;
  }

  puts(ANSI_COLOR_RESET);
}

static inline size_t ident_exists(SymbolTable* table, Token token) {
  for (size_t i = 0; i < table->size; i++) {
    if (!strcmp(table->items[i].ident, token.value)) {
      return i;
    }
  }
  return SIZE_MAX;
}

static inline TokenType term_type(SymbolTable* table, NodeTerm* term) {
  switch (term->type) {
    case TERM_INT_LIT: return i32_;
    case TERM_STRING_LIT: return string;
    case TERM_CHAR_LIT: return char_;
    case TERM_IDENT: 
      {
        size_t i = ident_exists(table, term->value.ident->ident);
        if (i == SIZE_MAX) return TERMINATE;
        return table->items[i].type;
      }
  }
  return TERMINATE;
}

static inline void check_assignment_compat(
    ErrorStack* errors,
    Token lhs_token,
    TokenType lhs_type,
    TokenType rhs_type,
    bool check_mut,
    bool is_mut
    ) {
  if (check_mut && !is_mut) {
    Error e = {
      .code = ERROR_TYPE_CONST_REASSIGNMENT,
      .trace = {0},
      .pos = lhs_token.pos
    };
    snprintf(e.message, sizeof(e.message), "Attempting to change an immutable variable");
    da_append(errors, e);
  }

  if (is_numeric(lhs_type) && is_numeric(rhs_type)) return;
  if (is_string_type(lhs_type) && is_string_type(rhs_type)) return;
  if (is_char_type(lhs_type) && is_char_type(rhs_type)) return;

  ErrorCode code;
  const char* msg;

  if (is_numeric(lhs_type)) {
    code = ERROR_TYPE_EXPECTED_INT;
    msg = "Expected integer type";
  } else if (is_string_type(lhs_type)) {
    code = ERROR_TYPE_EXPECTED_STRING;
    msg = "Expected string type";
  } else if (is_char_type(lhs_type)) {
    code = ERROR_TYPE_EXPECTED_CHAR;
    msg = "Expected char type";
  } else {
    code = ERROR_TYPE_TYPE_MISMATCH;
    msg = "Type mismatch";
  }

  Error e = {
    .code = code,
    .trace = {0},
    .pos = lhs_token.pos
  };

  snprintf(e.message, sizeof(e.message), "%s", msg);
  da_append(errors, e);
}

static inline bool check_type_compat(TokenType a, TokenType b) {
  return (
      (is_numeric(a) && is_numeric(b)) ||
      (is_string_type(a) && is_string_type(b)) ||
      (is_char_type(a) && is_char_type(b))
      );
}

static inline TokenType typecheck_expr(
    SymbolTable* table,
    NodeExpr* expr,
    ErrorStack* errors
    ) {
  if (expr->type == EXPR_TERM) {
    TokenType tt = term_type(table, expr->value.term);

    if (tt == TERMINATE) {
      Error e = {
        .code = ERROR_TYPE_UNDEFINED_IDENTIFIER,
        .trace = {0},
        .pos = expr->value.term->value.ident->ident.pos
      };
      snprintf(e.message, sizeof(e.message), "Unknown identifier");
      da_append(errors, e);
    }

    return tt;
  }

  NodeBinExpr* bin = expr->value.bin_expr;

  TokenType lhs = typecheck_expr(table, bin->lhs, errors);
  TokenType rhs = typecheck_expr(table, bin->rhs, errors);

  if (lhs == TERMINATE || rhs == TERMINATE) return TERMINATE;

  if (is_numeric(lhs) && is_numeric(rhs)) return lhs;

  if (is_string_type(lhs) && is_string_type(rhs)) {
    if (bin->type != EXPR_ADD) {
      Error e = {
        .code = ERROR_TYPE_BINARY_OPERATOR_TYPE_INVALID,
        .trace = {0},
        .pos = bin->op.pos
      };
      snprintf(e.message, sizeof(e.message), "Only + allowed on strings");
      da_append(errors, e);
      return TERMINATE;
    }
    return lhs;
  }

  if (is_char_type(lhs) || is_char_type(rhs)) {
    Error e = {
      .code = ERROR_TYPE_BINARY_OPERATOR_TYPE_INVALID,
      .trace = {0},
      .pos = bin->op.pos
    };
    snprintf(e.message, sizeof(e.message), "Binary operators not supported on char");
    da_append(errors, e);
    return TERMINATE;
  }

  if (!check_type_compat(lhs, rhs)) {
    Error e = {
      .code = ERROR_TYPE_TYPE_MISMATCH,
      .trace = {0},
      .pos = bin->op.pos
    };
    snprintf(e.message, sizeof(e.message), "Type mismatch in binary expression");
    da_append(errors, e);
    return TERMINATE;
  }

  return lhs;
}

static inline void typecheck_stmt(SymbolTable* table, NodeStmt* stmt, ErrorStack* errors) {
  switch (stmt->type) {

    case STMT_EXIT: 
      {
        TokenType t = typecheck_expr(table, stmt->stmt_exit->expr, errors);
        if (t == TERMINATE) break;

        if (!is_numeric(t)) {
          Error e = {
            .code = ERROR_TYPE_EXPECTED_INT,
            .trace = {0},
            .pos = stmt->stmt_exit->expr->value.term->value.ident->ident.pos
          };
          snprintf(e.message, sizeof(e.message), "Exit expects integer expression");
          da_append(errors, e);
        }
      } break;

    case STMT_PRINTLN: 
      {
        TokenType t = typecheck_expr(table, stmt->stmt_print->expr, errors);

        if (t == TERMINATE) break;

        if (!is_string_type(t)) {
          Error e = {
            .code = ERROR_TYPE_EXPECTED_STRING,
            .trace = {0},
            .pos = stmt->stmt_print->expr->value.term->value.ident->ident.pos
          };
          snprintf(e.message, sizeof(e.message), "println expects string expression");
          da_append(errors, e);
        }
      } break;
    case STMT_LET: 
      {
        TokenType rhs = typecheck_expr(table, stmt->stmt_let->expr, errors);
        check_assignment_compat(
            errors,
            stmt->stmt_let->ident,
            stmt->stmt_let->type,
            rhs,
            false,
            false
            );
      } break;

    case STMT_ASSIGN: 
      {
        size_t i = ident_exists(table, stmt->stmt_assign->ident);
        if (i == SIZE_MAX) {
          Error e = {
            .code = ERROR_TYPE_UNDEFINED_IDENTIFIER,
            .message = "The identifier does not exist",
            .trace = {0},
            .pos = stmt->stmt_assign->ident.pos
          };
          da_append(errors, e);
      } break;

        TokenType rhs = typecheck_expr(table, stmt->stmt_assign->expr, errors);

        check_assignment_compat(
            errors,
            stmt->stmt_assign->ident,
            table->items[i].type,
            rhs,
            true,
            table->items[i].mut
            );
      } break;
  }
}

void typecheck_prog(SymbolTable* table, NodeProg* prog, Tokens* tokens) {
  ErrorStack err_stack = {0};

  for (size_t i = 0; i < prog->size; i++) {
    typecheck_stmt(table, &prog->items[i], &err_stack);
  }

  for (size_t i = 0; i < err_stack.size; i++) {
    Error e = err_stack.items[i];
    checker_line_error(tokens, e.pos, e.message);
  }

  if (err_stack.size > 0) exit(1);

  printf(ANSI_COLOR_GREEN "[typechecker]: Semantic analysis finished successfully.\n" ANSI_COLOR_RESET);
}
