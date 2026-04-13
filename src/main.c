#include <stdio.h>
#include <stdlib.h>

// TODO The errors are really nice, but recovery is even nicer

#include "../include/inttypes.h"
#include "./tokenizer.h"
#include "./parser.h"
#include "./generator.h"

// TODO: argc check — if source file doesn't exist fopen returns NULL and fseek segfaults
// TODO: --loose flag — disable full move/borrow checking
// TODO: --freestanding flag — strip all libc and forgelang std dependencies

int main(int argc, char** argv) {
   if (argc == 1) return 1; // TODO the default dialog explaining how to use the compiler

   (void)system("rm out.c out");

   mem_arena* arena = arena_create(MiB(4));
   FILE* f = fopen(argv[argc-1], "r");
   fseek(f, 0, SEEK_END);
   u64 size = (u64)ftell(f);
   rewind(f);
   char* src = arena_push_arr(arena, char, size);
   (void)fread(src, 1, size, f);

   Tokenizer tokenizer = Tokenizer_create(&src, size, arena);
   Tokens tokens = tokenize(&tokenizer);

   Parser parser = Parser_create(&tokens, arena);
   NodeProg prog = parse_prog(&parser);

   FILE* out = fopen("out.c", "w+");

   Generator gen = Generator_create(&prog, out);
   gen_prog(&gen);

   fclose(out);
   fclose(f);
   (void)system("cc -oout out.c");

   arena_free(arena);
   free(tokens.items); // Those allocations are quite immovable to an arena cause they depend on da_append, which I dont want to touch for now
   free(prog.items);
   free(gen.table.items);
   return 0;
}
