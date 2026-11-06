
#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <stddef.h>
#include <stdbool.h>

// ==================================================
//                       Structs
// ==================================================

struct Token {
    size_t start;
    size_t length;
};

struct Line {
    struct Line *next;
    struct Line *prev;

    char *data;

    size_t length;
    size_t capacity;
};

struct ActiveLine {
    struct Line *source;

    struct Token *tokens;

    size_t token_count;
    size_t token_capacity;

    bool tokens_valid;
};

struct Cursor {
    struct Line *line;

    size_t offset;
};

struct Document {
    struct Line *first;
    struct Line *last;

    struct ActiveLine active;
    struct Cursor cursor;

    size_t line_count;
};

// ==================================================
//                  Constructors
// ==================================================

bool document__init(struct Document *document);
void document__destroy(struct Document *document);

// ==================================================
//                   Line Operations
// ==================================================

struct Line *line__new(void);
bool line__reserve(struct Line *line, size_t required);

bool document__split_line(struct Document *document);

bool document__merge_lines(
    struct Document *document,
    struct Line *line
);

// ==================================================
//                  Editing Operations
// ==================================================

bool document__insert(struct Document *document, char character);
bool document__delete(struct Document *document);
bool document__backspace(struct Document *document);

// ==================================================
//                  Cursor Operations
// ==================================================

void document__set_cursor(
    struct Document *document,
    struct Line *line,
    size_t offset
);

// ==================================================
//                  Token Operations
// ==================================================

bool active_line__tokenize(struct Document *document);
void active_line__invalidate(struct Document *document);

#endif
