#include <stdio.h>

struct Line {
	struct Token *first_token;
	struct Line *prev_line;
	struct Line *next_line;
};

struct Line *lines__from_document(struct Allocator *allocator, FILE *file, unsigned int first_line, unsigned int line_count);

/// Joins the right line into the left line and returns the left line
struct Line *lines__join(struct Allocator *allocator, struct Line *left, struct Line *right);

/// Places the latter part of the part of the line as the next line and returns the modified line
struct Line *lines__split(struct Allocator *allocator, struct Line *line, struct Token *delimeter);
