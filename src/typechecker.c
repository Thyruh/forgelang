// typecheck_ functions return; on success allowing the program to move on to generation

#include "./typechecker.h"
#include "./common/tokenhandlers.h"

static inline bool is_int_type(TokenType t) { return (t >= i8_ && t <= u64_) || t == usize; }
static inline bool is_float_type(TokenType t) { return t >= f32_ && t <= f64_; }
static inline bool is_numeric(TokenType t) { return is_int_type(t) || is_float_type(t); }
// static inline bool is_ptr_type(TokenType t) { return t == ptr || t == uptr; }
static inline bool is_string_type(TokenType t) { return t == string; }
static inline bool is_char_type(TokenType t) { return t == char_ || t == uchar_; }

/// Scans forward through the token array to find the line matching pos.line,
/// prints all tokens on that line, then exits with the provided error message.
static inline void checker_line_error(Tokens* tokens, TokenPos pos, char* err_msg) {
  printf(ANSI_COLOR_RED "[typechecker]: %s at line %zu in:\n" ANSI_COLOR_RESET,
      err_msg, pos.line);
  size_t i = 0;
  while (i < tokens->size && tokens->items[i].pos.line < pos.line) {
    i++;
  }
  printf("\t"ANSI_COLOR_YELLOW);
  while (i < tokens->size && tokens->items[i].pos.line == pos.line) {
    const char *s = token_repr(tokens->items[i]);
    if (tokens->items[i].type == char_lit) {
      printf("'%s' ", s);
    } 
    else if (tokens->items[i].type == string_lit) {
      printf("\"%s\" ", s);
    }
    else if (s[0] != ';') {
      printf("%s ", s);
    }
    else {
      printf(";");
    }
    i++;
  }
  puts(ANSI_COLOR_RESET);
}

static inline NodeExpr* stmt_expr(NodeStmt* stmt) {
  switch (stmt->type) {
  case STMT_EXIT:    return stmt->stmt_exit->expr;
  case STMT_LET:     return stmt->stmt_let->expr;
  case STMT_PRINTLN: return stmt->stmt_print->expr;
  case STMT_ASSIGN:  return stmt->stmt_assign->expr;
  }
  return NULL;
}

/// Checks if the given ident exists inside the symbol table.
/// Returns the index `i` of the symbol inside the symbol table on success
/// Returns SIZE_MAX on failure
static inline size_t ident_exists(SymbolTable* table, Token token) {
  for (size_t i = 0; i < table->size; i++) {
    if (!strcmp(table->items[i].ident, token.value)) {
      return i;
    }
  }
  return SIZE_MAX;
}

// TODO recursive expr check
static inline void typecheck_expr() {
  assert(false && "typecheck_expr is not implemented.");
}

static inline void typecheck_stmt(SymbolTable* table, NodeStmt* stmt, ErrorStack* errors) {
  NodeExpr* expr = stmt_expr(stmt);
  if (expr->type == EXPR_BIN_EXPR) {
    typecheck_expr();
  }
  switch (stmt->type) {
  case STMT_EXIT: 
    {
      switch (stmt->stmt_exit->expr->value.term->type) {
      case TERM_INT_LIT: 
        {
          return;
        } break;
      case TERM_IDENT:
        {
          Token ident = stmt->stmt_exit->expr->value.term->value.ident->ident;
          size_t i = ident_exists(table, ident);
          if (i == SIZE_MAX) {
            Error e = {
              .code = ERROR_TYPE_UNDEFINED_IDENTIFIER,
              .message = "The identifier does not exist",
              .trace = {0},
              .pos = ident.pos
            };
            da_append(errors, e);
            return; // Can immediately return, cause there is nothing else to check in this statement
          }
          if (is_numeric(table->items[i].type)) return;
          Error e = {
            .code = ERROR_TYPE_EXPECTED_INT,
            .message = "Exit expects integer expression",
            .trace = {0},
            .pos = ident.pos
          };
          da_append(errors, e);
        } break;
      case TERM_STRING_LIT:
        {
          Error e = {
            .code = ERROR_TYPE_EXPECTED_INT,
            .message = "Exit expects integer expression",
            .trace = {0},
            .pos = stmt->stmt_exit->expr->value.term->value.string_lit->string_lit.pos
          };
          da_append(errors, e);
        } break;
      case TERM_CHAR_LIT:
        {
          Error e = {
            .code = ERROR_TYPE_EXPECTED_INT,
            .message = "Exit expects integer expression",
            .trace = {0},
            .pos = stmt->stmt_exit->expr->value.term->value.string_lit->string_lit.pos
          };
          da_append(errors, e);
        } break;
      }    // switch term.type
    } break; // case STMT_EXIT
  case STMT_LET: 
    {
      switch (stmt->stmt_let->expr->value.term->type) { // rhs cases
      case TERM_INT_LIT: 
        {
          // The check here is purely to find the correct symbol table entry
          size_t i = ident_exists(table, stmt->stmt_let->ident);
          TokenType tt = table->items[i].type;
          if (is_numeric(tt)) return;

          Error e = {
            .code = ERROR_TYPE_EXPECTED_INT,
            .message = "Let expects integer type",
            .trace = {0},
            .pos = stmt->stmt_let->ident.pos
          };
          da_append(errors, e);

        } break;
      case TERM_STRING_LIT: 
        {
          size_t i = ident_exists(table, stmt->stmt_let->ident);
          TokenType tt = table->items[i].type;
          if (is_string_type(tt)) return;

          Error e = {
            .code = ERROR_TYPE_EXPECTED_STRING,
            .message = "Let expects string type",
            .trace = {0},
            .pos = stmt->stmt_let->ident.pos
          };
          da_append(errors, e);
        } break;
      case TERM_CHAR_LIT: 
        {
          size_t i = ident_exists(table, stmt->stmt_let->ident);
          TokenType tt = table->items[i].type;
          if (is_char_type(tt)) return;
          Error e = {
            .code = ERROR_TYPE_EXPECTED_CHAR,
            .message = "Let expects char type",
            .trace = {0},
            .pos = stmt->stmt_let->ident.pos
          };
          da_append(errors, e);
        } break;
      case TERM_IDENT:
        {
          Token ident = stmt->stmt_let->expr->value.term->value.ident->ident;
          size_t i = ident_exists(table, ident);
          if (i == SIZE_MAX) {
            Error e = {
              .code = ERROR_TYPE_UNDEFINED_IDENTIFIER,
              .message = "The identifier does not exist",
              .trace = {0},
              .pos = ident.pos
            };
            da_append(errors, e);
            return;
          }
          TokenType tt = table->items[i].type;
          if (is_numeric(tt) && is_numeric(stmt->stmt_let->type)) {
            return;
          }
          else {
            Error e = {
              .code = ERROR_TYPE_EXPECTED_INT,
              .message = "Let expects integer type",
              .trace = {0},
              .pos = ident.pos
            };
            da_append(errors, e);
          }

          if (is_string_type(tt) && is_string_type(stmt->stmt_let->type)) {
            return;
          }
          else { // this is unnecessary
            Error e = {
              .code = ERROR_TYPE_EXPECTED_STRING,
              .message = "Let expects string type",
              .trace = {0},
              .pos = ident.pos
            };
            da_append(errors, e);
          }

          if (is_char_type(tt) && is_char_type(stmt->stmt_let->type)) {
            return;
          }
          else {
            Error e = {
              .code = ERROR_TYPE_EXPECTED_CHAR,
              .message = "Let expects char type",
              .trace = {0},
              .pos = ident.pos
            };
            da_append(errors, e);
          }
        } break;
      } // switch term.type
    } break; // case STMT_LET
  case STMT_PRINTLN: 
    {
      switch (stmt->stmt_print->expr->value.term->type) {
      case TERM_STRING_LIT: 
        {
          return;
        } break;
      case TERM_IDENT:
        {
          Token ident = stmt->stmt_print->expr->value.term->value.ident->ident;
          size_t i = ident_exists(table, ident);
          if (i == SIZE_MAX) {
            Error e = {
              .code = ERROR_TYPE_UNDEFINED_IDENTIFIER,
              .message = "The identifier does not exist",
              .trace = {0},
              .pos = ident.pos
            };
            da_append(errors, e);
            return; // Can immediately return, cause there is nothing else to check in this statement
          }
          if (is_string_type(table->items[i].type)) return;
          Error e = {
            .code = ERROR_TYPE_EXPECTED_INT,
            .message = "Statement println expects string expression",
            .trace = {0},
            .pos = ident.pos
          };
          da_append(errors, e);
        } break;
      case TERM_INT_LIT:
        {
          Error e = {
            .code = ERROR_TYPE_EXPECTED_INT,
            .message = "println expects string expression",
            .trace = {0},
            .pos = stmt->stmt_print->expr->value.term->value.int_lit->int_lit.pos
          };
          da_append(errors, e);
        } break;
      case TERM_CHAR_LIT:
        {
          Error e = {
            .code = ERROR_TYPE_EXPECTED_INT,
            .message = "println expects string expression",
            .trace = {0},
            .pos = stmt->stmt_print->expr->value.term->value.char_lit->char_lit.pos
          };
          da_append(errors, e);
        } break;
      } // switch term.type
    } break; // case STMT_PRINTLN
  case STMT_ASSIGN: 
    {
      switch (stmt->stmt_assign->expr->value.term->type) { // rhs cases
      case TERM_INT_LIT: 
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
            return;
          }
          TokenType tt = table->items[i].type;
          if (!table->items[i].mut) {
            // TODO: Use snprintf for message to populate it with formatted strings
            Error e = {
              .code = ERROR_TYPE_CONST_REASSIGNMENT,
              .message = "Attempting to change an immutable variable",
              .trace = {0},
              .pos = stmt->stmt_assign->ident.pos
            };
            da_append(errors, e);
          }

          if (is_numeric(tt)) return;

          Error e = {
            .code = ERROR_TYPE_EXPECTED_INT,
            .message = "Assignment expects integer type",
            .trace = {0},
            .pos = stmt->stmt_assign->ident.pos
          };
          da_append(errors, e);

        } break;
      case TERM_STRING_LIT: 
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
            return;
          }
          TokenType tt = table->items[i].type;
          if (!table->items[i].mut) {
            Error e = {
              .code = ERROR_TYPE_CONST_REASSIGNMENT,
              .message = "Attempting to change an immutable variable",
              .trace = {0},
              .pos = stmt->stmt_assign->ident.pos
            };
            da_append(errors, e);
          }

          if (is_string_type(tt)) return;

          Error e = {
            .code = ERROR_TYPE_EXPECTED_STRING,
            .message = "Assignment expects string type",
            .trace = {0},
            .pos = stmt->stmt_assign->ident.pos
          };
          da_append(errors, e);
        } break;
      case TERM_CHAR_LIT: 
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
            return;
          }
          TokenType tt = table->items[i].type;
          if (!table->items[i].mut) {
            Error e = {
              .code = ERROR_TYPE_CONST_REASSIGNMENT,
              .message = "Attempting to change an immutable variable",
              .trace = {0},
              .pos = stmt->stmt_assign->ident.pos
            };
            da_append(errors, e);
          }

          if (is_char_type(tt)) return;

          Error e = {
            .code = ERROR_TYPE_EXPECTED_CHAR,
            .message = "Assignment expects char type",
            .trace = {0},
            .pos = stmt->stmt_assign->ident.pos
          };
          da_append(errors, e);
        } break;
      case TERM_IDENT:
        {
          Token ident = stmt->stmt_assign->expr->value.term->value.ident->ident;
          size_t i = ident_exists(table, ident);
          if (i == SIZE_MAX) {
            Error e = {
              .code = ERROR_TYPE_UNDEFINED_IDENTIFIER,
              .message = "The identifier does not exist",
              .trace = {0},
              .pos = ident.pos
            };
            da_append(errors, e);
            return;
          }
          if (!table->items[i].mut) {
            Error e = {
              .code = ERROR_TYPE_CONST_REASSIGNMENT,
              .message = "Attempting to change an immutable variable",
              .trace = {0},
              .pos = stmt->stmt_assign->ident.pos
            };
            da_append(errors, e);
          }
          TokenType tt = table->items[i].type;
          if (is_numeric(tt) && is_numeric(stmt->stmt_let->type)) {
            return;
          }
          else {
            Error e = {
              .code = ERROR_TYPE_EXPECTED_INT,
              .message = "Assignment expects integer type",
              .trace = {0},
              .pos = stmt->stmt_assign->ident.pos
            };
            da_append(errors, e);
          }

          if (is_string_type(tt) && is_string_type(stmt->stmt_assign->ident.type)) {
            return;
          }
          else {
            Error e = {
              .code = ERROR_TYPE_EXPECTED_STRING,
              .message = "Assignment expects string type",
              .trace = {0},
              .pos = stmt->stmt_assign->ident.pos
            };
            da_append(errors, e);
          }

          if (is_char_type(tt) && is_char_type(stmt->stmt_assign->ident.type)) {
            return;
          }
          else {
            Error e = {
              .code = ERROR_TYPE_EXPECTED_CHAR,
              .message = "Assignment expects char type",
              .trace = {0},
              .pos = stmt->stmt_assign->ident.pos
            };
            da_append(errors, e);
          }
        } break;
      } // switch term.type
    } break; // case STMT_ASSIGN
  } // switch stmt.type
} // typecheck_stmt()

void typecheck_prog(SymbolTable* table, NodeProg* prog, Tokens* tokens) {
  ErrorStack err_stack = { 0 };
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
