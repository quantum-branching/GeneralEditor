#include <stdlib.h>
#include <stdio.h>

#include "Allocator.h"

// ==================================================
//               Allocator Constructors
// ==================================================

struct Allocator arena__allocator__new(uintptr_t size) {
	uintptr_t start = (uintptr_t) malloc(size);

	if(start == NULL) {
		puts("Allocator.c arena::allocator_new() failed to allocated requested memory!");
		exit(1);
	}

	return (struct Allocator) {arena__allocator_free, arena__allocator_alloc, start, start, start + size - 1};
}

struct Allocator malloc__new() {
	return (struct Allocator) {malloc__allocator_free, malloc__allocator_alloc, 0, 0, 0};
}

struct Allocator stack__allocator__new(uintptr_t size) {
	uintptr_t start = (uintptr_t) malloc(size);

	if(start == NULL) {
		puts("Allocator.c stack::allocator_new() failed to allocate requested memory!");
		exit(1);
	}

	return (struct Allocator) {stack__allocator_free, arena__allocator_alloc, start, start, start + size - 1};
}

// ==================================================
//                  Allocator Frees
// ==================================================

void arena__allocator_free(struct Allocator *self, void *memory, uintptr_t amount) {
	self->alloc_ptr = self->alloc_start;
}

void stack__allocator_free(struct Allocator *self, void *memory, uintptr_t amount) {
	self->alloc_ptr -= amount;

	if(self->alloc_ptr < self->alloc_start) {
		puts("Allocator.c stack::allocator_free() freed more memory than was on the stack! Resetting stack pointer!");
		self->alloc_ptr = self->alloc_start;
	}
}

void malloc__allocator_free(struct Allocator *self, void *memory, uintptr_t amount) {
	free(memory);
}

// ==================================================
//                Allocator Allocates
// ==================================================

void *arena__allocator_alloc(struct Allocator *self, uintptr_t amount) {
	void *result = (void *) self->alloc_ptr;
	self->alloc_ptr += amount;

	if(self->alloc_ptr > self->alloc_end) {
		puts("Allocator.c arena::allocator_alloc() is allocating too much! Try requesting more space when creating the allocator.");
		exit(1);
	}

	return result;
}

void *malloc__allocator_alloc(struct Allocator *self, uintptr_t amount) {
	void *result = malloc(amount);

	if(result == NULL) {
		puts("Allocator.c malloc::allocator_alloc() failed to allocate requested memory!");
		exit(1);
	}

	return result;
}

// ==================================================
//                  Frees Allocator
// ==================================================

void free_allocator(struct Allocator *allocator) {
	free(allocator->alloc_start);
}

// ==================================================
//                      Wrappers
// ==================================================

void allocator__free(struct Allocator *self, void *memory, uintptr_t amount) {
	(self->free)(self, memory, amount);
}

void *allocator__alloc(struct Allocator *self, uintptr_t amount) {
	return (self->alloc)(self, amount);
}
