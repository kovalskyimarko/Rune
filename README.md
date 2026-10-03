# Terminal Text Editor

Rune is a lighweight terminal editor written for unix system in C.
This project is a personal learning endeavor to understand low-level text editing, file I/O, terminal input handling, memory managment etc.

---

## Features (Implemented)

* Terminal raw mode to ensure that characters are handled by me
* Insertion / Deletion of characters
* Basic navigation:
  * Arrow keys
  * Home, end keys
  * pgup, pgdown
* Different modes (Expanded later):
  * INSERT_MODE - mode where characters are inserted into the buffer
  * NORMAL_MODE - mode where user can control movement/ insertion, deletion of characters through various
  keybinds
  * VISUAL_MODE - mode where user select text, copyies, cuts it or removes
  * COMMAND_LINE_MODE - mode where a user writes into command line, to perform different commands or search
  text
* Renderer which draws text, status line, messages, line numbers etc
* Highlighting for C supports four different themes (Expanded later)
* File handling: save, open files. With either :e (to open/create), :w (save file), ./rune filename (to open from terminal)
* .runerc config hadnles personal user preferences (Expanded later)

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

---

## NORMAL_MODE
* Movement with h,j,k,l: h - move cursor left, j - move cursor down, k - move cursor up, l - move cursor right
* Movement with w,e,b: w - move forward to the start of the word, e - move to the end of the word, b - move back to the start of the word 
* Movement with $,A,0,I,^: $ - move cursor to the end of the line, A - move cursor to the end of the line, 0 - move cursor to the start of the line, I - move cursor to the first non-blank character and switch to INSERT_MODE, ^ - move cursor to the first non-blank character
* Deletion with d prefix: Currently only dw, and dd added. dw - delete a whole word, dd - delete a whole line
* Non-blocking number multiplier: If before w is pressed 3, cursor is moving to the start of 3 word. Non-blocking because it just saves multiplier into global variable, and not waits for another input. Supports certain keybinds after
* Search with f: search a character from your cursor in your current row
* Movement with G: number + G - brings you to row with that number
* Keybding y,x: copy, remove character on the cursor
* Paste with p or P: paste text from the local yankbuffer before or after the cursor
* Progress through search with n/N: if the local search buffer has text, pressing n or N will progress search forward or backwards accordingly

## VISUAL_MODE
* y,x,d - copy or cut or delete selected text

## COMMAND_LINE_MODE
* Run a command with : (Command list is written below)
* Run a search with /

---

## Commands:

* `q` - quit from an editor. If a file has unsaved changes will fail
* `q!` or `CTRL + Q` - force quit from an editor.
* `:e optional: <filename>` — open a file. If a name isn't written then reload current file if it is saved
* `:w` — save current file
* `:wq optional: <filename>` or `:x optional: <filename>` - save and exit from the editor
* `:info` - writes info about current file
* `:pdw` - prints the current working directory
* `set nu/nonu/rnu/nornu` - turn on line numbers, turn off, turn on relative line numbers or off, accordingly
* `:!<command to run>` - switched from edtior window runs a command given and shows its output. Waits for next press to switch back to the editor

---

## What .runerc file supports
* `set nu/nonu/rnu/nornu` - turn on/off line numbers. Though by default this feauture is turned off
* `theme none/astra/clouds/green` - different themes. None - no highlight, astra - dark and modern theme. Clouds - pleasent light theme. Green - retro everything green theme. None is default option. IMPORTANT theme set to none doesn't dissable hl array in erow, so the memory for it is allocated. If you wish to disable it use command syntax.
* `syntax on/off` - turn on or off highlight array allocation. Default is on. Off is usefull for big log files.
* `tab_size <number>` - how many spaces is one tab. By default 4.
* `status_line full/minimum/off` - make status line write full info, only file name and whether its modified, or turn it off completely. By default its full
* `autoindent on/of` - turn on or off autoindent. By default is off

---

## Project Structure

* `input.c` — Handles keyboard input, inserting or deleting rows, chars into erow buffer
* `input_processkeys.c` — Handles the characters (moves cursor, performs different keybinds) given by input.c
* `file.c` — Handles file opening and saving
* `main.c` - Main file handles start and runs main loop
* `output.c` - Renders everything to the terminal screen
* `terminal.c` - Configures terminal window
* `syntax_parser.c` - Receives erow, and expands its highlight array, putting there a token for each letter
* `config.c` - Loads .runerc from home directory, parses it
* `xmemory.c` - Defines wrappers for malloc, realloc, strdup - xmalloc, xrealloc, xstrdup - which fail if not enough memory
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

## License

This project is open-source under the MIT License. See `LICENSE` for details.
