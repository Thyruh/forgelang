#include "./parser.h"
#include "../include/inttypes.h"

static inline Token peek(Parser* p, size_t offset) {
   if (p->index+offset >= p->tokens->size ) return (Token){TERMINATE, ""}; // I never actually use the TERMINATE token
   return p->tokens->items[p->index+offset];
}

static inline Token consume(Parser* p) {
   if (p->index >= p->tokens->size) {
      printf("[ERROR]: Unexpected end of tokens");
      exit(1);
   }
   return p->tokens->items[p->index++];
}

Parser Parser_create(Tokens* tokens) {
   Parser p = { 0 };
   p.tokens = tokens;
   return p;
}

static inline NodeExpr parse_expr(Parser* p) { // TODO pratt parsing
   NodeExpr expr = { 0 };

   if (peek(p, 0).type == int_lit) {
      expr.type = EXPR_INT_LIT;
      expr.value.int_lit = (NodeExprIntLit){consume(p)};
      return expr;
   }
   else {
      expr.type = EXPR_IDENT;
      expr.value.ident = (NodeExprIdent){consume(p)};
   }

   return expr;
}

static inline NodeStmt parse_stmt(Parser* p) {
   NodeStmt stmt = { 0 };
   if (peek(p, 0).type == let
    && peek(p, 1).type == ident
    && peek(p, 2).type == equals) {
      consume(p);
      stmt.stmt_let.ident = consume(p);
      consume(p);
      stmt.stmt_let.expr = parse_expr(p);
      stmt.type = STMT_LET; 
      if (peek(p, 0).type != semi) {
         printf("Missing a semicolon\n");
         exit(1);
      }
      consume(p);
   }
   else if (peek(p, 0).type == exit_
         && peek(p, 1).type == open_paren) {
      consume(p);
      consume(p);
      stmt.stmt_exit.expr = parse_expr(p);
      stmt.type = STMT_EXIT; 
      if (peek(p, 0).type == close_paren) {
         consume(p);
      }
      else {
         printf("Missing a paren\n");
         exit(1);
      }
      if (peek(p, 0).type == semi) {
         consume(p);
      }
      else {
         printf("Missing a semicolon\n");
         exit(1);
      }
   }
   else {
      printf("Invalid expression\n");
      exit(1);
   }
   return stmt;
}

NodeProg parse_prog(Parser* p) {
   NodeProg prog = { 0 };
   while (p->index < p->tokens->size) {
      NodeStmt stmt = parse_stmt(p);
      da_append(&prog, stmt);
   }
   printf(ANSI_COLOR_GREEN"Compilation finished successfully%s\n", ANSI_COLOR_RESET);
   return prog;
}
