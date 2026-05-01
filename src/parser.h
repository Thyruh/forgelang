#pragma once
#ifndef PARSER
#define PARSER
#include "./tokenizer.h"
typedef struct NodeExpr_ NodeExpr;

typedef struct {
  Token int_lit;
} NodeTermIntLit;

typedef struct {
  Token string_lit;
} NodeTermStringLit;

typedef struct {
  Token char_lit;
} NodeTermCharLit;

typedef struct {
  Token ident;
} NodeTermIdent;

typedef enum {
  EXPR_TERM,
  EXPR_BIN_EXPR,
} NodeExprType;

typedef enum {
  EXPR_ADD,
  EXPR_MULTI,
  EXPR_SUBTR,
  EXPR_DIVIDE
} BinExprType;

typedef struct {
  BinExprType type;
  NodeExpr* lhs;
  NodeExpr* rhs;
  Token op;
} NodeBinExpr;

typedef enum {
  TERM_INT_LIT,
  TERM_STRING_LIT,
  TERM_CHAR_LIT,
  TERM_IDENT,
} NodeTermType;

typedef struct {
  NodeTermType type;
  union {
    NodeTermIdent* ident;
    NodeTermIntLit* int_lit;
    NodeTermStringLit* string_lit;
    NodeTermCharLit* char_lit;
  } value;
} NodeTerm;

struct NodeExpr_ {
  NodeExprType type;
  union {
    NodeTerm* term;
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

/// The ident inside expr is the rhs value
typedef struct {
  NodeStmtType type;
  union {
    NodeStmtAssign* stmt_assign;
    NodeStmtExit* stmt_exit;
    NodeStmtLet* stmt_let;
    NodeStmtPrint* stmt_print;
  };
} NodeStmt;

typedef struct {
  NodeStmt* items;
  size_t size;
  size_t capacity;
} NodeProg;

typedef struct {
  const char* ident;
  TokenType type;
  bool mut;
} Symbol;

// TODO: symbol table - per-type partitioned dynamic arrays, separate variable and const sections
typedef struct {
  Symbol* items;
  size_t size;
  size_t capacity;
} SymbolTable;

typedef struct {
  Tokens* tokens;
  size_t index;
  mem_arena* arena;
  SymbolTable* table;
} Parser;

// TODO Add if-else statement nodes and codegen
// TODO Add for loop statement nodes and codegen
// TODO Add function declaration and call nodes and codegen

Parser Parser_create(Tokens* tokens, mem_arena* arena, SymbolTable* table);
NodeProg parse_prog(Parser* parser);
SymbolTable get_symbols(Parser* parser);
#endif // PARSER
