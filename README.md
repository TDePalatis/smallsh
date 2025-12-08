# smallsh — A Minimal Unix Shell in C
A fully-functional command shell demonstrating process control, redirection, signals, and job management.

`smallsh` is a custom Unix-like shell written in C as part of the Oregon State University CS 344: Operating Systems curriculum.
It implements a core subset of Bash behavior, including:

- Command parsing
- Foreground & background execution
- Input/output redirection
- Custom built-ins (`exit`, `cd`, `status`)
- Signal handling for `SIGINT` (Ctrl-C) and `SIGTSTP` (Ctrl-Z)
- Process management using `fork()`, `exec()`, and `waitpid()`

## Features

### Command Execution
- Executes external programs using `execvp()`
- Spawns child processes via `fork()`
- Captures exit status and termination signals

### Built-In Commands

| Command | Description |
|---------|-------------|
| `exit`  | Terminates the shell, optionally killing all background processes |
| `cd`    | Changes the working directory (`cd` alone → HOME) |
| `status` | Prints the exit value or signal of the last foreground process |

### Input & Output Redirection
Supports `<` and `>`:

```
smallsh> sort < unsorted.txt > sorted.txt
```

Background commands default stdin/stdout to `/dev/null` when not specified.

### Background Processes
Commands ending with `&` run asynchronously:

```
smallsh> sleep 10 &
background pid is 12345
```

The shell tracks and reports completed background processes.

### Foreground-Only Mode (`SIGTSTP`)
Pressing Ctrl-Z toggles foreground-only mode:

```
Entering foreground-only mode (& is now ignored)
Exiting foreground-only mode
```

When active, `&` has no effect and all commands run in the foreground.

### Signal Handling
- Shell ignores SIGINT (Ctrl-C)
- Foreground child processes receive SIGINT
- SIGTSTP toggles foreground-only mode
- Background processes are not interrupted by Ctrl-C

## Key Concepts Demonstrated

- Process creation & execution (`fork`, `execvp`)
- Background job management (`waitpid`, `WNOHANG`)
- File redirection using `open`, `dup2`
- Signal handling with `sigaction`
- Parsing input into tokens
- Error handling & memory safety in C

## File Structure

```
├── smallsh.c         # Main shell implementation
├── test_smallsh.sh   # Automated test script (if included)
└── README.md         # Project documentation
```
## Build & Run

```
gcc -Wall -Wextra -o smallsh smallsh.c
./smallsh
```

## Example Usage

```
$ smallsh
: ls -l
: echo hello > out.txt
: cat < out.txt
hello
: sleep 5 &
background pid is 10293
: status
exit value 0
```

Foreground-only mode:

```
^Z
Entering foreground-only mode (& is now ignored)
```

### Automated Tests

Run the included test script (bash):

```bash
chmod +x test_smallsh.sh
./test_smallsh.sh
```

## Skills Demonstrated

- C systems programming
- POSIX process control
- Unix signals & handlers
- Shell parsing & validation
- Redirection and file descriptor management
- Managing foreground/background processes

## License

This project was developed as part of university coursework and is provided for educational and portfolio purposes.
