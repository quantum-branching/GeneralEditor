#include <termios.h>
#include <unistd.h>

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
	// write(STDOUT_FILENO, "\033[?2004h", 8);
}

void disable_raw_mode() {
	// write(STDOUT_FILENO, "\033[?2004l", 8);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original);
}