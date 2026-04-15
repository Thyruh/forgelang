#pragma once
#ifndef GENERATOR
#define GENERATOR
#include "./parser.h"

// TODO: symbol table — per-type partitioned dynamic arrays, separate variable and const sections
typedef struct {
   const char* ident;
   Token token;
   TokenType type; // ident_type
   bool mut;
} Symbol;

typedef struct {
   Symbol* items;
   size_t size;
   size_t capacity;
} SymbolTable;

typedef struct {
    SymbolTable table;
    NodeProg* stmts;
    FILE* out;
} Generator;

void gen_prog(Generator* gen);
Generator Generator_create(NodeProg* prog, FILE* out);
#endif // GENERATOR
