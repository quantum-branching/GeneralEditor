#include "Types.h"
#include "Allocator.h"

typedef struct Token Token;

struct Token {
	usize length;
	char *string;
	struct Token *next_token;
};

struct Token *token__new(Allocator *allocator, usize length, char *string, struct Token *next_token);