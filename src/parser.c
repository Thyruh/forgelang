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
   return term;
}

// TODO Implement an actual pratt parser. This works for now because it is dependent on gcc
// More complicated custom syntax expressions *WILL* fail.
__attribute__((warn_unused_result))
   static inline NodeExpr* parse_expr(Parser* p) {
      NodeExpr* expr = arena_push(p->arena, NodeExpr);
      NodeTerm* term = parse_term(p);
      if (!term) {
         printf("Missing the second expression.\n");
         exit(1);
      }
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
   else if (peek(p, 0).type == print
         && peek(p, 1).type == open_paren) {
      NodeStmtPrint* ptr = arena_push(p->arena, NodeStmtPrint);
      consume(p);
      consume(p);
      ptr->expr = parse_expr(p);
      stmt->stmt_print = ptr;
      stmt->type = STMT_PRINT;
      if (peek(p, 0).type == close_paren) {
         consume(p);
      }
      else {
         printf("[parser]: Missing a closing paren.\n");
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
