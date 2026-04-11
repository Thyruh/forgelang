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
} NodeTermIntLit;

typedef struct {
   Token string_lit;
} NodeTermStringLit;

typedef struct {
   Token ident;
} NodeTermIdent; 

typedef enum {
   EXPR_TERM,
   EXPR_BIN_EXPR,
} NodeExprType;

typedef struct {
   NodeExpr* lhs;
   NodeExpr* rhs;
} NodeBinExprAdd;

typedef struct {
   NodeExpr* lhs;
   NodeExpr* rhs;
} NodeBinExprMulti;

typedef enum {
   EXPR_ADD,
   EXPR_MULTI,
} BinExprType;

typedef struct {
   BinExprType type;
   union {
      NodeBinExprAdd* add;
      NodeBinExprMulti* multi;
   } var;
} NodeBinExpr;

typedef enum {
   TERM_INT_LIT,
   TERM_STRING_LIT,
   TERM_IDENT,
} NodeTermType;

typedef struct {
   NodeTermType type;
   union {
      NodeTermIdent* ident;
      NodeTermIntLit* int_lit;
      NodeTermStringLit* string_lit;
   } value;
} NodeTerm;

struct NodeExpr_ {
   NodeExprType type;
   union {
      NodeTerm*    term;
      NodeBinExpr* bin_expr;
   } value;
};

typedef struct {
   NodeExpr* expr;
} NodeStmtExit; 

typedef struct {
   NodeExpr* expr;
} NodeStmtPrint;

typedef struct {
   Token ident;
   NodeExpr* expr;
   TokenType type;
   bool mut;
} NodeStmtLet;

typedef struct {
   Token ident;
   NodeExpr* expr;
} NodeStmtAssign;

typedef enum {
   STMT_EXIT,
   STMT_LET,
   STMT_PRINTLN,
   STMT_ASSIGN
} NodeStmtType;

typedef struct {
   NodeStmtType type;
   union {
      NodeStmtAssign* stmt_assign;
      NodeStmtExit* stmt_exit;
      NodeStmtLet*  stmt_let;
      NodeStmtPrint*  stmt_print;
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
__attribute__((warn_unused_result)) static inline NodeBinExpr* parse_bin_expr(Parser* p);
#endif // PARSER
