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

static inline TokenType term_type(SymbolTable* table, NodeTerm* term) {
  switch (term->type) {
  case TERM_INT_LIT:    return i32_;
  case TERM_STRING_LIT: return string;
  case TERM_CHAR_LIT:   return char_;
  case TERM_IDENT: {
    size_t i = ident_exists(table, term->value.ident->ident);
    if (i == SIZE_MAX) return TERMINATE;
    return table->items[i].type;
  }
  }
  return TERMINATE;
}


/// Checks whether a term satisfies a type predicate.
/// Handles undefined ident (TERMINATE) case internally.
/// Pushes an error if the type check fails.
///
/// Parameters:
///   errors     - the error stack to push into
///   table      - symbol table for ident lookups
///   term       - the term to check
///   type_check - predicate function (is_numeric, is_string_type, is_char_type)
///   code       - error code to use on failure
///   msg        - error message to use on failure
static inline void check_expr_type(
    ErrorStack* errors,
    SymbolTable* table,
    NodeTerm* term,
    bool (*type_check)(TokenType),
    ErrorCode code,
    const char* msg) {
  TokenType tt = term_type(table, term);
  TokenPos pos;
  switch (term->type) {
  case TERM_INT_LIT:    pos = term->value.int_lit->int_lit.pos; break;
  case TERM_STRING_LIT: pos = term->value.string_lit->string_lit.pos; break;
  case TERM_CHAR_LIT:   pos = term->value.char_lit->char_lit.pos; break;
  case TERM_IDENT:      pos = term->value.ident->ident.pos; break;
  default:              pos = (TokenPos){0}; break;
  }
  if (tt == TERMINATE) {
    Error e = {
      .code = ERROR_TYPE_UNDEFINED_IDENTIFIER,
      .trace = {0},
      .pos = pos
    };
    snprintf(e.message, sizeof(e.message), "Unknown identifier");
    da_append(errors, e);
    return;
  }
  if (type_check(tt)) return;
  Error e = {
    .code = code,
    .trace = {0},
    .pos = pos
  };
  snprintf(e.message, sizeof(e.message), "%s", msg);
  da_append(errors, e);
}


/// Checks whether an assignment is valid given the lhs and rhs types.
/// Pushes errors into the error stack on failure, returns void on success.
///
/// Parameters:
///   errors     - the error stack to push into
///   lhs_token  - the token of the variable being assigned to (for position in error reporting)
///   lhs_type   - the declared type of the lhs variable (from symbol table or stmt->stmt_let->type)
///   rhs_type   - the type of the rhs expression (from term_type())
///   check_mut  - whether to check immutability (true for STMT_ASSIGN, false for STMT_LET)
///   is_mut     - whether the lhs variable is mutable (from symbol table, ignored if check_mut=false)
///
/// Use cases:
///   STMT_LET:    check_assignment_compat(errors, ident, stmt->stmt_let->type, rhs, false, false)
///   STMT_ASSIGN: check_assignment_compat(errors, ident, table->items[i].type, rhs, true, table->items[i].mut)
///
/// Type compatibility rules:
///   numeric  <- numeric  : ok
///   string   <- string   : ok
///   char     <- char     : ok
///   anything else        : type mismatch error, message driven by lhs type category
static inline void check_assignment_compat(
    ErrorStack* errors,
    Token lhs_token,
    TokenType lhs_type,
    TokenType rhs_type,
    bool check_mut,
    bool is_mut) {
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
  }
  else if (is_string_type(lhs_type)) {
    code = ERROR_TYPE_EXPECTED_STRING;
    msg = "Expected string type";
  }
  else if (is_char_type(lhs_type)) {
    code = ERROR_TYPE_EXPECTED_CHAR;
    msg = "Expected char type";
  }
  else {
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
      check_expr_type(errors, table,
          stmt->stmt_exit->expr->value.term,
          is_numeric, ERROR_TYPE_EXPECTED_INT,
          "Exit expects integer expression");
    } break;

  case STMT_PRINTLN: 
    {
      check_expr_type(errors, table,
          stmt->stmt_print->expr->value.term,
          is_string_type, ERROR_TYPE_EXPECTED_STRING,
          "println expects string expression");
    } break;
  case STMT_LET: 
    {
      TokenType rhs = term_type(table, stmt->stmt_let->expr->value.term);
      check_assignment_compat(errors, stmt->stmt_let->ident,
          stmt->stmt_let->type, rhs, false, false);
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
        break;
      }
      TokenType rhs = term_type(table, stmt->stmt_assign->expr->value.term);
      check_assignment_compat(errors, stmt->stmt_assign->ident,
          table->items[i].type, rhs, true, table->items[i].mut);
    } break;
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
