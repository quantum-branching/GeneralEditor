
#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stddef.h>
#include <stdbool.h>

// ==================================================
//                     Allocator
// ==================================================

struct Allocator {
    void *(*alloc)(
        struct Allocator *self,
        size_t amount,
        size_t alignment
    );

    void (*free)(
        struct Allocator *self,
        void *memory
    );

    void (*destroy)(struct Allocator *self);

    unsigned char *alloc_start;
    unsigned char *alloc_ptr;

    size_t capacity;
    size_t used;
};

// ==================================================
//                   Constructors
// ==================================================

struct Allocator arena_allocator_new(size_t capacity);
struct Allocator stack_allocator_new(size_t capacity);
struct Allocator malloc_allocator_new(void);

// ==================================================
//                  Allocator API
// ==================================================

void *allocator_alloc(struct Allocator *self, size_t amount);

void *allocator_alloc_aligned(
    struct Allocator *self,
    size_t amount,
    size_t alignment
);

void allocator_free(struct Allocator *self, void *memory);

void allocator_destroy(struct Allocator *self);

// ==================================================
//                  Arena Allocator
// ==================================================

void arena_allocator_reset(struct Allocator *self);

// ==================================================
//                  Stack Allocator
// ==================================================

size_t stack_allocator_checkpoint(struct Allocator *self);

bool stack_allocator_restore(
    struct Allocator *self,
    size_t checkpoint
);

#endif
