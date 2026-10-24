#include <stdint.h>

struct Allocator {
	void (*free)(struct Allocator *self, void *memory, uintptr_t amount);
	void *(*alloc)(struct Allocator *self, uintptr_t amount);
	uintptr_t alloc_start;
	uintptr_t alloc_ptr;
	uintptr_t alloc_end;
};

// ==========================
//   Allocator Constructors
// ==========================

struct Allocator arena__allocator__new(uintptr_t size);

struct Allocator malloc__new();

struct Allocator stack__allocator__new(uintptr_t size);

// ===================
//   Frees Allocator
// ===================

void free_allocator(struct Allocator *allocator);

// ============
//   Wrappers
// ============

void allocator__free(struct Allocator *self, void *memory, uintptr_t amount);

void *allocator__alloc(struct Allocator *self, uintptr_t amount);
