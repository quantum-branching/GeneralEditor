#include "Lines.h"

void file__go_to_line(FILE *file, int target_line) {
    char buffer[1024];
    int current_line = 1;

    // Rewind to the beginning of the file just in case
    rewind(file);

    // Read and discard lines until reaching the target
    while (current_line < target_line && fgets(buffer, sizeof(buffer), file) != NULL) {
        current_line++;
    }
}

Line *lines__new(Allocator *allocator, Token *first_token, Line *prev_line, Line *next_line) {
	Line *result = allocator->alloc(allocator, sizeof(Line));

	if(result == NULL) {
		return result;
	}

	result->first_token = first_token;
	result->prev_line = prev_line;
	result->next_line = next_line;


	return result;
}

Line *lines__from_document(Allocator *allocator, FILE *file, usize first_line, usize line_count) {
	if(file == NULL) {
		return NULL;
	}

	file__go_to_line(file, first_line);

	Token *token = token__new(allocator, 0, 0, 0);
	Line *result = lines__new(allocator, token, 0, 0);
}