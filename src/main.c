#include <stdio.h>
#include <stdlib.h>

#include "../include/inttypes.h"
#include "./tokenizer.h"
#include "./parser.h"
#include "./generator.h"

typedef int Errno;

int main(int argc, char** argv) {
   if (argc < 1) return 1;

   FILE* f = fopen(argv[argc-1], "r");
   fseek(f, 0, SEEK_END);
   u64 size = ftell(f);
   rewind(f);
   char* src = malloc(size);
   fread(src, 1, size, f);

   Tokenizer tokenizer = Tokenizer_create(&src, size); // perhaps arena allocate that baby
   Tokens tokens = tokenize(&tokenizer);


   Parser parser = Parser_create(&tokens);
   NodeProg prog = parse_prog(&parser);

   FILE* out = fopen("out.c", "w+");

   Generator gen = Generator_create(&prog, out);
   gen_prog(&gen);

   // for (size_t i = 0; i < tokens.size; i++) {
   //    printf("[%zu] type=%d value=%s\n", i, tokens.items[i].type, tokens.items[i].value);
   // }

   fclose(out);
   fclose(f);
   return 0;
}
