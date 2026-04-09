#include "./parser.h"
#include "../include/inttypes.h"

__attribute__((warn_unused_result))
static inline Token peek(Parser* p, size_t offset) {
   if (p->index+offset >= p->tokens->size ) return (Token){TERMINATE, {0}, ""}; // I never actually use the TERMINATE token
   return p->tokens->items[p->index+offset];
}

static inline Token consume(Parser* p) {
   if (p->index >= p->tokens->size) {
      printf("[parser]: [ERROR]: Unexpected end of tokens");
      exit(1);
   }
   return p->tokens->items[p->index++];
}

Parser Parser_create(Tokens* tokens) {
   Parser p = { 0 };
   mem_arena* arena = arena_create(MiB(4));
   p.tokens = tokens;
   p.arena = arena;
   return p;
}

__attribute__((warn_unused_result))
static inline NodeExpr* parse_expr(Parser* p) { // TODO pratt parsing
   NodeExpr* expr = arena_push(p->arena, NodeExpr);
   if (peek(p, 0).type == int_lit) {
      NodeExprIntLit* ptr = arena_push(p->arena, NodeExprIntLit);
      ptr->int_lit = consume(p);
      expr->value.int_lit = ptr;
      expr->type = EXPR_INT_LIT;
   }
   else if (peek(p, 0).type == ident) {
      NodeExprIdent* ptr = arena_push(p->arena, NodeExprIdent);
      ptr->ident = consume(p);
      expr->value.ident = ptr;
      expr->type = EXPR_IDENT;
   }
   else {
      printf("Unimplemented. You might be onto something!\n");
      exit(1);
   }
   return expr;
}

static inline NodeStmt* parse_stmt(Parser* p) {
   NodeStmt* stmt = arena_push(p->arena, NodeStmt);
   if (peek(p, 0).type == let
    && peek(p, 1).type == ident
    && peek(p, 2).type == equals) {
      NodeStmtLet* ptr = arena_push(p->arena, NodeStmtLet);
      consume(p);
      ptr->ident = consume(p);
      stmt->stmt_let = ptr;
      consume(p);
      stmt->stmt_let->expr = parse_expr(p);
      stmt->type = STMT_LET;
      if (peek(p, 0).type != semi) {
         printf("[parser]: Missing a semicolon.\n");
         exit(1);
      }
      consume(p);
   }
   else if (peek(p, 0).type == exit_
         && peek(p, 1).type == open_paren) {
      NodeStmtExit* ptr = arena_push(p->arena, NodeStmtExit);
      consume(p);
      consume(p);
      ptr->expr = parse_expr(p);
      stmt->stmt_exit = ptr;
      stmt->type = STMT_EXIT;
      if (peek(p, 0).type == close_paren) {
         consume(p);
      }
      else {
         printf("[parser]: Missing a paren.\n");
         exit(1);
      }
      if (peek(p, 0).type == semi) {
         consume(p);
      }
      else {
         printf("[parser]: Missing a semicolon.\n");
         exit(1);
      }
   }
   else if (peek(p, 0).type == ident
         && peek(p, 1).type == equals) {
      NodeStmtAssign* ptr = arena_push(p->arena, NodeStmtAssign);
      ptr->ident = consume(p);
      stmt->stmt_assign = ptr;
      consume(p);
      stmt->stmt_assign->expr = parse_expr(p);
      stmt->type = STMT_ASSIGN;
      if (peek(p, 0).type == semi) {
         consume(p);
      }
      else {
         printf("[parser]: Missing a semicolon.\n");
         exit(1);
      }
   }
   else {
      printf("[parser]: Invalid statement.\n");
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
   printf(ANSI_COLOR_GREEN"[lexer]: Compilation finished successfully.%s\n", ANSI_COLOR_RESET);
   return prog;
}
