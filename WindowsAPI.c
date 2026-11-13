#include <windows.h>

DWORD original_in_mode;
DWORD original_out_mode;

void enable_raw_mode() {
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);

    GetConsoleMode(hStdin, &original_in_mode);
    GetConsoleMode(hStdout, &original_out_mode);

    DWORD raw_in = original_in_mode;
    DWORD raw_out = original_out_mode;

    raw_in &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT);

    raw_in |= ENABLE_VIRTUAL_TERMINAL_INPUT;

    raw_out &= ~(ENABLE_PROCESSED_OUTPUT);
    raw_out |= ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN;

    SetConsoleMode(hStdin, raw_in);
    SetConsoleMode(hStdout, raw_out);
}

void disable_raw_mode() {
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);

    SetConsoleMode(hStdin, original_in_mode);
    SetConsoleMode(hStdout, original_out_mode);
}
