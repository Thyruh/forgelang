#include <stdio.h>
#include <stdlib.h>

#include "../include/inttypes.h"
#include "./tokenizer.h"
#include "./parser.h"
#include "./generator.h"

int main(int argc, char** argv) {
   if (argc == 1) return 1; // TODO the default dialog explaining how to use the compiler

   (void)system("rm out.c");

   FILE* f = fopen(argv[argc-1], "r");
   fseek(f, 0, SEEK_END);
   u64 size = (u64)ftell(f);
   rewind(f);
   char* src = malloc(size);
   (void)fread(src, 1, size, f);

   Tokenizer tokenizer = Tokenizer_create(&src, size); // TODO perhaps arena allocate everything from this moment
   Tokens tokens = tokenize(&tokenizer);

   Parser parser = Parser_create(&tokens);
   NodeProg prog = parse_prog(&parser);

   FILE* out = fopen("out.c", "w+");

   Generator gen = Generator_create(&prog, out);
   gen_prog(&gen);

   free(src); // TODO carry everything onto that arena?
   free(parser.arena);
   for (size_t i = 0; i < tokens.size; i++) {
      if (strcmp(tokens.items[i].value, "") == 0) continue;
      free((void*)tokens.items[i].value);
   }
   free(tokens.items);
   free(prog.items);
   free(gen.table.items);
   fclose(out);
   fclose(f);
   return 0;
}
