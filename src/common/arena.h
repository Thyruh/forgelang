#pragma once
#ifndef ARENA_H
#define ARENA_H
#include <stdbool.h>
#include <stdint.h>

#include "inttypes.h"

#define KiB(n) ((u64)n << 10)
#define MiB(n) ((u64)n << 20)
#define GiB(n) ((u64)n << 30)

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define ALIGN_UP_POW2(n, p) (((u64)(n) + ((u64)(p) - 1)) & (~((u64)(p) - 1)))

#define ARENA_BASE_POS sizeof(mem_arena)
#define ARENA_ALIGN sizeof(void*)

#define arena_push(arena, T) ((T*)m_arena_push(arena, sizeof(T), false))
#define arena_push_dn(arena, T) ((T*)m_arena_push(arena, sizeof(T), true)) // dynamic
#define arena_push_arr(arena, T, n) ((T*)m_arena_push(arena, sizeof(T) * (n), false))
#define arena_push_arr_dn(arena, T, n) ((T*)m_arena_push(arena, sizeof(T) * (n), true)) // dynamic

typedef struct {
   u64 pos;
   u64 capacity;
   i64 offset; // For reallocation to prevent dangling pointers
} mem_arena;

static inline mem_arena* arena_create(u64 capacity);
static inline void arena_free(mem_arena* arena);
static inline void arena_pop(mem_arena* arena, u64 size); // TODO implement a more stack-like behaviour
static inline void arena_pop_to(mem_arena* arena, u64 pos);
static inline void arena_clear(mem_arena* arena);
static inline void* m_arena_push(mem_arena* arena, u64 size, bool dn);

// header guards
#ifdef ARENA_IMPLEMENTATION
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

mem_arena* arena_create(u64 capacity) {
#ifndef ARENA_VOID_RESPONSE
   printf("[arena.h]: CONSTRUCTING THE ARENA\n[arena.h]: EXPECTING THE DESTRUCTOR\n");
#endif
   mem_arena* arena = (mem_arena*) malloc(capacity);
   arena->capacity = capacity;
   arena->pos = ARENA_BASE_POS;
   return arena;
}

void arena_free(mem_arena* arena) {
#ifndef ARENA_VOID_RESPONSE
   printf("[arena.h]: DESTROYING THE ARENA\n");
#endif

#ifdef ARENA_MEM_SAFETY
   printf("[MEM_SAFETY_MESSAGE]: If you segfault - check for accessing freed memory\n");
#endif

#ifdef ARENA_REPORT
   printf("[arena.h]: Used (no header): %luB\n",
         arena->pos - ARENA_BASE_POS);

   printf("[arena.h]: Used (total): %luB\n",
         arena->pos);

   printf("[arena.h]: Capacity: %luB\n",
         arena->capacity);

   printf("[arena.h]: Ratio (%%): %lu/100%%\n",
         (arena->pos - ARENA_BASE_POS) * 100 /
         (arena->capacity - ARENA_BASE_POS));

#endif

   arena_clear(arena);
   free(arena);
}

void* m_arena_push(mem_arena* arena, u64 size, bool dn) {
   u64 pos_aligned = ALIGN_UP_POW2(arena->pos, ARENA_ALIGN);
   u64 new_pos = pos_aligned + size;

   if (new_pos > arena->capacity) {
      if (dn) {  // if dynamic
#ifndef ARENA_VOID_RESPONSE
         printf("[DEV_MESSAGE]: Resized the arena for the new allocation, ");
         printf("consider creating another arena or initializing with more memory.\n");
#endif
         char* old_pos = (void*)arena;
         arena = (mem_arena*)realloc(arena, arena->capacity+new_pos-arena->capacity); // abstract into another function
         arena->offset = old_pos - (char*)arena;
#ifdef ARENA_MEM_SAFETY
         printf("[MEM_SAFETY_MESSAGE]: If you segfault - check for dangling pointers. ");
         printf("Use the mem_arena->offset property\n");
#endif
      } 
      else {
#ifndef ARENA_VOID_RESPONSE
         printf("[DEV_MESSAGE]: Not enough memory on the arena for a new allocation, ");
         printf("consider creating another arena or initializing with more memory.\n");
#endif
         return NULL;
      }
   }

   u8* rsp = (u8*) arena + pos_aligned;

   arena->pos = new_pos;

   return rsp;
}

void arena_pop(mem_arena* arena, u64 size){
   size = MIN(size, arena->pos - ARENA_BASE_POS);
   arena->pos -= size;
}

void arena_pop_to(mem_arena* arena, u64 pos) {
   u64 size = pos < arena->pos ? arena->pos - pos : 0;
   arena_pop(arena, size);
}

void arena_clear(mem_arena* arena) {
   arena_pop(arena, ARENA_BASE_POS);
#ifndef ARENA_VOID_RESPONSE
   printf("[arena.h]: CLEANING THE ARENA\n");
#endif
}

#endif // ARENA_IMPLEMENTATION
#endif // ARENA_H
