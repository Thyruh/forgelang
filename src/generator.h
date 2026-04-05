#pragma once
#ifndef GENERATOR
#define GENERATOR
#include "./parser.h"

typedef struct {
   char* ident;
   char* value;
} Symbol;

typedef struct {
   Symbol* items;
   size_t size;
   size_t capacity;
} SymbolTable;

typedef struct {
    SymbolTable table;
    NodeProg* prog;
    FILE* out;
} Generator;

void gen_prog(Generator* gen);
Generator Generator_create(NodeProg* prog, FILE* out);
#endif // GENERATOR
