#include "arena.h"

#include <stdlib.h>
#include <string.h>

static ArenaBlock *newBlock(size_t size) {
    ArenaBlock *block = malloc(sizeof(ArenaBlock));
    if (block == NULL)
        return NULL;

    block->memory = malloc(size);

    if (block->memory == NULL) {
        free(block);
        return NULL;
    }

    block->capacity = size;
    block->offset = 0;
    block->next = NULL;

    return block;
}

void arenaInit(Arena *arena) {
    arena->head = NULL;
    arena->current = NULL;
}

void *arenaAlloc(Arena *arena, size_t size) {
    // ポインタサイズ境界にアライン
    size = (size + sizeof(void *) - 1) &
           ~(sizeof(void *) - 1);

    if (arena->current == NULL ||
        arena->current->offset + size >
            arena->current->capacity) {

        size_t blockSize =
            size > ARENA_BLOCK_SIZE ?
            size : ARENA_BLOCK_SIZE;

        ArenaBlock *block = newBlock(blockSize);

        if (block == NULL)
            return NULL;

        if (arena->head == NULL) {
            arena->head = block;
        } else {
            arena->current->next = block;
        }

        arena->current = block;
    }

    void *ptr =
        arena->current->memory +
        arena->current->offset;

    arena->current->offset += size;

    return ptr;
}

char *arenaStrdup(Arena *arena, const char *str) {
    size_t len = strlen(str) + 1;

    char *dst = arenaAlloc(arena, len);

    if (dst == NULL)
        return NULL;

    memcpy(dst, str, len);

    return dst;
}

void arenaReset(Arena *arena) {
    for (ArenaBlock *b = arena->head;
         b != NULL;
         b = b->next) {
        b->offset = 0;
    }

    arena->current = arena->head;
}

void arenaFree(Arena *arena) {
    ArenaBlock *b = arena->head;

    while (b != NULL) {
        ArenaBlock *next = b->next;

        free(b->memory);
        free(b);

        b = next;
    }

    arena->head = NULL;
    arena->current = NULL;
}