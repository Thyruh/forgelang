#include "./parser.h"
#include "../include/inttypes.h"

__attribute__((warn_unused_result))
   static inline Token peek(Parser* p, size_t offset) {
      if (p->index+offset >= p->tokens->size )
         return (Token){.type = TERMINATE, .pos = {0}, .value = ""};
      return p->tokens->items[p->index+offset];
   }

static inline Token consume(Parser* p) {
   if (p->index >= p->tokens->size) {
      printf(ANSI_COLOR_RED"[parser]: [ERROR]: Unexpected end of tokens"ANSI_COLOR_RESET);
      exit(1);
   }
   return p->tokens->items[p->index++];
}

// This function walks back until encountering a token from a previous line.
// Increments p->index by one to jump back to the original line, then walks 
// to the right while on that same line, printing stringified tokens one by one.
// Outputs the total length amount of whitespaces and a caret at the end to point
// at the error spot.
//
// TLDR: Outputs the entire line where an issue happened based on p-index, along with a 
// provided error message
//
// TODO implement a similar function for generator
static inline void print_line_error(Parser* p, char* err_msg, u8 offset) {
   TokenPos pos = p->tokens->items[p->index-offset].pos;
   printf(ANSI_COLOR_RED"[parser]: %s at line %zu in:\n"ANSI_COLOR_RESET"\t`", err_msg, pos.line);
   while (p->index > 0 && p->tokens->items[p->index-offset].pos.line == pos.line) {
      p->index--;
   }
   p->index++;
   size_t visual_width = 0; // offset based on each tokens length
   while (p->index < p->tokens->size && p->tokens->items[p->index-offset].pos.line == pos.line) {
      const char* s = token_repr(p->tokens->items[p->index-offset]);
      printf(ANSI_COLOR_YELLOW"%s ", s);
      if (p->tokens->items[p->index-offset].pos.line_pos <= pos.line_pos) {
         // +1 is for a space that gets added between the tokens
         visual_width += strlen(s) + 1;
      }
      p->index++;
   }
   puts("`"ANSI_COLOR_RESET);
   printf("\t");
   for (size_t _ = 0; _ < visual_width; _++) {
      printf(" ");
   }
   printf("^\n");
   exit(1);
}

static inline Token try_consume(Parser* p, TokenType type, char* err_msg) {
   if (type == semi) {
      if (peek(p, 0).type == semi) {
         return consume(p);
      }
      else {
         print_line_error(p, err_msg, 1);
         exit(1);
      }
   }
   if (peek(p, 0).type == type) {
      return consume(p);
   }
   else print_line_error(p, err_msg, 1);
   exit(1); // to silence void return warning
}

Parser Parser_create(Tokens* tokens, mem_arena* arena) {
   Parser p = { 0 };
   p.tokens = tokens;
   p.arena = arena;
   return p;
}

static inline NodeTerm* parse_term(Parser* p) {
   NodeTerm* term = arena_push(p->arena, NodeTerm);
   if (peek(p, 0).type == int_lit) {
      NodeTermIntLit* term_int_lit = arena_push(p->arena, NodeTermIntLit);
      term_int_lit->int_lit = consume(p);
      term->value.int_lit = term_int_lit;
      term->type = TERM_INT_LIT;
   }
   else if (peek(p, 0).type == string_lit) {
      NodeTermStringLit* term_string = arena_push(p->arena, NodeTermStringLit);
      term_string->string_lit = consume(p);
      term->value.string_lit = term_string;
      term->type = TERM_STRING_LIT;
   }
   else if (peek(p, 0).type == char_lit) {
      NodeTermCharLit* term_char = arena_push(p->arena, NodeTermCharLit);
      term_char->char_lit= consume(p);
      term->value.char_lit = term_char;
      term->type = TERM_CHAR_LIT;
   }
   else if (peek(p, 0).type == ident) {
      NodeTermIdent* term_ident = arena_push(p->arena, NodeTermIdent);
      term_ident->ident = consume(p);
      term->value.ident = term_ident;
      term->type = TERM_IDENT;
   }
   else {
      print_line_error(p, "Incomplete binary expression", 1);
      exit(1);
   }
   return term;
}

static inline int get_prec(TokenType t) {
   switch (t) {
      case plus: case minus: return 1;
      case star: case fslash: return 2;
      default: return -1;
   }
}

// TODO Add unary operations like negatives, negation, ++ and --
static inline NodeExpr* parse_expr(Parser* p, int min_prec) {
   NodeExpr* lhs = arena_push(p->arena, NodeExpr);
   NodeTerm* term = parse_term(p);
   lhs->type = EXPR_TERM;
   lhs->value.term = term;

   while (1) {
      TokenType op = peek(p, 0).type;
      int prec = get_prec(op);
      if (prec < min_prec) break;

      consume(p);
      NodeExpr* rhs = parse_expr(p, prec+1);
      NodeBinExpr* bin_expr = arena_push(p->arena, NodeBinExpr);
      NodeExpr* new_lhs = arena_push(p->arena, NodeExpr);
      if (op == plus) {
         NodeBinExprAdd* bin_expr_add = arena_push(p->arena, NodeBinExprAdd);
         bin_expr_add->lhs = lhs;
         bin_expr_add->rhs = rhs;
         bin_expr->type = EXPR_ADD;
         bin_expr->var.add = bin_expr_add;
      }
      else if (op == star) {
         NodeBinExprMulti* bin_expr_multi = arena_push(p->arena, NodeBinExprMulti);
         bin_expr_multi->lhs = lhs;
         bin_expr_multi->rhs = rhs;
         bin_expr->type = EXPR_MULTI;
         bin_expr->var.multi = bin_expr_multi;
      }
      else if (op == fslash) {
         NodeBinExprMulti* bin_expr_multi = arena_push(p->arena, NodeBinExprMulti);
         bin_expr_multi->lhs = lhs;
         bin_expr_multi->rhs = rhs;
         bin_expr->type = EXPR_DIVIDE;
         bin_expr->var.multi = bin_expr_multi;
      }
      else if (op == minus) {
         NodeBinExprMulti* bin_expr_multi = arena_push(p->arena, NodeBinExprMulti);
         bin_expr_multi->lhs = lhs;
         bin_expr_multi->rhs = rhs;
         bin_expr->type = EXPR_SUBTR;
         bin_expr->var.multi = bin_expr_multi;
      }
      new_lhs->type = EXPR_BIN_EXPR;
      new_lhs->value.bin_expr = bin_expr;
      lhs = new_lhs;
   }
   return lhs;
}

// TODO: Type tracker creation
static inline NodeStmt* parse_stmt(Parser* p) {
   NodeStmt* stmt = arena_push(p->arena, NodeStmt);
   if ((peek(p, 0).type == mut || peek(p, 0).type == const_)
         && peek(p, 1).type == ident
         && peek(p, 2).type == colon) {
      NodeStmtLet* stmt_let = arena_push(p->arena, NodeStmtLet);
      stmt_let->mut = false;
      if (peek(p, 0).type == mut) {
         stmt_let->mut = true;
      }
      consume(p);
      stmt_let->ident = consume(p);
      consume(p);
      stmt_let->token = consume(p);
      try_consume(p, equals, "Expected a '='");
      stmt->stmt_let = stmt_let;
      stmt->stmt_let->expr = parse_expr(p, 0);
      stmt->type = STMT_LET;
      try_consume(p, semi, "Expected a ';'");
   }
   else if (peek(p, 0).type == exit_
         && peek(p, 1).type == open_paren) {
      NodeStmtExit* stmt_exit = arena_push(p->arena, NodeStmtExit);
      consume(p);
      consume(p);
      stmt_exit->expr = parse_expr(p, 0);
      stmt->stmt_exit = stmt_exit;
      stmt->type = STMT_EXIT;
      try_consume(p, close_paren, "Expected a ')'");
      try_consume(p, semi       , "Expected a ';'");
   }
   else if (peek(p, 0).type == ident
         && peek(p, 1).type == equals) {
      NodeStmtAssign* stmt_assign = arena_push(p->arena, NodeStmtAssign);
      stmt_assign->ident = consume(p);
      stmt->stmt_assign = stmt_assign;
      consume(p);
      stmt->stmt_assign->expr = parse_expr(p, 0);
      stmt->type = STMT_ASSIGN;
      try_consume(p, semi, "Expected a ';'");
   }
   else if (peek(p, 0).type == println
         && peek(p, 1).type == open_paren) {
      NodeStmtPrint* stmt_println = arena_push(p->arena, NodeStmtPrint);
      consume(p);
      consume(p);
      stmt_println->expr = parse_expr(p, 0);
      stmt->stmt_print = stmt_println;
      stmt->type = STMT_PRINTLN;
      try_consume(p, close_paren, "Expected a ')'");
      try_consume(p, semi       , "Expected a ';'");
   }
   else {
      print_line_error(p, "Invalid statement", 0);
   }
   return stmt;
}

NodeProg parse_prog(Parser* p) {
   NodeProg prog = { 0 };
   while (p->index < p->tokens->size) {
      NodeStmt* stmt = parse_stmt(p);
      da_append(&prog, *stmt);
   }
   puts(ANSI_COLOR_GREEN"[lexer]: Backend pipeline completed successfully."ANSI_COLOR_RESET);
   return prog;
}
