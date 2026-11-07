#include <stdio.h>
#include <sys/ioctl.h>

#include "Termios.c"
#include "Input.h"
#include "Render.c"
#include "File.c"

// #define DEBUG

int MAX_ROWS = 75;

int row = 0;
int column = 0;

char *working_file = NULL;
size_t file_size = 0;
char *buffer = NULL;

void __exit(int status) {
	if(working_file) {
		free(working_file);
	}

	_exit(status);
}

void parse_escape(char *input) {
	if(*input == 0) {
		disable_raw_mode();
		Render__clear_screen();
		__exit(0);
	}

	if(*input == '[') {

		switch(input[1]) {
			case 'A':

				if(row > MAX_ROWS) {
					Render__blank_screen();
					row = Render__putunix(working_file, file_size, buffer, 512, row - 1 - MAX_ROWS, row - 1);
				} else if(row > 0) {
					row -= 1;
					Render__putstring("\033[1A");
				}

				break;
			case 'B':
				if(row < MAX_ROWS) {
					Render__putstring("\033[1B");
					row += 1;
				} else {
					Render__blank_screen();
					row = Render__putunix(working_file, file_size, buffer, 512, row + 1 - MAX_ROWS, row + 2) - 1;
				}
				
				break;
			case 'C':
				Render__putstring("\033[1C");
				break;
			case 'D':
				Render__putstring("\033[1D");
				break;
			case '2':
				// paste_handler(input + 5);
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

int main(int argc, char **argv) {
	enable_raw_mode();
	Render__blank_screen();

	char input[10];
	buffer = malloc(512);

	struct winsize w;
	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) {
		perror("ioctl");
		return 1;
	}

	MAX_ROWS = w.ws_row - 3;

	if(argc > 1) {
		file_size = File__get_size(argv[1]);
		working_file = malloc(file_size);
		File__read_file(argv[1], working_file);
		row = Render__putunix(working_file, file_size, buffer, 512, 0, MAX_ROWS);
		free(buffer);
	}

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

			#ifdef DEBUG

			case 3:
				Render__putstring("\033[31mI should copy!\033[0m\r\n");
				break;
			case 22:
				Render__putstring("\033[31mI should paste!\033[0m\r\n");
				break;
			case 24:
				Render__putstring("\033[31mI should cut!\033[0m\r\n");
				break;

			#endif

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
