#include "Document.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// ==================================================
//                        Line
// ==================================================

struct Line *line__new(void) {
    struct Line *line = malloc(sizeof(struct Line));

    if (!line) {
        return NULL;
    }

    line->data = malloc(16);

    if (!line->data) {
        free(line);
        return NULL;
    }

    line->next = NULL;
    line->prev = NULL;

    line->length = 0;
    line->capacity = 16;

    line->data[0] = '\0';

    return line;
}

bool line__reserve(struct Line *line, size_t required) {
    if (required <= line->capacity) {
        return true;
    }

    size_t capacity = line->capacity;

    while (capacity < required) {
        if (capacity > SIZE_MAX / 2) {
            capacity = required;
            break;
        }

        capacity *= 2;
    }

    char *data = realloc(line->data, capacity);

    if (!data) {
        return false;
    }

    line->data = data;
    line->capacity = capacity;

    return true;
}

// ==================================================
//                      Document
// ==================================================

bool document__init(struct Document *document) {
    if (!document) {
        return false;
    }

    struct Line *line = line__new();

    if (!line) {
        return false;
    }

    document->first = line;
    document->last = line;

    document->line_count = 1;

    document->cursor.line = line;
    document->cursor.offset = 0;

    document->active.source = NULL;
    document->active.tokens = NULL;
    document->active.token_count = 0;
    document->active.token_capacity = 0;
    document->active.tokens_valid = false;

    return true;
}

bool document__insert(struct Document *document, char character) {
    struct Line *line = document->cursor.line;
    size_t position = document->cursor.offset;

    if (!line__reserve(line, line->length + 2)) {
        return false;
    }

    memmove(
        line->data + position + 1,
        line->data + position,
        line->length - position + 1
    );

    line->data[position] = character;

    line->length++;
    document->cursor.offset++;

    document->active.tokens_valid = false;

    return true;
}

bool document__delete(struct Document *document) {
    struct Line *line = document->cursor.line;
    size_t position = document->cursor.offset;

    if (position >= line->length) {
        return false;
    }

    memmove(
        line->data + position,
        line->data + position + 1,
        line->length - position
    );

    line->length--;

    document->active.tokens_valid = false;

    return true;
}

bool document__split_line(struct Document *document) {
    struct Line *line = document->cursor.line;
    size_t position = document->cursor.offset;

    struct Line *new_line = line__new();

    if (!new_line) {
        return false;
    }

    size_t remaining = line->length - position;

    if (!line__reserve(new_line, remaining + 1)) {
        free(new_line->data);
        free(new_line);
        return false;
    }

    memcpy(
        new_line->data,
        line->data + position,
        remaining
    );

    new_line->length = remaining;
    new_line->data[remaining] = '\0';

    // Shorten the original line.
    line->length = position;
    line->data[position] = '\0';

    // Link the new line into the document.
    new_line->next = line->next;
    new_line->prev = line;

    if (line->next) {
        line->next->prev = new_line;
    } else {
        document->last = new_line;
    }

    line->next = new_line;

    document->line_count++;

    // Move cursor to the beginning of the new line.
    document->cursor.line = new_line;
    document->cursor.offset = 0;

    // Invalidate token cache.
    document->active.tokens_valid = false;

    return true;
}

bool document__merge_lines(struct Document *document, struct Line *line) {
    if (!line || !line->next) {
        return false;
    }

    struct Line *next = line->next;

    size_t original_length = line->length;

    if (next->length > SIZE_MAX - original_length - 1) {
        return false;
    }

    if (!line__reserve(
        line,
        original_length + next->length + 1
    )) {
        return false;
    }

    memcpy(
        line->data + original_length,
        next->data,
        next->length + 1
    );

    line->length += next->length;

    // Remove next from the linked list.
    line->next = next->next;

    if (next->next) {
        next->next->prev = line;
    } else {
        document->last = line;
    }

    // Move cursor to the join position.
    document->cursor.line = line;
    document->cursor.offset = original_length;

    free(next->data);
    free(next);

    document->line_count--;

    document->active.tokens_valid = false;

    return true;
}

void document__set_cursor(struct Document *document, struct Line *line, size_t offset) {
    if (!document || !line) {
        return;
    }

    if (offset > line->length) {
        offset = line->length;
    }

    if (document->cursor.line != line) {
        document->active.tokens_valid = false;
    }

    document->cursor.line = line;
    document->cursor.offset = offset;
}

void document__destroy(struct Document *document) {
    if (!document) {
        return;
    }

    struct Line *line = document->first;

    while (line) {
        struct Line *next = line->next;

        free(line->data);
        free(line);

        line = next;
    }

    free(document->active.tokens);

    *document = (struct Document) {0};
}



// ==================================================
//                    Active Line
// ==================================================

static bool active_line__reserve(struct ActiveLine *active, size_t required) {
    if (required <= active->token_capacity) {
        return true;
    }

    size_t capacity = active->token_capacity
        ? active->token_capacity * 2
        : 8;

    while (capacity < required) {
        if (capacity > SIZE_MAX / 2) {
            capacity = required;
            break;
        }

        capacity *= 2;
    }

    struct Token *tokens = realloc(
        active->tokens,
        capacity * sizeof(struct Token)
    );

    if (!tokens) {
        return false;
    }

    active->tokens = tokens;
    active->token_capacity = capacity;

    return true;
}

bool active_line__tokenize(struct Document *document) {
    struct ActiveLine *active = &document->active;
    struct Line *line = document->cursor.line;

    if (active->source == line && active->tokens_valid) {
        return true;
    }

    size_t position = 0;
    size_t count = 0;

    while (position < line->length) {
        size_t start = position;

        bool whitespace = isspace(
            (unsigned char) line->data[position]
        );

        while (position < line->length) {
            bool current_whitespace = isspace(
                (unsigned char) line->data[position]
            );

            if (current_whitespace != whitespace) {
                break;
            }

            position++;
        }

        if (!active_line__reserve(active, count + 1)) {
            return false;
        }

        active->tokens[count++] = (struct Token) {
            .start = start,
            .length = position - start
        };
    }

    active->source = line;
    active->token_count = count;
    active->tokens_valid = true;

    return true;
}