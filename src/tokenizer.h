#pragma once
#ifndef TOKENIZER
#define TOKENIZER
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

typedef enum {
   exit_,
   int_lit,
   string_lit,
   semi,
   colon,
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

   // types

   i8_ , char_,   // __INT8_TYPE__
   i16_,          // __INT16_TYPE__
   i32_,          // __INT32_TYPE__
   i64_,          // __INT64_TYPE__
   u8_ , uchar_,  // __UINT8_TYPE__
   u16_,          // __UINT16_TYPE__
   u32_,          // __UINT32_TYPE__
   u64_,          // __UINT64_TYPE__
   f32_,
   f64_,
   uptr,

   bool_,

   string,
   ustring,

   TERMINATE
} TokenType;

typedef struct {
   char* items;
   size_t size;
   size_t capacity;
} Buf;

typedef struct {
   size_t line;
   size_t line_pos;
} TokenPos;

typedef struct {
   TokenType type;
   TokenPos pos;
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
   TokenPos pos;
} Tokenizer;

Tokenizer Tokenizer_create(char** src, size_t length);
Tokens    tokenize(Tokenizer* t);

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
#endif // TOKENIZER
