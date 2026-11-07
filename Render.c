#include <unistd.h>
#include <string.h>
#include <stdlib.h>

static inline void Render__putstring(char *string) {
	write(STDOUT_FILENO, string, strlen(string));
}

size_t Render__putunix(char *unix_string, int size, char *buffer, size_t buffer_size, size_t minline, size_t maxline) {
	size_t buffer_idx = 0;
	size_t line_count = 1;

	for (size_t i = 0; i < size; i++) {
		if (unix_string[i] == '\n') {
			if(line_count > maxline) {
				if(buffer_idx > 0) {
					write(STDOUT_FILENO, buffer, buffer_idx);
				}

				return line_count - 1;
			}
			
			if(line_count > minline) {
				buffer[buffer_idx++] = '\r';
				buffer[buffer_idx++] = '\n';
			}

			line_count += 1;
		} else {
			if(line_count > minline) {
				buffer[buffer_idx++] = unix_string[i];
			}
		}

		if(buffer_idx + 2 > buffer_size) {
			write(STDOUT_FILENO, buffer, buffer_idx);
			buffer_idx = 0;
		}
	}

	if(buffer_idx > 0) {
		write(STDOUT_FILENO, buffer, buffer_idx);
	}

	return line_count;
}

static inline void Render__putchar(char character) {
	write(STDOUT_FILENO, &character, 1);
}

void Render__clear_screen() {
	Render__putstring("\033[0m\033[0;0H\033[2J");
}

void Render__blank_screen() {
	Render__clear_screen();
	Render__putstring("\033[47m\033[30m     Quantum's Little Text Editor\033[K\033[0m\r\n");
}