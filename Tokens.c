#include "Tokens.h"

struct Token *token__new(Allocator *allocator, usize length, char *string, struct Token *next_token) {
	struct Token *result = allocator->alloc(allocator, sizeof(Token));
	
	if(result == (void *) 0) {
		return result;
	}

	result->length = length;
	result->string = string;
	result->next_token = next_token;

	return result;
}