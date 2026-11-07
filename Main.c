#include <termios.h>
#include <stdio.h>
#include <unistd.h>

#include "Input.h"
#include "Render.c"

#define DEBUG

struct termios original;

void enable_raw_mode() {
    struct termios raw;

    tcgetattr(STDIN_FILENO, &original);

    raw = original;

    // Disable canonical mode and echo.
    raw.c_lflag &= ~(ICANON | ECHO);

    // Configure minimum input requirements.
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

	raw.c_lflag &= ~(ICANON | ECHO | ISIG | IEXTEN);

	raw.c_iflag &= ~(IXON | ICRNL);

	raw.c_oflag &= ~(OPOST);

	raw.c_cflag |= CS8;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original);
}

void parse_escape(char *input) {
	if(*input == 0) {
		disable_raw_mode();
		Render__clear_screen();
		_exit(0);
	}

	if(*input == '[') {
		switch(input[1]) {
			case 'A':
				Render__putstring("\033[1A");
				break;
			case 'B':
				Render__putstring("\033[1B");
				break;
			case 'C':
				Render__putstring("\033[1C");
				break;
			case 'D':
				Render__putstring("\033[1D");
				break;
			case '3':
				Render__putstring(Input__delete);
				break;
			default:
				#ifdef DEBUG

				puts(input);
				break;

				#endif
		}
	}

}

int main() {
	enable_raw_mode();
	Render__start_screen();

	char input[10];

	while(1) {
		ssize_t bytes_read = read(STDIN_FILENO, input, 10);

		if (bytes_read <= 0) {
			break;
		}

		switch (input[0]) {
			case INPUT__ESCAPE_KEY:
				parse_escape(input + 1);
				strncpy(input, "\0", 10);
				break;
			case INPUT__BACKSPACE:
				Render__putstring(Input__backspace);
				break;
			case 3:
				Render__putstring("\033[31mI should copy!\033[0m\r\n");
				break;
			case 22:
				Render__putstring("\033[31mI should paste!\033[0m\r\n");
				break;
			case 24:
				Render__putstring("\033[31mI should cut!\033[0m\r\n");
				break;
			case '\r':
			case '\n':
				Render__putstring(Input__newline);
				break;

			default:
				Render__putchar(input[0]);
				break;
		}
	}
}
