#pragma once

#include <string.h>
#include <unistd.h>

#define false 0
#define true 1

#define COLOR_SIZE 6

char consts_color[COLOR_SIZE] = "\033[31m";
char types_color[COLOR_SIZE] = "\033[32m";
char string_color[COLOR_SIZE] = "\033[33m";
char nums_color[COLOR_SIZE] = "\033[34m";
char control_color[COLOR_SIZE] = "\033[35m";
char braces_color[COLOR_SIZE] = "\033[36m";
char preproc_color[COLOR_SIZE] = "\033[91m";
char comment_color[COLOR_SIZE] = "\033[90m";
char reset_color[COLOR_SIZE] = "\033[39m";

typedef char bool;

typedef struct {
	char *buffer;
	size_t buffer_size;

	bool in_string: 1;
	bool using_color: 1;
	bool escaped: 1;
	bool until_newline: 1;
} Highlighter;

static inline Highlighter Highlighter__new(size_t buffer_size) {
	char *buffer = malloc(buffer_size);
	return (Highlighter) {buffer, buffer_size}; 
}

void Highlighter__print_highlighted(Highlighter *highlighter, char *str, size_t str_len) {

	#define add_char() highlighter->buffer[buffer_used++] = str[string_loc]

	size_t buffer_used = 0;
	size_t buffer_size = highlighter->buffer_size;

	bool in_string = highlighter->in_string;
	bool using_color = highlighter->using_color;
	bool escaped = highlighter->escaped;
	bool until_newline = highlighter->until_newline;

	for (size_t string_loc = 0; string_loc < str_len; string_loc++) {
		if(buffer_used + COLOR_SIZE + 1 >= buffer_size) {
			write(STDOUT_FILENO, highlighter->buffer, buffer_used);
			buffer_used = 0;
		}
		
		if(in_string) {
			add_char();

			if(str[string_loc] == '\"' || str[string_loc] == '\'') {
				in_string = false;
				strncpy(highlighter->buffer + buffer_used, reset_color, COLOR_SIZE);
				buffer_used += COLOR_SIZE;
			}

			continue;
		}

		if(escaped) {
			escaped = false;
			add_char();
			continue;
		}

		if(until_newline) {
			if(str[string_loc] == '\n') {
				until_newline = false;
				strncpy(highlighter->buffer + buffer_used, reset_color, COLOR_SIZE);
				buffer_used += COLOR_SIZE;
			}

			add_char();
			continue;
		}

		switch(str[string_loc]) {
			case '/':
				strncpy(highlighter->buffer + buffer_used, comment_color, COLOR_SIZE);
				buffer_used += COLOR_SIZE;
				until_newline = true;
				add_char();
				break;
			case '#':
				strncpy(highlighter->buffer + buffer_used, preproc_color, COLOR_SIZE);
				buffer_used += COLOR_SIZE;
				until_newline = true;
				add_char();
				break;
			case '(':
			case ')':
			case '[':
			case ']':
			case '{':
			case '}':
			case ';':
			case '\n':
			case ' ':
				if(using_color) {
					strncpy(highlighter->buffer + buffer_used, reset_color, COLOR_SIZE);
					buffer_used += COLOR_SIZE;
				}

				using_color = false;
				add_char();
				break;
			case '\'':
			case '\"':
				strncpy(highlighter->buffer + buffer_used, string_color, COLOR_SIZE);
				buffer_used += COLOR_SIZE;
				add_char();

				in_string = true;
				break;
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				strncpy(highlighter->buffer + buffer_used, nums_color, COLOR_SIZE);
				buffer_used += COLOR_SIZE;
				using_color = true;
				add_char();
				break;
			case '\\':
				escaped = true;
				add_char();
				break;
			default:
				add_char();
				break;
		}

		highlighter->in_string = in_string;
		highlighter->using_color = using_color;
		highlighter->escaped = escaped;
		highlighter->until_newline = until_newline;
	}

	if(buffer_used) {
		write(STDOUT_FILENO, highlighter->buffer, buffer_used);
	}

	#undef add_char
}
