#include <stdio.h>

#include "Input.h"
#include "Renderer.c"
#include "Highlighter.c"
#include "File.c"

// #define DEBUG

#ifdef _WIN32
	#include "WindowsAPI.c"

	static inline int get_terminal_height() {
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);

		if (hStdout == INVALID_HANDLE_VALUE) {
			return -1;
		}
	
		if (!GetConsoleScreenBufferInfo(hStdout, &csbi)) {
			return -1;
		}
		
		return csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
	}
#else
    	#include <sys/ioctl.h>
	#include "Termios.c"

	static inline int get_terminal_height() {
		struct winsize w;
		
		if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) {
			perror("ioctl");
			return 1;
		}
		
		return w.ws_row;
	}
#endif

#define new_buffer(type, size) ((type) {malloc(size), size})
#define free_buffer(loc) (free(loc.buffer))

int MAX_ROWS = 75;

int top_row = 0;
int row = 0;
int column = 0;

char *working_file = NULL;
size_t file_size = 0;
char *buffer = NULL;

Renderer renderer = (Renderer) {0, 0};
Highlighter highlighter = (Highlighter) {0, 0};

void __exit(int status) {
	if(working_file) {
		free(working_file);
	}

	free_buffer(renderer);

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
				if(row && row == top_row) {
					Render__blank_screen();
					top_row = Render__putunix(working_file, file_size, &renderer, &highlighter, top_row - MAX_ROWS, top_row) - MAX_ROWS;

					top_row = top_row > 0 ? top_row : (0 * Render__putunix(working_file, file_size, &renderer, &highlighter, 0, MAX_ROWS));

					row = top_row + MAX_ROWS;
					Render__putstring("\033[999H\033[1A");
				} else if(row > 0) {
					row -= 1;
					Render__putstring("\033[1A");
				}

				break;
			case 'B':
				if(row < top_row + MAX_ROWS) {
					Render__putstring("\033[1B");
					row += 1;
				} else {
					Render__blank_screen();
					top_row = Render__putunix(working_file, file_size, &renderer, &highlighter, row, row + MAX_ROWS) - MAX_ROWS - 1;
					row = top_row;
					Render__putstring("\033[2H");
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

	renderer = new_buffer(Renderer, 512);
	highlighter = Highlighter__new(1024);

	MAX_ROWS = get_terminal_height() - 3;

	if(argc > 1) {
		file_size = File__get_size(argv[1]);
		working_file = malloc(file_size);
		File__read_file(argv[1], working_file);
		row = Render__putunix(working_file, file_size, &renderer, &highlighter, 0, MAX_ROWS);
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
