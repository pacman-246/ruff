#ifndef ARENA_H
#define ARENA_H

#include <stddef.h>

#define ARENA_BLOCK_SIZE (64 * 1024)

typedef struct ArenaBlock {
    char *memory;
    size_t capacity;
    size_t offset;
    struct ArenaBlock *next;
} ArenaBlock;

typedef struct {
    ArenaBlock *head;
    ArenaBlock *current;
} Arena;

void arenaInit(Arena *arena);
void *arenaAlloc(Arena *arena, size_t size);
char *arenaStrdup(Arena *arena, const char *str);
void arenaReset(Arena *arena);
void arenaFree(Arena *arena);

#endif
