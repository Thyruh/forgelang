#include <stdio.h>
#include <stdlib.h>

#include "./common/inttypes.h"
#include "./common/tokenhandlers.h"
#include "./generator.h"
#include "./parser.h"
#include "./tokenizer.h"
#include "./typechecker.h"

// TODO: argc check - if source file doesn't exist fopen returns NULL and fseek segfaults
// TODO: --loose flag - disable full move/borrow checking
// TODO: --freestanding flag - strip all libc and forgelang std dependencies

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
    const char* s = token_repr(tokens->items[i]);
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

char* read_source(char* filename, u64* m_size, mem_arena* arena) {
  FILE* f = fopen(filename, "r");
  if (!f) {
    printf(ANSI_COLOR_RED"Error: No file or directory `%s` found.\n"ANSI_COLOR_RESET, filename);
    goto exit;
  }
  fseek(f, 0, SEEK_END);
  u64 size = (u64)ftell(f);
  if (size == 0) {
    printf(ANSI_COLOR_RED"Error: The translation unit `%s` is empty\n"ANSI_COLOR_RESET, filename);
    goto exit;
  }
  rewind(f);
  char* src = arena_push_arr(arena, char, size);
  size_t size_check = fread(src, 1, size, f);
  if (size_check != size) {
    if (ferror(f)) {
      printf(ANSI_COLOR_RED"Error reading file"ANSI_COLOR_RESET);
      goto exit;
    } else if (feof(f)) {
      printf(ANSI_COLOR_RED"Error: File ended unexpectedly (size changed?)\n"ANSI_COLOR_RESET);
      goto exit;
    } else {
      printf(ANSI_COLOR_RED"Error: Unknown fread issue\n"ANSI_COLOR_RESET);
      goto exit;
    }
  }
  *m_size = size;
  fclose(f);
  return src;
exit:
  arena_free(arena);
  exit(1);
}

int main(int argc, char** argv) {
  if (argc == 1) {
    printf(ANSI_COLOR_RED"Error: At least one argument required\n");
    printf("\tCorrect syntax: `forge <filename>.fg`.\n"ANSI_COLOR_RESET);
    return 1;
  }

  remove("out.c");
  remove("out");

  mem_arena* arena = arena_create(KiB(4));

  u64 size;
  char* src = read_source(argv[1], &size, arena);

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
  (void)system("cc -O3 -oout out.c");

  arena_free(arena);
  // Those allocations are quite immovable to an arena cause they are resized like a billion times
  free(table.items);
  free(tokens.items);
  free(prog.items);
  return 0;
}
