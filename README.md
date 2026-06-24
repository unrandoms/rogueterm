# rogueterm

A terminal roguelike written in C++17. Procedural dungeons, shadow-casting FOV, and turn-based combat — no ncurses required.

## Build

```bash
cmake -B build
cmake --build build
./build/rogueterm          # random seed
./build/rogueterm 12345    # fixed seed
```

Requires: a C++17 compiler (GCC 9+ or Clang 9+), CMake 3.14+, POSIX terminal (Linux/macOS).

## Controls

| Key         | Action            |
|-------------|-------------------|
| WASD / hjkl | Move / attack     |
| g or ,      | Pick up item      |
| . or Space  | Wait (skip turn)  |
| Q or Esc    | Quit              |

## Gameplay

Descend five dungeon floors by finding and stepping on the stairs (`>`). Each floor is generated fresh via BSP room placement with L-shaped corridors. Enemies include **Rats** (fast, weak), **Goblins** (balanced chasers), and **Orcs** (slow, powerful). Collect **Health Potions** (`!`) to restore HP, **Swords** (`)`) to increase attack, and **Shields** (`[`) to increase defense. Reach floor 5 and take the stairs to win.

## Architecture

The renderer uses raw ANSI escape sequences — `ESC[H` / `ESC[2J` family — with a software double-buffer: only cells that differ between frames are re-emitted, so the terminal never flickers on redraws. No ncurses dependency means the binary runs on any POSIX terminal emulator that speaks ANSI/VT100 without requiring the curses library or its terminfo database. Input is handled via Linux `termios` in raw mode so single keypresses are read without waiting for Enter.
