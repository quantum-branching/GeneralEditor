
#include "Allocator.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdalign.h>

// ==================================================
//                  Internal Helpers
// ==================================================

static bool is_power_of_two(size_t value) {
    return value != 0 && (value & (value - 1)) == 0;
}

static bool align_offset(
    size_t offset,
    size_t alignment,
    size_t *result
) {
    if (!is_power_of_two(alignment)) {
        return false;
    }

    size_t mask = alignment - 1;

    if (offset > SIZE_MAX - mask) {
        return false;
    }

    *result = (offset + mask) & ~mask;

    return true;
}

static struct Allocator allocator_empty(void) {
    return (struct Allocator) {0};
}

// ==================================================
//                 Arena Allocation
// ==================================================

static void *arena_allocator_alloc(
    struct Allocator *self,
    size_t amount,
    size_t alignment
) {
    if (!self || !self->alloc_start || amount == 0) {
        return NULL;
    }

    size_t aligned;

    if (!align_offset(self->used, alignment, &aligned)) {
        return NULL;
    }

    if (aligned > self->capacity ||
        amount > self->capacity - aligned) {
        return NULL;
    }

    void *result = self->alloc_start + aligned;

    self->used = aligned + amount;
    self->alloc_ptr = self->alloc_start + self->used;

    return result;
}

static void arena_allocator_free(
    struct Allocator *self,
    void *memory
) {
    // Individual allocations cannot be freed.
    // Use arena_allocator_reset() instead.
    (void) self;
    (void) memory;
}

void arena_allocator_reset(struct Allocator *self) {
    if (!self || !self->alloc_start) {
        return;
    }

    self->used = 0;
    self->alloc_ptr = self->alloc_start;
}

// ==================================================
//                 Stack Allocation
// ==================================================

struct StackAllocation {
    size_t previous_used;
    size_t end_used;
};

static void *stack_allocator_alloc(
    struct Allocator *self,
    size_t amount,
    size_t alignment
) {
    if (!self || !self->alloc_start || amount == 0) {
        return NULL;
    }

    if (!is_power_of_two(alignment)) {
        return NULL;
    }

    size_t header_size = sizeof(struct StackAllocation);

    if (self->used > self->capacity ||
        header_size > self->capacity - self->used) {
        return NULL;
    }

    size_t offset;

    // Reserve room for allocation metadata.
    if (!align_offset(
        self->used + header_size,
        alignment,
        &offset
    )) {
        return NULL;
    }

    if (offset > self->capacity ||
        amount > self->capacity - offset) {
        return NULL;
    }

    size_t end = offset + amount;

    struct StackAllocation metadata = {
        metadata.previous_used = self->used,
        metadata.end_used = end
    };

    // Store metadata immediately before the returned pointer.
    memcpy(
        self->alloc_start + offset - header_size,
        &metadata,
        header_size
    );

    void *result = self->alloc_start + offset;

    self->used = end;
    self->alloc_ptr = self->alloc_start + end;

    return result;
}

static void stack_allocator_free(
    struct Allocator *self,
    void *memory
) {
    if (!self || !memory || !self->alloc_start) {
        return;
    }

    unsigned char *ptr = memory;

    // Verify that the pointer belongs to this allocator.
    if (ptr < self->alloc_start ||
        ptr >= self->alloc_start + self->used) {
        return;
    }

    size_t header_size = sizeof(struct StackAllocation);

    if ((size_t)(ptr - self->alloc_start) < header_size) {
        return;
    }

    struct StackAllocation metadata;

    memcpy(
        &metadata,
        ptr - header_size,
        header_size
    );

    // Only the most recent allocation can be freed.
    if (metadata.end_used != self->used ||
        metadata.previous_used >= metadata.end_used) {
        return;
    }

    self->used = metadata.previous_used;
    self->alloc_ptr = self->alloc_start + self->used;
}

size_t stack_allocator_checkpoint(struct Allocator *self) {
    if (!self) {
        return 0;
    }

    return self->used;
}

bool stack_allocator_restore(
    struct Allocator *self,
    size_t checkpoint
) {
    if (!self || !self->alloc_start ||
        checkpoint > self->used) {
        return false;
    }

    self->used = checkpoint;
    self->alloc_ptr = self->alloc_start + checkpoint;

    return true;
}

// ==================================================
//                 Malloc Allocation
// ==================================================

static void *malloc_allocator_alloc(
    struct Allocator *self,
    size_t amount,
    size_t alignment
) {
    (void) self;

    if (amount == 0 || !is_power_of_two(alignment)) {
        return NULL;
    }

    // malloc guarantees fundamental alignment.
    if (alignment <= alignof(max_align_t)) {
        return malloc(amount);
    }

    // aligned_alloc requires the size to be a
    // multiple of the requested alignment.
    size_t remainder = amount % alignment;

    if (remainder != 0) {
        size_t padding = alignment - remainder;

        if (amount > SIZE_MAX - padding) {
            return NULL;
        }

        amount += padding;
    }

    return aligned_alloc(alignment, amount);
}

static void malloc_allocator_free(
    struct Allocator *self,
    void *memory
) {
    (void) self;
    free(memory);
}

// ==================================================
//                  Destructors
// ==================================================

static void arena_allocator_destroy(struct Allocator *self) {
    if (!self) {
        return;
    }

    free(self->alloc_start);

    *self = allocator_empty();
}

static void stack_allocator_destroy(struct Allocator *self) {
    if (!self) {
        return;
    }

    free(self->alloc_start);

    *self = allocator_empty();
}

static void malloc_allocator_destroy(struct Allocator *self) {
    // Individual allocations belong to the caller.
    // There is no backing allocation to destroy.
    if (self) {
        *self = allocator_empty();
    }
}

// ==================================================
//                  Constructors
// ==================================================

struct Allocator arena_allocator_new(size_t capacity) {
    if (capacity == 0) {
        return allocator_empty();
    }

    unsigned char *memory = malloc(capacity);

    if (!memory) {
        return allocator_empty();
    }

    return (struct Allocator) {
        .alloc = arena_allocator_alloc,
        .free = arena_allocator_free,
        .destroy = arena_allocator_destroy,

        .alloc_start = memory,
        .alloc_ptr = memory,

        .capacity = capacity,
        .used = 0
    };
}

struct Allocator stack_allocator_new(size_t capacity) {
    if (capacity == 0) {
        return allocator_empty();
    }

    unsigned char *memory = malloc(capacity);

    if (!memory) {
        return allocator_empty();
    }

    return (struct Allocator) {
        .alloc = stack_allocator_alloc,
        .free = stack_allocator_free,
        .destroy = stack_allocator_destroy,

        .alloc_start = memory,
        .alloc_ptr = memory,

        .capacity = capacity,
        .used = 0
    };
}

struct Allocator malloc_allocator_new(void) {
    return (struct Allocator) {
        .alloc = malloc_allocator_alloc,
        .free = malloc_allocator_free,
        .destroy = malloc_allocator_destroy,

        .alloc_start = NULL,
        .alloc_ptr = NULL,

        .capacity = 0,
        .used = 0
    };
}

// ==================================================
//                    Wrappers
// ==================================================

void *allocator_alloc_aligned(
    struct Allocator *self,
    size_t amount,
    size_t alignment
) {
    if (!self || !self->alloc) {
        return NULL;
    }

    return self->alloc(self, amount, alignment);
}

void *allocator_alloc(
    struct Allocator *self,
    size_t amount
) {
    return allocator_alloc_aligned(
        self,
        amount,
        alignof(max_align_t)
    );
}

void allocator_free(
    struct Allocator *self,
    void *memory
) {
    if (!self || !self->free || !memory) {
        return;
    }

    self->free(self, memory);
}

void allocator_destroy(struct Allocator *self) {
    if (!self || !self->destroy) {
        return;
    }

    self->destroy(self);
}
