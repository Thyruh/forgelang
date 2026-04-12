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
         printf(ANSI_COLOR_RED"[parser]: %s at line %zu\n"ANSI_COLOR_RESET, err_msg, token.pos.line);
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

static inline const char* get_type(Token token) {
   switch (token.type) {
      case char_:
         return "char";
      case uchar_:
         return "unsigned char";
      case i8_:
         return "__INT8_TYPE__";
      case i16_:
         return "__INT16_TYPE__";
      case i32_:
         return "__INT32_TYPE__";
      case i64_:
         return "__INT64_TYPE__";
      case u8_:
         return "__UINT8_TYPE__";
      case u16_:
         return "__UINT16_TYPE__";
      case u32_:
         return "__UINT32_TYPE__";
      case u64_:
         return "__UINT64_TYPE__";
      case f32_:
         return "float";
      case f64_:
         return "double";
      case bool_:
         return "_Bool";
      case ptr:
         return "__INTPTR_TYPE__";
      case uptr:
         return "__UINTPTR_TYPE__";
      case string:
         return "string"; // to change later, placeholder
      case ustring:
         return "ustring"; // this too
      default:
         printf("Unknown type %s at %zu:%zu\n", tokentype_repr(token), token.pos.line, token.pos.line_pos-strlen(token.value)+1);
         exit(1);
   }
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

static inline int get_prec(TokenType t) {
   switch (t) {
      case plus: case minus: return 1;
      case star: case fslash: return 2;
      default: return -1;
   }
}

// TODO Add unary operations like negation
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
      // TODO parse slash and hyphon for division and subtraction
      new_lhs->type = EXPR_BIN_EXPR;
      new_lhs->value.bin_expr = bin_expr;
      lhs = new_lhs;
   }
   return lhs;
}

// TODO: Type system
// TODO Combine the mut and const branches as they only differ in the flag.
static inline NodeStmt* parse_stmt(Parser* p) {
   NodeStmt* stmt = arena_push(p->arena, NodeStmt);
   if (peek(p, 0).type == mut
         && peek(p, 1).type == ident
         && peek(p, 2).type == colon) {
      NodeStmtLet* stmt_let = arena_push(p->arena, NodeStmtLet);
      consume(p);
      stmt_let->ident = consume(p);
      consume(p);
      stmt_let->type = get_type(consume(p));
      try_consume(p, equals, "Expected a '='");
      stmt_let->mut = true;
      stmt->stmt_let = stmt_let;
      stmt->stmt_let->expr = parse_expr(p, 0);
      stmt->type = STMT_LET;
      try_consume(p, semi, "Expected a ';'");
   }
   else if (peek(p, 0).type == const_
         && peek(p, 1).type == ident
         && peek(p, 2).type == colon) {
      NodeStmtLet* stmt_let = arena_push(p->arena, NodeStmtLet);
      consume(p);
      stmt_let->ident = consume(p);
      consume(p);
      stmt_let->type = get_type(consume(p));
      try_consume(p, equals, "Expected a '='");
      stmt_let->mut = false;
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
      NodeStmtAssign* ptr = arena_push(p->arena, NodeStmtAssign);
      ptr->ident = consume(p);
      stmt->stmt_assign = ptr;
      consume(p);
      stmt->stmt_assign->expr = parse_expr(p, 0);
      stmt->type = STMT_ASSIGN;
      try_consume(p, semi, "Expected a ';'");
   }
   else if (peek(p, 0).type == println
         && peek(p, 1).type == open_paren) {
      NodeStmtPrint* ptr = arena_push(p->arena, NodeStmtPrint);
      consume(p);
      consume(p);
      ptr->expr = parse_expr(p, 0);
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
   puts(ANSI_COLOR_GREEN"[lexer]: Lexer stage finished successfully."ANSI_COLOR_RESET);
   return prog;
}
