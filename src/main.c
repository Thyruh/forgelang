#include <stdio.h>
#include <stdlib.h>

#include "./common/inttypes.h"
#include "./common/tokenhandlers.h"
#include "./generator.h"
#include "./parser.h"
#include "./tokenizer.h"
#include "./typechecker.h"

// TODO: argc check — if source file doesn't exist fopen returns NULL and fseek segfaults
// TODO: --loose flag — disable full move/borrow checking
// TODO: --freestanding flag — strip all libc and forgelang std dependencies

#ifndef DEBUG
void print_tokens_as_strings(Tokens* tokens) {
  (void)tokens;
  return;
}
#endif
#ifdef DEBUG
void print_tokens_as_strings(Tokens* tokens) {
  puts(ANSI_COLOR_MAGENTA "\n```");
  for (size_t i = 0; i < tokens->size; i++) {
    const char *s = token_repr(tokens->items[i]);
    if (tokens->items[i].type == char_lit) {
      printf("'%s' ", s);
    } else if (tokens->items[i].type == string_lit) {
      printf("\"%s\" ", s);
    } else if (s[0] != ';') {
      printf("%s ", s);
    } else
      puts(";");
  }
  puts("```\n" ANSI_COLOR_RESET);
}
#endif

int main(int argc, char** argv) {
  if (argc == 1)
    return 1; // TODO the default dialog explaining how to use the compiler

  (void)system("rm out.c out");

  mem_arena* arena = arena_create(KiB(4));
  FILE* f = fopen(argv[argc - 1], "r");
  fseek(f, 0, SEEK_END);
  u64 size = (u64)ftell(f);
  rewind(f);
  char* src = arena_push_arr(arena, char, size);
  (void)fread(src, 1, size, f);

  Tokenizer tokenizer = Tokenizer_create(&src, size, arena);
  Tokens tokens = tokenize(&tokenizer);

  print_tokens_as_strings(&tokens);

  SymbolTable table = {0};
  Parser parser = Parser_create(&tokens, arena, &table);
  NodeProg prog = parse_prog(&parser);

  typecheck_prog(&table, &prog, &tokens);

  FILE* out = fopen("out.c", "w+");
  Generator gen = Generator_create(&prog, out);
  gen_prog(&gen);

  fclose(out);
  fclose(f);
  (void)system("cc -O3 -oout out.c");

  arena_free(arena);
  free(table.items);
  free(tokens.items); // Those allocations are quite immovable to an arena cause they depend on da_append, which I dont want to touch for now
  free(prog.items);
  return 0;
}
