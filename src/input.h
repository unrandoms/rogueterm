#pragma once
#include <optional>

enum class Key {
    Up, Down, Left, Right,
    PickUp,     // g or ,
    Quit,       // Q or ESC
    Wait,       // . or space
    Unknown
};

// Configure terminal for raw single-keypress input (Linux termios)
void input_init();
// Restore terminal settings
void input_restore();

// Returns the next key; blocks until one is available.
Key read_key();
