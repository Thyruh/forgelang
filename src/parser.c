#include "./parser.h"

static inline Token peek(Parser* parser, size_t offset) {
   if (parser->index+offset >= parser->tokens->size ) return (Token){TERMINATE, ""};
   return parser->tokens->items[parser->index+offset];
}

static inline Token consume(Parser* parser) {
   if (parser->index >= parser->tokens->size) {
      printf("[ERROR]: Unexpected end of tokens");
      exit(1);
   }
   return parser->tokens->items[parser->index++];
}

Parser Parser_create(Tokens* tokens) {
   Parser p = { 0 };
   p.tokens = tokens;
   return p;
}

NodeExpr parse_expr(Parser* parser) {
   NodeExpr expr = { 0 };

   if (peek(parser, 0).type == int_lit) {
      expr.type = EXPR_INT_LIT;
      expr.value.int_lit = (NodeExprIntLit){consume(parser)};
      return expr;
   }
   else {
      expr.type = EXPR_IDENT;
      expr.value.ident = (NodeExprIdent){consume(parser)};
   }

   return expr;
}

NodeStmt parse_stmt(Parser* parser) {
   NodeStmt stmt = { 0 };
   if (peek(parser, 0).type == let 
    && peek(parser, 1).type == ident 
    && peek(parser, 2).type == equals) {
      consume(parser);
      stmt.stmt_let.ident = consume(parser);
      consume(parser);
      stmt.stmt_let.expr = parse_expr(parser);
      stmt.type = STMT_LET; 
      if (peek(parser, 0).type != semi) {
         printf("Missing a semicolon\n");
         exit(1);
      }
      consume(parser);
   }
   else if (peek(parser, 0).type == exit_
         && peek(parser, 1).type == open_paren) {
      consume(parser);
      consume(parser);
      stmt.stmt_exit.expr = parse_expr(parser);
      stmt.type = STMT_EXIT; 
      if (peek(parser, 0).type == close_paren) {
         consume(parser);
      }
      else {
         printf("Missing a paren\n");
         exit(1);
      }
      if (peek(parser, 0).type == semi) {
         consume(parser);
      }
      else {
         printf("Missing a semicolon\n");
         exit(1);
      }
   }
   else {
      printf("Invalid expression");
   }
   return stmt;
}

NodeProg parse_prog(Parser* parser) {
   NodeProg prog = { 0 };
   while (peek(parser, 0).type != TERMINATE) {
      NodeStmt stmt = parse_stmt(parser);
      da_append(&prog, stmt);
   }
   return prog;
}
