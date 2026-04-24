# Terminal Text Editor

A lightweight, terminal-based text editor written in C.
This project is a personal learning endeavor to understand low-level text editing, file I/O, and terminal input handling.

---

## Features (Implemented)

* Open and save files (`:e` and `:w`)
* Basic navigation:
  * Arrow keys (up, down, left, right)
  * Home / End
  * Page Up / Page Down
* Insert and delete characters
* Split lines and join lines at the cursor
* Supports tilde expansion (`~`) for paths
* Tracks current cursor position (`E.cx`, `E.cy`)
* Command mode and insert mode (basic)

---

## Technical Highlights & Memory Safety

Building a text editor from scratch in C presents unique challenges, particularly regarding terminal interfaces and memory management. 

* **Raw Terminal Control:** Bypasses standard I/O (canonical mode) using `termios.h` and `ioctl` to manually handle VT100 escape sequences, enabling instant keystroke processing and screen rendering.
* **Zero-Leak Memory Management:** Implements dynamic memory allocation for row buffers (`erow`) and file paths. Uses safe `realloc` patterns (temporary pointers) to prevent heap corruption and ensures complete memory freeing upon exit (`valgrind` clean).
* **Dynamic Path Expansion:** Safely handles `~` tilde expansion by dynamically calculating and allocating path lengths on the heap, preventing buffer overflow vulnerabilities common in fixed-size `char[]` buffers.

---

## Features in Progress / TODO

* Copy (`Ctrl+C`) and paste (`Ctrl+V`)
* Undo / redo
* Search and replace
* Syntax highlighting
* Mouse support
* Better error messages
* Configuration options

---

## Usage

Compile the project with:

```bash
gcc -o rune *.c
or 
make
```

Run it:

```bash
./rune
```

Commands:

* `:e <filename>` — Open a file
* `:w` — Save current file
* Navigation keys: arrow keys, Home, End, Page Up, Page Down
* Insert mode is active by default; press `Esc` to enter command mode.

---

## Project Structure

* `input.c` — Handles keyboard input, cursor movement, and editing commands
* `file.c` — Handles file opening and saving
* `main.c` - Main file handles start and runs main loop
* `output.c` - Renders everything to the terminal screen
* `terminal.c` - Configures terminal window
* `rune.h` — Shared structures and definitions 
* `Makefile` - builds everything

---

## Learning Goals

This project helps me:

* Understand low-level terminal I/O and control sequences
* Practice memory management in C
* Build a functional text editor from scratch
* Learn how real editors like `nano` and `vim` are structured

---

## Notes

* This is a **work-in-progress** project. Expect missing features and rough edges.
* Contributions and suggestions are welcome!

---

## License

This project is open-source under the MIT License. See `LICENSE` for details.
