#pragma once
#ifndef PARSER
#define PARSER
#include "./tokenizer.h"
#define ARENA_IMPLEMENTATION
#define ARENA_VOID_RESPONSE
#include "./arena.h"
struct NodeExpr_;
typedef struct NodeExpr_ NodeExpr;

typedef struct {
   Token int_lit;
} NodeExprIntLit;

typedef struct {
   Token ident;
} NodeExprIdent; 

typedef enum {
   EXPR_INT_LIT,
   EXPR_IDENT,
} NodeExprType;

typedef struct {
   NodeExpr* lhs;
   NodeExpr* rhs;
} BinExprAdd;

typedef struct {
   NodeExpr* lhs;
   NodeExpr* rhs;
} BinExprMulti;

typedef enum {
   EXPR_ADD,
   EXPR_MULTI
} BinExprType;

typedef struct {
   BinExprType type;
   union {
      BinExprAdd* add;
      BinExprMulti* multi;
   } var;
} BinExpr;

struct NodeExpr_ {
   NodeExprType type;
   union {
      NodeExprIntLit* int_lit;
      NodeExprIdent*  ident;
      BinExpr* bin_expr;
   } value;
};

typedef struct {
   NodeExpr* expr;
} NodeStmtExit; 

typedef struct {
   Token ident;
   NodeExpr* expr;
} NodeStmtLet;

typedef enum {
   STMT_EXIT,
   STMT_LET,
   STMT_ASSIGN
} NodeStmtType;

typedef struct {
   Token ident;
   NodeExpr* expr;
} NodeStmtAssign;

typedef struct {
   NodeStmtType type;
   union {
      NodeStmtAssign* stmt_assign;
      NodeStmtExit* stmt_exit;
      NodeStmtLet*  stmt_let;
   };
} NodeStmt;

typedef struct {
   NodeStmt* items;
   size_t size;
   size_t capacity;
} NodeProg; 

typedef struct {
   Tokens* tokens;
   size_t index;
   mem_arena* arena;
} Parser;

Parser Parser_create(Tokens* tokens);
NodeProg parse_prog(Parser* parser);
#endif // PARSER
