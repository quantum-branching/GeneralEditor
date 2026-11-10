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
char reset_color[COLOR_SIZE] = "\033[39m";

typedef struct {
	char *buffer;
	size_t buffer_size;
} Highlighter;

typedef char bool;

void Highlighter__print_highlighted(Highlighter *highligher, char *str, size_t str_len) {

	#define add_char() highligher->buffer[buffer_used++] = str[string_loc]

	size_t buffer_used = 0;
	size_t buffer_size = highligher->buffer_size;

	bool in_string = false;
	bool using_color = false;
	bool escaped = false;
	bool until_newline = false;

	for (size_t string_loc = 0; string_loc < str_len; string_loc++) {
		if(buffer_used + COLOR_SIZE >= buffer_size) {
			write(STDOUT_FILENO, highligher->buffer, buffer_used);
			buffer_used = 0;
		}
		
		if(escaped) {
			escaped = false;
			highligher->buffer[buffer_used++] = str[string_loc];
			continue;
		}

		if(until_newline) {
			if(str[string_loc] == '\n') {
				until_newline = false;
			} else {
				add_char();
				continue;
			}
		}

		switch(str[string_loc]) {
			case '#':
				strncpy(highligher->buffer + buffer_used, preproc_color, COLOR_SIZE);
				buffer_used += COLOR_SIZE;
				using_color = true;
				until_newline = true;
				add_char();
				break;
			case '(':
			case ')':
			case '[':
			case ']':
			case '{':
			case '}':
			case '\n':
			case ' ':
				if(using_color) {
					strncpy(highligher->buffer + buffer_used, reset_color, COLOR_SIZE);
					buffer_used += COLOR_SIZE;
				}

				using_color = false;
				add_char();
				break;
			case '\'':
			case '\"':
				if(in_string) {
					add_char();
					strncpy(highligher->buffer + buffer_used, reset_color, COLOR_SIZE);
					buffer_used += COLOR_SIZE;
				} else {
					strncpy(highligher->buffer + buffer_used, string_color, COLOR_SIZE);
					buffer_used += COLOR_SIZE;
					add_char();
				}
				
				using_color = true;
				in_string = !in_string;
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
				strncpy(highligher->buffer + buffer_used, nums_color, COLOR_SIZE);
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

	}

	if(buffer_used) {
		write(STDOUT_FILENO, highligher->buffer, buffer_used);
	}

	#undef add_char
}
