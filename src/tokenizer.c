#include "./tokenizer.h"
#include <ctype.h>

const char* tokentype_str(TokenType t) {
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
      case open_bracket:  return "open_bracket";
      case close_bracket: return "close_bracket";
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
      case char_lit:      return "char_lit";
      case i8_:           return "i8";
      case char_:         return "char";
      case i16_:          return "i16";
      case i32_:          return "i32";
      case i64_:          return "i64";
      case u8_:           return "u8";
      case uchar_:        return "uchar";
      case u16_:          return "u16";
      case u32_:          return "u32";
      case u64_:          return "u64";
      case f32_:          return "f32";
      case f64_:          return "f64";
      case bool_:         return "bool";
      case string:        return "string";
      case TERMINATE:     return "TERMINATE";
      default:            return "unknown";
   }
}

const char* token_repr(Token t) {
    switch (t.type) {
        case int_lit:
        case string_lit:
        case char_lit:
        case func_lit:
        case ident:       return t.value;
        // keywords
        case const_:      return "const";
        case mut:         return "mut";
        case print:       return "print";
        case println:     return "println";
        case write:       return "write";
        case writeln:     return "writeln";
        case return_:     return "return";
        case exit_:       return "exit";
        // types
        case char_:       return "char";
        case uchar_:      return "uchar";
        case i8_:         return "i8";
        case u8_:         return "u8";
        case i16_:        return "i16";
        case i32_:        return "i32";
        case i64_:        return "i64";
        case u16_:        return "u16";
        case u32_:        return "u32";
        case u64_:        return "u64";
        case f32_:        return "f32";
        case f64_:        return "f64";
        case bool_:       return "bool";
        case string:      return "string";
        case uptr:        return "uptr";
        case ptr:         return "ptr";
        // single char symbols
        case semi:        return ";";
        case colon:       return ":";
        case plus:        return "+";
        case minus:       return "-";
        case star:        return "*";
        case fslash:      return "/";
        case bslash:      return "\\";
        case open_paren:  return "(";
        case close_paren: return ")";
        case open_brace:  return "{";
        case close_brace: return "}";
        case open_bracket: return "[";
        case close_bracket: return "]";
        case equals:      return "=";
        case ampersand:   return "&";
        case comma:       return ",";
        case dot:         return ".";
        case caret:       return "^";
        case pipe:        return "|";
        case exclam:      return "!";
        case greater:     return ">";
        case less:        return "<";
        case grave:        return "`";
        case tilda:        return "~";
        case TERMINATE:   return "EOF";
    }
}

Tokenizer Tokenizer_create(char** src, size_t length, mem_arena* arena) {
   assert(length && "[ERROR]: empty source file");
   Tokenizer t = { 0 };
   t.length = length;
   t.src = *src;
   t.arena = arena;
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

      if (isalpha(ch) || ch == '_') {
         size_t start = t->pos.line_pos;
         da_append(&buf, consume(t));
         while (isalnum(peek(t)) || peek(t) == '_') {
            da_append(&buf, consume(t));
         }
         da_append(&buf, '\0');
         TokenPos start_pos = { t->pos.line, start};
         if (!strcmp(buf.items, "println")) {
            da_append(&tokens, ((Token){.type = println, .value = "", .pos = start_pos}));
            da_clear(&buf);
         }
         else if (!strcmp(buf.items, "const")) {
            da_append(&tokens, ((Token){.type = const_, .value = "", .pos = start_pos}));
            da_clear(&buf);
         }
         else if (!strcmp(buf.items, "mut")) {
            da_append(&tokens, ((Token){.type = mut, .value = "", .pos = start_pos}));
            da_clear(&buf);
         }
         else if (!strcmp(buf.items, "exit")) {
            da_append(&tokens, ((Token){.type = exit_, .value = "", .pos = start_pos}));
            da_clear(&buf);
         }
         else if (!strcmp(buf.items, "i8")) {
            da_append(&tokens, ((Token){.type = i8_, .value = "", .pos = start_pos}));
            da_clear(&buf);
         }
         else if (!strcmp(buf.items, "i16")) {
            da_append(&tokens, ((Token){.type = i16_, .value = "", .pos = start_pos}));
            da_clear(&buf);
         }
         else if (!strcmp(buf.items, "i32")) {
            da_append(&tokens, ((Token){.type = i32_, .value = "", .pos = start_pos}));
            da_clear(&buf);
         }
         else if (!strcmp(buf.items, "i64")) {
            da_append(&tokens, ((Token){.type = i64_, .value = "", .pos = start_pos}));
            da_clear(&buf);
         }
         else if (!strcmp(buf.items, "u8")) {
            da_append(&tokens, ((Token){.type = u8_, .value = "", .pos = start_pos}));
            da_clear(&buf);
         }
         else if (!strcmp(buf.items, "u16")) {
            da_append(&tokens, ((Token){.type = u16_, .value = "", .pos = start_pos}));
            da_clear(&buf);
         }
         else if (!strcmp(buf.items, "u32")) {
            da_append(&tokens, ((Token){.type = u32_, .value = "", .pos = start_pos}));
            da_clear(&buf);
         }
         else if (!strcmp(buf.items, "u64")) {
            da_append(&tokens, ((Token){.type = u64_, .value = "", .pos = start_pos}));
            da_clear(&buf);
         }
         else if (!strcmp(buf.items, "f32")) {
            da_append(&tokens, ((Token){.type = f32_, .value = "", .pos = start_pos}));
            da_clear(&buf);
         }
         else if (!strcmp(buf.items, "f64")) {
            da_append(&tokens, ((Token){.type = f64_, .value = "", .pos = start_pos}));
            da_clear(&buf);
         }
         else if (!strcmp(buf.items, "ptr")) {
            da_append(&tokens, ((Token){.type = ptr, .value = "", .pos = start_pos}));
            da_clear(&buf);
         }
         else if (!strcmp(buf.items, "uptr")) {
            da_append(&tokens, ((Token){.type = uptr, .value = "", .pos = start_pos}));
            da_clear(&buf);
         }
         else if (!strcmp(buf.items, "bool")) {
            da_append(&tokens, ((Token){.type = bool_, .value = "", .pos = start_pos}));
            da_clear(&buf);
         }
         else if (!strcmp(buf.items, "string")) {
            da_append(&tokens, ((Token){.type = string, .value = "", .pos = start_pos}));
            da_clear(&buf);
         }
         else if (!strcmp(buf.items, "char")) {
            da_append(&tokens, ((Token){.type = char_, .value = "", .pos = start_pos}));
            da_clear(&buf);
         }
         else if (!strcmp(buf.items, "uchar")) {
            da_append(&tokens, ((Token){.type = uchar_, .value = "", .pos = start_pos}));
            da_clear(&buf);
         }
         else {
            char* name = arena_push_arr(t->arena, char, buf.size);
            memcpy(name, buf.items, buf.size);
            da_append(&tokens, ((Token){.type = ident, .value = name, .pos = start_pos}));
            da_clear(&buf);
         }
         continue;
      }

      if (isdigit(ch)) {
         size_t start = t->pos.line_pos;
         da_append(&buf, consume(t));
         while (isdigit(peek(t)))
            da_append(&buf, consume(t));
         da_append(&buf, '\0');
         TokenPos start_pos = { t->pos.line, start};
         char* name = arena_push_arr(t->arena, char, buf.size);
         memcpy(name, buf.items, buf.size);
         da_append(&tokens, ((Token){.type = int_lit, .value = name, .pos = start_pos}));
         da_clear(&buf);
         continue;
      }

      if (ch == '"') { // TODO no closing " case
         consume(t);
         size_t start = t->pos.line_pos;
         da_append(&buf, consume(t));
         while (peek(t) != '"')
            da_append(&buf, consume(t));
         consume(t);
         da_append(&buf, '\0');
         TokenPos start_pos = { t->pos.line, start};
         char* name = arena_push_arr(t->arena, char, buf.size);
         memcpy(name, buf.items, buf.size);
         da_append(&tokens, ((Token){.type = string_lit, .value = name, .pos = start_pos}));
         da_clear(&buf);
         continue;
      }

      switch (ch) {
         case 39: // single quote
            {
               size_t start = t->pos.line_pos;
               TokenPos p = { t->pos.line, start };
               consume(t);
               da_clear(&buf);
               da_append(&buf, consume(t));
               consume(t);
               char* name = arena_push_arr(t->arena, char, 2);
               da_append(&buf, '\0');
               memcpy(name, buf.items, 2);
               da_append(&tokens, ((Token){.type = char_lit, .value = name, .pos = p}));
               da_clear(&buf);
            }
            break;
         case '+': 
            {
               size_t start = t->pos.line_pos;
               consume(t);
               TokenPos p = { t->pos.line, start };
               da_append(&tokens, ((Token){.type = plus, .value = "", .pos = p}));
            }
            break;
         case '-':
            {
               size_t start = t->pos.line_pos;
               consume(t);
               TokenPos p = { t->pos.line, start };
               da_append(&tokens, ((Token){.type = minus, .value = "", .pos = p}));
            }
            break;
         case '*':
            {
               size_t start = t->pos.line_pos;
               consume(t);
               TokenPos p = { t->pos.line, start };
               da_append(&tokens, ((Token){.type = star, .value = "", .pos = p}));
            }
            break;
         case '/':
            {
               size_t start = t->pos.line_pos;
               consume(t);
               if (peek(t) == '/') {
                  consume(t);
                  while (peek(t) != '\n') {
                     consume(t);
                  }
                  break;
               }
               TokenPos p = { t->pos.line, start };
               da_append(&tokens, ((Token){.type = fslash, .value = "", .pos = p}));
            }
            break;
         case '\\':
            {
               size_t start = t->pos.line_pos;
               consume(t);
               TokenPos p = { t->pos.line, start };
               da_append(&tokens, ((Token){.type = bslash, .value = "", .pos = p}));
            }
            break;
         case '{':
            {
               size_t start = t->pos.line_pos;
               consume(t);
               TokenPos p = { t->pos.line, start };
               da_append(&tokens, ((Token){.type = open_brace, .value = "", .pos = p}));
            }
            break;
         case '}':
            {
               size_t start = t->pos.line_pos;
               consume(t);
               TokenPos p = { t->pos.line, start };
               da_append(&tokens, ((Token){.type = close_brace, .value = "", .pos = p}));
            }
            break;
         case '[':
            {
               size_t start = t->pos.line_pos;
               consume(t);
               TokenPos p = { t->pos.line, start };
               da_append(&tokens, ((Token){.type = open_bracket, .value = "", .pos = p}));
            }
            break;
         case ']':
            {
               size_t start = t->pos.line_pos;
               consume(t);
               TokenPos p = { t->pos.line, start };
               da_append(&tokens, ((Token){.type = close_bracket, .value = "", .pos = p}));
            }
            break;
         case '&':
            {
               size_t start = t->pos.line_pos;
               consume(t);
               TokenPos p = { t->pos.line, start };
               da_append(&tokens, ((Token){.type = ampersand, .value = "", .pos = p}));
            }
            break;
         case ',':
            {
               size_t start = t->pos.line_pos;
               consume(t);
               TokenPos p = { t->pos.line, start };
               da_append(&tokens, ((Token){.type = comma, .value = "", .pos = p}));
            }
            break;
         case '.':
            {
               size_t start = t->pos.line_pos;
               consume(t);
               TokenPos p = { t->pos.line, start };
               da_append(&tokens, ((Token){.type = dot, .value = "", .pos = p}));
            }
            break;
         case '^':
            {
               size_t start = t->pos.line_pos;
               consume(t);
               TokenPos p = { t->pos.line, start };
               da_append(&tokens, ((Token){.type = caret, .value = "", .pos = p}));
            }
            break;
         case '|':
            {
               size_t start = t->pos.line_pos;
               consume(t);
               TokenPos p = { t->pos.line, start };
               da_append(&tokens, ((Token){.type = pipe, .value = "", .pos = p}));
            }
            break;
         case '!':
            {
               size_t start = t->pos.line_pos;
               consume(t);
               TokenPos p = { t->pos.line, start };
               da_append(&tokens, ((Token){.type = exclam, .value = "", .pos = p}));
            }
            break;
         case '>':
            {
               size_t start = t->pos.line_pos;
               consume(t);
               TokenPos p = { t->pos.line, start };
               da_append(&tokens, ((Token){.type = greater, .value = "", .pos = p}));
            }
            break;
         case '<':
            {
               size_t start = t->pos.line_pos;
               consume(t);
               TokenPos p = { t->pos.line, start };
               da_append(&tokens, ((Token){.type = less, .value = "", .pos = p}));
            }
            break;
         case '`':
            {
               size_t start = t->pos.line_pos;
               consume(t);
               TokenPos p = { t->pos.line, start };
               da_append(&tokens, ((Token){.type = grave, .value = "", .pos = p}));
            }
            break;
         case '~':
            {
               size_t start = t->pos.line_pos;
               consume(t);
               TokenPos p = { t->pos.line, start };
               da_append(&tokens, ((Token){.type = tilda, .value = "", .pos = p}));
            }
            break;
         case '=':
            {
               size_t start = t->pos.line_pos;
               consume(t);
               TokenPos p = { t->pos.line, start };
               da_append(&tokens, ((Token){.type = equals, .value = "", .pos = p}));
            }
            break;
         case '(':
            {
               size_t start = t->pos.line_pos;
               consume(t);
               TokenPos p = { t->pos.line, start };
               da_append(&tokens, ((Token){.type = open_paren, .value = "", .pos = p}));
            }
            break;
         case ')':
            {
               size_t start = t->pos.line_pos;
               consume(t);
               TokenPos p = { t->pos.line, start };
               da_append(&tokens, ((Token){.type = close_paren, .value = "", .pos = p}));
            }
            break;
         case ';':
            {
               size_t start = t->pos.line_pos;
               consume(t);
               TokenPos p = { t->pos.line, start };
               da_append(&tokens, ((Token){.type = semi, .value = "", .pos = p}));
            }
            break;
         case ':':
            {
               size_t start = t->pos.line_pos;
               consume(t);
               TokenPos p = { t->pos.line, start };
               da_append(&tokens, ((Token){.type = colon, .value = "", .pos = p}));
            }
            break;
         default:
            printf("Error: invalid character: '%c'.\n", consume(t));
            // exit(1);
      }
   }
   free(buf.items);
   return tokens;
} // tokenize()
