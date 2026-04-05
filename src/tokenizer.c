#include "./tokenizer.h"

Tokenizer Tokenizer_create(char** src, size_t length) {
   assert(length && "[ERROR]: empty source file");
   Tokenizer t = { 0 };
   t.length = length;
   t.src = *src;
   return t;
}

static inline char peek(Tokenizer* t) {
   if (t->index >= t->length) return '\0';
   return t->src[t->index];
}

static inline char consume(Tokenizer* t) {
   return t->src[t->index++];
}

Tokens tokenize(Tokenizer* t) {
   Tokens tokens = { 0 };
   Buf buf = { 0 };

   while (t->index < t->length) {
      char ch = peek(t);
      if (ch == 0) return tokens;
      if (ch == ' ' || ch == '\n' || ch == '\t' || ch == '\r') {
         consume(t);
         continue;
      }
      if (isalpha(ch)) {
         da_append(&buf, consume(t));
         while (isalnum(peek(t))) {
            da_append(&buf, consume(t));
         }

         if (!strcmp(buf.items, "print")) {
            da_append(&tokens, ((Token){print, ""}));
            printf("keyword %s added successfully\n", buf.items);
            da_clear(&buf);
         }
         else if (!strcmp(buf.items, "let")) {
            da_append(&tokens, ((Token){let, ""}));
            printf("keyword %s added successfully\n", buf.items);
            da_clear(&buf);
         }
         else if (!strcmp(buf.items, "exit")) {
            da_append(&tokens, ((Token){exit_, ""}));
            printf("keyword %s added successfully\n", buf.items);
            da_clear(&buf);
         }
         else {
            da_append(&tokens, ((Token){ident, strdup(buf.items)}));
            printf("ident \"%s\" added successfully\n", buf.items);
            da_clear(&buf);
         }
         continue;
      }

      if (isdigit(ch)) {
         da_append(&buf, consume(t));
         while (isdigit(peek(t)))
            da_append(&buf, consume(t));
         da_append(&tokens, ((Token){int_lit, strdup(buf.items)}));
         printf("int literal \"%s\" added successfully\n", buf.items);
         da_clear(&buf);
         continue;
      }

      switch (ch) {
         case '+':
            // consume(t);
            printf("token '%c' successfully added\n", consume(t));
            da_append(&tokens, ((Token){plus, ""}));
            break;
         case '-':
            // consume(t);
            printf("token '%c' successfully added\n", consume(t));
            da_append(&tokens, ((Token){minus, ""}));
            break;
         case '*':
            // consume(t);
            printf("token '%c' successfully added\n", consume(t));
            da_append(&tokens, ((Token){times, ""}));
            break;
         case '/':
            // consume(t);
            printf("token '%c' successfully added\n", consume(t));
            da_append(&tokens, ((Token){slash, ""}));
            break;
         case '=':
            // consume(t);
            printf("token '%c' successfully added\n", consume(t));
            da_append(&tokens, ((Token){equals, ""}));
            break;
         case '(':
            // consume(t);
            printf("token '%c' successfully added\n", consume(t));
            da_append(&tokens, ((Token){open_paren, ""}));
            break;
         case ')':
            // consume(t);
            printf("token '%c' successfully added\n", consume(t));
            da_append(&tokens, ((Token){close_paren, ""}));
            break;
         case 39: // single quote
            // consume(t);
            printf("token '%c' successfully added\n", consume(t));
            da_append(&tokens, ((Token){s_quote, ""}));
            break;
         case '"': // double quote
            // consume(t);
            printf("token '%c' successfully added\n", consume(t));
            da_append(&tokens, ((Token){d_quote, ""}));
            break;
         case ';':
            // consume(t);
            printf("token '%c' successfully added\n", consume(t));
            da_append(&tokens, ((Token){semi, ""}));
            break;
         default:
            printf("Error: invalid character: '%c'\n", consume(t));
            // exit(1);
      }
   }
   return tokens;
} // tokenize()
