#include "./parser.h"
#include "../include/inttypes.h"

__attribute__((warn_unused_result))
   static inline Token peek(Parser* p, size_t offset) {
      if (p->index+offset >= p->tokens->size ) return (Token){.type = TERMINATE, .pos = {0}, .value = ""}; // I never actually use the TERMINATE token
      return p->tokens->items[p->index+offset];
   }

static inline Token consume(Parser* p) {
   if (p->index >= p->tokens->size) {
      printf("[parser]: [ERROR]: Unexpected end of tokens");
      exit(1);
   }
   return p->tokens->items[p->index++];
}


static inline Token try_consume(Parser* p, TokenType type, char* err_msg) {
   Token token = p->tokens->items[p->index];

   if (type == semi) {
      if (peek(p, 0).type == semi){
         return consume(p);
      }
      else {
         printf(ANSI_COLOR_RED"[parser]: %s at line %zu\n"ANSI_COLOR_RESET, err_msg, token.pos.line-1);
         exit(1);
      }
   }
   if (peek(p, 0).type == type) {
      return consume(p);
   }
   else {
      printf(ANSI_COLOR_RED"[parser]: %s at line %zu:%zu\n"ANSI_COLOR_RESET, err_msg, token.pos.line, token.pos.line_pos-strlen(token.value));
      exit(1);
   }
}

Parser Parser_create(Tokens* tokens) {
   Parser p = { 0 };
   mem_arena* arena = arena_create(MiB(4));
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
   else if (peek(p, 0).type == ident) {
      NodeTermIdent* term_ident = arena_push(p->arena, NodeTermIdent);
      term_ident->ident = consume(p);
      term->value.ident = term_ident;
      term->type = TERM_IDENT;
   }
   else {
      printf("Incomplete expression.\n");
      exit(1);
   }
   return term;
}

// TODO Implement an actual pratt parser. This works for now because it is dependent on gcc
// More complicated custom syntax expressions *WILL* fail.
__attribute__((warn_unused_result))
   static inline NodeExpr* parse_expr(Parser* p) {
      NodeExpr* expr = arena_push(p->arena, NodeExpr);
      NodeTerm* term = parse_term(p);
      if (peek(p, 0).type == plus) {
         NodeBinExpr* bin_expr = arena_push(p->arena, NodeBinExpr);
         NodeBinExprAdd* bin_expr_add = arena_push(p->arena, NodeBinExprAdd);
         NodeExpr* lhs_expr = arena_push(p->arena, NodeExpr);
         bin_expr->type = EXPR_ADD;
         lhs_expr->value.term = term;
         lhs_expr->type = EXPR_TERM;
         bin_expr_add->lhs = lhs_expr;
         consume(p);
         NodeExpr* rhs = parse_expr(p);
         bin_expr_add->rhs = rhs;
         bin_expr->var.add = bin_expr_add;
         expr->value.bin_expr = bin_expr;
         expr->type = EXPR_BIN_EXPR;
         return expr;
      }
      else if (peek(p, 0).type == star) {
         NodeBinExpr* bin_expr = arena_push(p->arena, NodeBinExpr);
         NodeBinExprMulti* bin_expr_multi = arena_push(p->arena, NodeBinExprMulti);
         NodeExpr* lhs_expr = arena_push(p->arena, NodeExpr);
         bin_expr->type = EXPR_MULTI;
         lhs_expr->value.term = term;
         lhs_expr->type = EXPR_TERM;
         bin_expr_multi->lhs = lhs_expr;
         consume(p);
         NodeExpr* rhs = parse_expr(p);
         bin_expr_multi->rhs = rhs;
         bin_expr->var.multi = bin_expr_multi;
         expr->value.bin_expr = bin_expr;
         expr->type = EXPR_BIN_EXPR;
         return expr;
      }
      expr->type = EXPR_TERM;
      expr->value.term = term;
      return expr;
   }

static inline NodeStmt* parse_stmt(Parser* p) {
   NodeStmt* stmt = arena_push(p->arena, NodeStmt);
   if (peek(p, 0).type == let // TODO: Type system
         && peek(p, 1).type == ident
         && peek(p, 2).type == equals) {
      NodeStmtLet* stmt_let = arena_push(p->arena, NodeStmtLet);
      consume(p);
      stmt_let->ident = consume(p);
      stmt->stmt_let = stmt_let;
      consume(p);
      stmt->stmt_let->expr = parse_expr(p);
      stmt->type = STMT_LET;
      try_consume(p, semi, "Expected a ';'");
   }
   else if (peek(p, 0).type == exit_
         && peek(p, 1).type == open_paren) {
      NodeStmtExit* stmt_exit = arena_push(p->arena, NodeStmtExit);
      consume(p);
      consume(p);
      stmt_exit->expr = parse_expr(p);
      stmt->stmt_exit = stmt_exit;
      stmt->type = STMT_EXIT;
      try_consume(p, close_paren, "Expected a ')'");
      try_consume(p, semi       , "Expected a ';'");
   }
   else if (peek(p, 0).type == ident
         && peek(p, 1).type == equals) {
      NodeStmtAssign* ptr = arena_push(p->arena, NodeStmtAssign);
      ptr->ident = consume(p);
      stmt->stmt_assign = ptr;
      consume(p);
      stmt->stmt_assign->expr = parse_expr(p);
      stmt->type = STMT_ASSIGN;
      try_consume(p, semi, "Expected a ';'");
   }
   else if (peek(p, 0).type == print
         && peek(p, 1).type == open_paren) {
      NodeStmtPrint* ptr = arena_push(p->arena, NodeStmtPrint);
      consume(p);
      consume(p);
      ptr->expr = parse_expr(p);
      stmt->stmt_print = ptr;
      stmt->type = STMT_PRINTLN;
      try_consume(p, close_paren, "Expected a ')'");
      try_consume(p, semi       , "Expected a ';'");
   }
   else {
      printf("[parser]: Invalid statement at line %zu\n", p->tokens->items[p->index].pos.line);
      exit(1);
   }
   return stmt;
}

NodeProg parse_prog(Parser* p) {
   NodeProg prog = { 0 };
   while (p->index < p->tokens->size) {
      NodeStmt* stmt = parse_stmt(p);
      da_append(&prog, *stmt);
   }
   puts(ANSI_COLOR_GREEN"[lexer]: Compilation finished successfully."ANSI_COLOR_RESET);
   return prog;
}
