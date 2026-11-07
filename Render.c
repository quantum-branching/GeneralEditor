#include <unistd.h>
#include <string.h>

static inline void Render__putstring(char *string) {
	write(STDOUT_FILENO, string, strlen(string));
}

static inline void Render__putchar(char character) {
	write(STDOUT_FILENO, &character, 1);
}

void Render__clear_screen() {
	Render__putstring("\033[0m\033[0;0H\033[2J");
}

void Render__start_screen() {
	Render__clear_screen();
	Render__putstring("\033[47m\033[30mQuantum's Little Text Editor\r\n\033[0m");
}