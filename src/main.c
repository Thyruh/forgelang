#include <stdio.h>
#include <stdlib.h>

#include "../include/inttypes.h"
#include "./tokenizer.h"
#include "./parser.h"
#include "./generator.h"

static inline const char* tokentype_str(TokenType t) {
    switch (t) {
        case semi:          return "semi";
        case colon:         return "colon";
        case plus:          return "plus";
        case minus:         return "minus";
        case star:          return "star";
        case fslash:        return "fslash";
        case equals:        return "equals";
        case open_paren:    return "open_paren";
        case close_paren:   return "close_paren";
        case open_brace:    return "open_brace";
        case close_brace:   return "close_brace";
        case print:         return "print";
        case println:       return "println";
        case write:         return "write";
        case writeln:       return "writeln";
        case const_:        return "const";
        case mut:           return "mut";
        case ident:         return "ident";
        case return_:       return "return";
        case exit_:         return "exit";
        case int_lit:       return "int_lit";
        case string_lit:    return "string_lit";
        case i8_:           return "i8";
        case i16_:          return "i16";
        case i32_:          return "i32";
        case i64_:          return "i64";
        case u8_:           return "u8";
        case u16_:          return "u16";
        case u32_:          return "u32";
        case u64_:          return "u64";
        case f32_:          return "f32";
        case f64_:          return "f64";
        case bool_:         return "bool";
        case string:        return "string";
        case ustring:       return "ustring";
        case TERMINATE:     return "TERMINATE";
        default:            return "unknown";
    }
}

static inline void debug_tokens(Tokens* tokens) {
    for (size_t i = 0; i < tokens->size; i++) {
        Token t = tokens->items[i];
        printf("[%zu] %-15s value=%-20s pos=%zu:%zu\n",
            i, tokentype_str(t.type), t.value, t.pos.line, t.pos.line_pos-strlen(tokens->items[i].value));
    }
}

int main(int argc, char** argv) {
   if (argc == 1) return 1; // TODO the default dialog explaining how to use the compiler

   (void)system("rm out.c out");

   FILE* f = fopen(argv[argc-1], "r");
   fseek(f, 0, SEEK_END);
   u64 size = (u64)ftell(f);
   rewind(f);
   char* src = malloc(size);
   (void)fread(src, 1, size, f);

   Tokenizer tokenizer = Tokenizer_create(&src, size); // TODO perhaps arena allocate everything from this moment
   Tokens tokens = tokenize(&tokenizer);

   debug_tokens(&tokens);

   Parser parser = Parser_create(&tokens);
   NodeProg prog = parse_prog(&parser);

   FILE* out = fopen("out.c", "w+");

   Generator gen = Generator_create(&prog, out);
   gen_prog(&gen);


   free(src); // TODO carry everything onto that arena?
   free(parser.arena);
   for (size_t i = 0; i < tokens.size; i++) {
      if (!strcmp(tokens.items[i].value, "")) continue;
      free((void*)tokens.items[i].value);
   }
   free(tokens.items);
   free(prog.items);
   free(gen.table.items);
   fclose(out);
   fclose(f);
   (void)system("cc -oout -Wall -Wextra -pedantic out.c");
   return 0;
}
