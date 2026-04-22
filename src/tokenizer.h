#pragma once
#ifndef TOKENIZER
#define TOKENIZER
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARENA_IMPLEMENTATION
#define ARENA_VOID_RESPONSE
#define ARENA_REPORT
#include "./common/arena.h"

typedef enum {
  // symbols
  semi,
  colon,
  plus,
  minus,
  star,
  open_paren,
  close_paren,
  equals,
  fslash,

  bslash,
  open_brace,
  close_brace,
  open_bracket,
  close_bracket,
  ampersand,
  comma,
  dot, // Does both dereferences and access
  caret,
  pipe,
  exclam, // Generics symbol
  greater,
  less,
  tilda,
  grave,

  // functions and keywords
  print,
  println,
  write,
  writeln,
  const_,
  mut,
  ident,
  return_,
  exit_,

  // literals

  int_lit,
  string_lit,
  char_lit,
  func_lit, // for much later

  // types

  char_,
  uchar_,

  i8_,  // __INT8_TYPE__
  i16_, // __INT16_TYPE__
  i32_, // __INT32_TYPE__
  i64_, // __INT64_TYPE__
  u8_,  // __UINT8_TYPE__
  u16_, // __UINT16_TYPE__
  u32_, // __UINT32_TYPE__
  u64_, // __UINT64_TYPE__
  usize,
  f32_,
  f64_,
  uptr,
  ptr,

  bool_,

  string,

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

  mem_arena* arena;
} Tokenizer;

Tokenizer Tokenizer_create(char** src, size_t length, mem_arena* arena);
Tokens tokenize(Tokenizer* tokenizer);
#endif // TOKENIZER
