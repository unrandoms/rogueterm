#include "input.h"
#include <termios.h>
#include <unistd.h>
#include <cstdio>

static struct termios orig_termios;
static bool termios_saved = false;

void input_init() {
    if (tcgetattr(STDIN_FILENO, &orig_termios) == 0) {
        termios_saved = true;
        struct termios raw = orig_termios;
        // Disable canonical mode and echo
        raw.c_lflag &= ~(ICANON | ECHO);
        // Return immediately with at least 1 byte
        raw.c_cc[VMIN]  = 1;
        raw.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    }
}

void input_restore() {
    if (termios_saved)
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

Key read_key() {
    unsigned char c = 0;
    if (read(STDIN_FILENO, &c, 1) <= 0) return Key::Unknown;

    // Handle ESC sequences (arrow keys)
    if (c == 27) {
        unsigned char seq[3] = {0,0,0};
        // Try to read two more bytes (with small timeout)
        // We set non-blocking temporarily
        struct termios t;
        tcgetattr(STDIN_FILENO, &t);
        struct termios nb = t;
        nb.c_cc[VMIN]  = 0;
        nb.c_cc[VTIME] = 1;  // 0.1s timeout
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &nb);

        if (read(STDIN_FILENO, &seq[0], 1) == 1 && seq[0] == '[') {
            if (read(STDIN_FILENO, &seq[1], 1) == 1) {
                tcsetattr(STDIN_FILENO, TCSAFLUSH, &t);
                switch (seq[1]) {
                    case 'A': return Key::Up;
                    case 'B': return Key::Down;
                    case 'C': return Key::Right;
                    case 'D': return Key::Left;
                    default:  return Key::Unknown;
                }
            }
        }
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &t);
        return Key::Quit;  // bare ESC = quit
    }

    switch (c) {
        // vi keys
        case 'k': case 'K': return Key::Up;
        case 'j': case 'J': return Key::Down;
        case 'h':           return Key::Left;
        case 'l':           return Key::Right;
        // WASD
        case 'w': case 'W': return Key::Up;
        case 's': case 'S': return Key::Down;
        case 'a': case 'A': return Key::Left;
        case 'd': case 'D': return Key::Right;
        // Actions
        case 'g': case ',': return Key::PickUp;
        case 'Q':           return Key::Quit;
        case '.': case ' ': return Key::Wait;
        default:            return Key::Unknown;
    }
}
