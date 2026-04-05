#pragma once
#ifndef PARSER
#define PARSER
#include "./tokenizer.h"

typedef struct {
   Token intLit;
} NodeExprIntLit;

typedef struct {
   Token ident;
} NodeExprIdent; 

typedef enum {
    EXPR_INT_LIT,
    EXPR_IDENT,
} NodeExprType;

typedef struct {
    NodeExprType type;
    union {
        NodeExprIntLit int_lit;
        NodeExprIdent  ident;
    } value;
} NodeExpr;

typedef struct {
   NodeExpr expr;
} NodeStmtExit; 

typedef struct {
   Token ident;
   NodeExpr expr;
} NodeStmtLet;

typedef enum {
    STMT_EXIT,
    STMT_LET,
} NodeStmtType;

typedef struct {
    NodeStmtType type;
    union {
        NodeStmtExit stmt_exit;
        NodeStmtLet  stmt_let;
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
} Parser;

Parser Parser_create(Tokens* tokens);
NodeProg parse_prog(Parser* parser);
#endif // PARSER
