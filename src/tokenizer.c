#include "./tokenizer.h"
#include <ctype.h>

#ifdef DEBUG // the DEBUG macro is automatically defined with `./nob debug` through `-D`
#define DEBUG_TOKEN(ch) printf("[tokenizer]: token '%c' successfully added at %zu:%zu\n", (ch), t->pos.line, t->pos.line_pos)
#define DEBUG_KW(buf) printf("[tokenizer]: keyword %s added successfully at %zu:%zu\n", (buf), t->pos.line, t->pos.line_pos-strlen(tokens.items[tokens.size-1].value))
#define DEBUG_IDENT(buf) printf("[tokenizer]: ident \"%s\" added successfully at %zu:%zu\n", (buf), t->pos.line, t->pos.line_pos-strlen(tokens.items[tokens.size-1].value)+1)
#define DEBUG_INT(buf) printf("[tokenizer]: int literal \"%s\" added successfully at %zu:%zu\n", (buf), t->pos.line, t->pos.line_pos-strlen(tokens.items[tokens.size-1].value)+1)
#define DEBUG_STRING(buf) printf("[tokenizer]: string literal \"%s\" added successfully at %zu:%zu\n", (buf), t->pos.line, t->pos.line_pos-strlen(tokens.items[tokens.size-1].value))
#define DEBUG_TYPE(buf) printf("[tokenizer]: type \"%s\" added successfully at %zu:%zu\n", (buf), t->pos.line, t->pos.line_pos-strlen(tokens.items[tokens.size-1].value)+1)
#else
#define DEBUG_TOKEN(ch) ch // this need to be here for the token switch
#define DEBUG_KW(buf)
#define DEBUG_IDENT(buf)
#define DEBUG_INT(buf)
#define DEBUG_STRING(buf)
#define DEBUG_TYPE(buf)
#endif

Tokenizer Tokenizer_create(char** src, size_t length) {
   assert(length && "[ERROR]: empty source file");
   Tokenizer t = { 0 };
   t.length = length;
   t.src = *src;
   t.pos.line = 1;
   t.pos.line_pos = 0;
   return t;
}

__attribute__((warn_unused_result))
static inline char peek(Tokenizer* t) {
   if (t->index >= t->length) return '\0';
   return t->src[t->index];
}

static inline char consume(Tokenizer* t) {
   t->pos.line_pos++;
   return t->src[t->index++];
}

Tokens tokenize(Tokenizer* t) {
   Tokens tokens = { 0 };
   Buf buf = { 0 };

   while (t->index < t->length) {
      char ch = peek(t);
      if (ch == 0) return tokens;
      if (ch == '\n') {
         consume(t);
         t->pos.line++;
         t->pos.line_pos = 0;
         continue;
      }
      if (ch == ' ' || ch == '\t' || ch == '\r') {
         consume(t);
         continue;
      }
      if (isalpha(ch)) {
         da_append(&buf, consume(t));
         while (isalnum(peek(t))) {
            da_append(&buf, consume(t));
         }
         da_append(&buf, '\0');
         if (!strcmp(buf.items, "println")) { // duping the value to calculate its length in the DEBUG_ macros
            da_append(&tokens, ((Token){.type = println, .value = strdup(buf.items), .pos = t->pos}));
            DEBUG_KW(buf.items);
            da_clear(&buf);
         }
         else if (!strcmp(buf.items, "const")) {
            da_append(&tokens, ((Token){.type = const_, .value = strdup(buf.items), .pos = t->pos}));
            DEBUG_KW(buf.items);
            da_clear(&buf);
         }
         else if (!strcmp(buf.items, "mut")) {
            da_append(&tokens, ((Token){.type = mut, .value = strdup(buf.items), .pos = t->pos}));
            DEBUG_KW(buf.items);
            da_clear(&buf);
         }
         else if (!strcmp(buf.items, "exit")) {
            da_append(&tokens, ((Token){.type = exit_, .value = strdup(buf.items), .pos = t->pos}));
            DEBUG_KW(buf.items);
            da_clear(&buf);
         }
         else if (!strcmp(buf.items, "exit")) {
            da_append(&tokens, ((Token){.type = exit_, .value = strdup(buf.items), .pos = t->pos}));
            DEBUG_KW(buf.items);
            da_clear(&buf);
         }
         else if (!strcmp(buf.items, "char") || !strcmp(buf.items, "i8")) {
            da_append(&tokens, ((Token){.type = i8_, .value = strdup(buf.items), .pos = t->pos}));
            DEBUG_KW(buf.items);
            da_clear(&buf);
         }
         else if (!strcmp(buf.items, "uchar") || !strcmp(buf.items, "u8")) {
            da_append(&tokens, ((Token){.type = u8_, .value = strdup(buf.items), .pos = t->pos}));
            DEBUG_KW(buf.items);
            da_clear(&buf);
         }
         else if (!strcmp(buf.items, "i16")) {
            da_append(&tokens, ((Token){.type = i16_, .value = strdup(buf.items), .pos = t->pos}));
            DEBUG_KW(buf.items);
            da_clear(&buf);
         }
         else if (!strcmp(buf.items, "i32")) {
            da_append(&tokens, ((Token){.type = i32_, .value = strdup(buf.items), .pos = t->pos}));
            DEBUG_TYPE(buf.items);
            da_clear(&buf);
         }
         else if (!strcmp(buf.items, "i64")) {
            da_append(&tokens, ((Token){.type = i64_, .value = strdup(buf.items), .pos = t->pos}));
            DEBUG_TYPE(buf.items);
            da_clear(&buf);
         }
         else if (!strcmp(buf.items, "u32")) {
            da_append(&tokens, ((Token){.type = u32_, .value = strdup(buf.items), .pos = t->pos}));
            DEBUG_TYPE(buf.items);
            da_clear(&buf);
         }
         else if (!strcmp(buf.items, "u64")) {
            da_append(&tokens, ((Token){.type = u64_, .value = strdup(buf.items), .pos = t->pos}));
            DEBUG_TYPE(buf.items);
            da_clear(&buf);
         }
         else if (!strcmp(buf.items, "f32")) {
            da_append(&tokens, ((Token){.type = f32_, .value = strdup(buf.items), .pos = t->pos}));
            DEBUG_TYPE(buf.items);
            da_clear(&buf);
         }
         else if (!strcmp(buf.items, "f64")) {
            da_append(&tokens, ((Token){.type = f64_, .value = strdup(buf.items), .pos = t->pos}));
            DEBUG_TYPE(buf.items);
            da_clear(&buf);
         }
         else if (!strcmp(buf.items, "uptr")) {
            da_append(&tokens, ((Token){.type = uptr, .value = strdup(buf.items), .pos = t->pos}));
            DEBUG_TYPE(buf.items);
            da_clear(&buf);
         }
         else if (!strcmp(buf.items, "ptr")) {
            da_append(&tokens, ((Token){.type = ptr, .value = strdup(buf.items), .pos = t->pos}));
            DEBUG_TYPE(buf.items);
            da_clear(&buf);
         }
         else if (!strcmp(buf.items, "bool")) {
            da_append(&tokens, ((Token){.type = bool_, .value = strdup(buf.items), .pos = t->pos}));
            DEBUG_TYPE(buf.items);
            da_clear(&buf);
         }
         else if (!strcmp(buf.items, "string")) {
            da_append(&tokens, ((Token){.type = string, .value = strdup(buf.items), .pos = t->pos}));
            DEBUG_TYPE(buf.items);
            da_clear(&buf);
         }
         else if (!strcmp(buf.items, "ustring")) {
            da_append(&tokens, ((Token){.type = ustring, .value = strdup(buf.items), .pos = t->pos}));
            DEBUG_TYPE(buf.items);
            da_clear(&buf);
         }
         else {
            da_append(&tokens, ((Token){.type = ident, .value = strdup(buf.items), .pos = t->pos}));
            DEBUG_IDENT(buf.items);
            da_clear(&buf);
         }
         continue;
      }

      if (isdigit(ch)) {
         da_append(&buf, consume(t));
         while (isdigit(peek(t)))
            da_append(&buf, consume(t));
         da_append(&buf, '\0');
         da_append(&tokens, ((Token){.type = int_lit, .value = strdup(buf.items), .pos = t->pos}));
         DEBUG_INT(buf.items);
         da_clear(&buf);
         continue;
      }

      if (ch == '"') {
         consume(t);
         da_append(&buf, consume(t));
         while (peek(t) != '"') {
            da_append(&buf, consume(t));
         }
         consume(t);
         da_append(&buf, '\0');
         da_append(&tokens, ((Token){.type = string_lit, .value = strdup(buf.items), .pos = t->pos}));
         DEBUG_STRING(buf.items);
         da_clear(&buf);
         continue;
      }

      switch (ch) {
         case '+':
            DEBUG_TOKEN(consume(t));
            da_append(&tokens, ((Token){.type = plus, .value = "", .pos = t->pos}));
            break;
         case '-':
            DEBUG_TOKEN(consume(t));
            da_append(&tokens, ((Token){.type = minus, .value = "", .pos = t->pos}));
            break;
         case '*':
            DEBUG_TOKEN(consume(t));
            da_append(&tokens, ((Token){.type = star, .value = "", .pos = t->pos}));
            break;
         case '/':
            DEBUG_TOKEN(consume(t));
            da_append(&tokens, ((Token){.type = fslash, .value = "", .pos = t->pos}));
            break;
         case '=':
            DEBUG_TOKEN(consume(t));
            da_append(&tokens, ((Token){.type = equals, .value = "", .pos = t->pos}));
            break;
         case '(':
            DEBUG_TOKEN(consume(t));
            da_append(&tokens, ((Token){.type = open_paren, .value = "", .pos = t->pos}));
            break;
         case ')':
            DEBUG_TOKEN(consume(t));
            da_append(&tokens, ((Token){.type = close_paren, .value = "", .pos = t->pos}));
            break;
         case 39: // single quote
            DEBUG_TOKEN(consume(t));
            da_append(&tokens, ((Token){.type = s_quote, .value = "", .pos = t->pos}));
            break;
         // case '"':
         //    DEBUG_TOKEN(consume(t));
         //    da_append(&tokens, ((Token){.type = d_quote, .value = "", .pos = t->pos}));
         //    break;
         case ';':
            DEBUG_TOKEN(consume(t));
            da_append(&tokens, ((Token){.type = semi, .value = "", .pos = t->pos}));
            break;
         case ':':
            DEBUG_TOKEN(consume(t));
            da_append(&tokens, ((Token){.type = colon, .value = "", .pos = t->pos}));
            break;
         default:
            printf("Error: invalid character: '%c'.\n", consume(t));
            // exit(1);
      }
   }
   free(buf.items);
   return tokens;
} // tokenize()
