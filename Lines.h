#include <stdio.h>

#include "Types.h"
#include "Allocator.h"
#include "Tokens.h"

typedef struct Line Line;

struct Line {
	Token *first_token;
	Line *prev_line;
	Line *next_line;
};

struct Line *lines__from_document(Allocator *allocator, FILE *file, usize first_line, usize line_count);

/// Joins the right line into the left line and returns the left line
struct Line *lines__join(struct Allocator *allocator, struct Line *left, struct Line *right);

/// Places the latter part of the part of the line as the next line and returns the modified line
struct Line *lines__split(struct Allocator *allocator, struct Line *line, struct Token *delimeter);
