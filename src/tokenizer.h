#pragma once
#ifndef TOKENIZER
#define TOKENIZER
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <string.h>

typedef enum {
   exit_,
   int_lit,
   string_lit,
   semi,
   d_quote,
   s_quote,
   plus,
   minus,
   times,
   slash,
   print,
   ident,
   open_paren,
   close_paren,
   return_,
   let,
   equals,
   TERMINATE
} TokenType;

typedef struct {
   TokenType type;
   const char* value;
} Token;

typedef struct {
   Token* items;
   size_t size;
   size_t capacity;
} Tokens;

typedef struct {
   const char* src;
   size_t length;

   size_t index;
} Tokenizer;

Tokenizer Tokenizer_create(char** src, size_t length);
Tokens    tokenize(Tokenizer* t);

#define DA_INIT_CAP 256
#define da_clear(da)  /* What the fuck */\
   do { \
      (da)->size = 0;\
      memset(((da))->items, 0, (da)->items[(da)->size]);\
   } while(0)
   

#define da_append(da, item)                                                          \
   do {                                                                             \
      if ((da)->size >= (da)->capacity) {                                         \
         (da)->capacity = (da)->capacity == 0 ? DA_INIT_CAP : (da)->capacity*2;   \
         (da)->items = realloc((da)->items, (da)->capacity*sizeof(*(da)->items)); \
         assert((da)->items != NULL && "Buy more RAM lol");                       \
      }                                                                            \
      \
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
#endif // TOKENIZER
