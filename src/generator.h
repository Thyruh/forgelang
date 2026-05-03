#pragma once
#ifndef GENERATOR
#define GENERATOR
#include "./parser.h"

typedef struct {
  NodeProg* stmts;
  FILE* out;
} Generator;

void gen_prog(Generator* gen);
Generator Generator_create(NodeProg* prog, FILE* out);
#endif // GENERATOR
