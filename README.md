# smallsh – A Minimal Unix-Like Shell in C

`smallsh` is a compact Unix-style shell implemented in C. It supports command parsing, background/foreground processes, I/O redirection, and custom signal handling. This project demonstrates core systems programming skills: process control, signals, parsing, and manual memory management.

---

## Features

- **Command prompt & parsing**
  - Prompt: `: `
  - Up to 2048 characters per line
  - Up to 512 arguments
  - Ignores blank lines
  - Supports comments starting with `#`

- **Built-in commands**
  - `cd [path]`
    - No argument: changes to `$HOME`
    - With argument: supports absolute and relative paths
  - `status`
    - Reports exit status or terminating signal of the last foreground process
  - `exit`
    - Exits the shell and terminates any remaining child processes

- **External commands**
  - Resolved via the system `PATH`
  - Executed with `fork()` + `execvp()`
  - Prints `command: no such file or directory` on failure

- **Foreground & background execution**
  - `cmd &` runs in the background (unless in foreground-only mode)
  - Background PIDs are printed when spawned
  - Background jobs are tracked and reported when they finish

- **Input / output redirection**
  - `< file` to redirect stdin
  - `> file` to redirect stdout (creates/truncates the file)
  - For background commands:
    - If no input redirection is provided, stdin is redirected from `/dev/null`
    - If no output redirection is provided, stdout is redirected to `/dev/null`

- **Signal handling**
  - `SIGINT` (`Ctrl+C`)
    - Shell ignores `SIGINT`
    - Foreground child processes are terminated and report `terminated by signal N`
  - `SIGTSTP` (`Ctrl+Z`)
    - Toggles *foreground-only* mode
    - In foreground-only mode, `&` is ignored
    - Prints messages when entering/exiting foreground-only mode

---

## Build

Requires a POSIX-like environment (Linux, macOS, WSL, etc.) and a C compiler:

```bash
gcc -Wall -Wextra -std=c99 -o smallsh smallsh.c
