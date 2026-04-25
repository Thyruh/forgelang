#pragma once
#ifndef TOKENHANDLER_H
#define TOKENHANDLER_H
#include "../tokenizer.h"
#define EXIT(s, a)\
  printf(ANSI_COLOR_RED "[token_handler]: " s "\n" ANSI_COLOR_RESET, a, pos.line, pos.line_pos+1);\
  (void)system("rm out.c");\
  exit(1);

const char* token_repr(Token t);
const char* get_type(TokenType type);
const char* tokentype_str(TokenType t);

#define DA_INIT_CAP 256
#define da_clear(da) \
    do { \
        memset((da)->items, 0, (da)->size * sizeof(*(da)->items)); \
        (da)->size = 0; \
    } while(0)
   

#define da_append(da, item)                                                          \
   do {                                                                             \
      if ((da)->size >= (da)->capacity) {                                         \
         (da)->capacity = (da)->capacity == 0 ? DA_INIT_CAP : (da)->capacity*2;   \
         (da)->items = realloc((da)->items, (da)->capacity*sizeof(*(da)->items)); \
         assert((da)->items != NULL && "Buy more RAM lol");                       \
      }                                                                            \
      (da)->items[(da)->size++] = (item);                                         \
   } while (0)

#define da_append_many(da, new_items, new_items_size)                                      \
   do {                                                                                    \
      if ((da)->size + new_items_count > (da)->capacity) {                               \
         if ((da)->capacity == 0) {                                                      \
            (da)->capacity = DA_INIT_CAP;                                               \
         }                                                                               \
         while ((da)->size + new_items_count > (da)->capacity) {                        \
            (da)->capacity *= 2;                                                        \
         }                                                                               \
         (da)->items = realloc((da)->items, (da)->capacity*sizeof(*(da)->items));        \
         assert((da)->items != NULL && "Buy more RAM lol");                              \
      }                                                                                   \
      memcpy((da)->items + (da)->size, new_items, new_items_count*sizeof(*(da)->items)); \
      (da)->size += new_items_count;                                                     \
   } while (0)
#endif // TOKENHANDLER_H
