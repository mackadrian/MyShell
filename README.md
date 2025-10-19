# COMP3659-Linux_Shell

Repository for COMP3659: Operating Systems Project 1.  
**Name(s):** Mack Bautista, Jarod Dacocos, Tarun Jaswal

## About
This project is one out of the two projects that are part of the Operating Systems formal evaluation.

The goal of this project is to create our own SHELL program through the use of system calls and core C programming concepts. This shell aims to mimic the behaviour of Bash, the standard Unix shell.

## Build Instructions
To build the shell and test drivers:

```bash
# Clean previous builds
make clean

# Build the shell only
make

# Build all (shell + test drivers)
make all
```
The executable for the shell is mysh. Test drivers will be built in the same directory with names corresponding to their source files.

## Running The Program
Once mysh is built, run it as follows:
```bash
./mysh
```

You should see the prompt:
```bash
mysh$ ...
```

## Example Usage
1. Run a simple command
```bash
mysh$ ls -l
```

2. Display a file
```bash
mysh$ cat mysh.h
```

3. Pipelines
```bash
mysh$ ls -al | wc -l
42
```

## Supported Features
+ Execute single commands and pipelines
+ Input and output redirection (>, <)
+ Background jobs using &
+ Built-in commands: cd, exit, export, jobs, fg, bg
+ Job control (foreground/background process management)
+ Signal handling (Ctrl+C, Ctrl+Z)

## Limitations
+ Does not support advanced Bash features such as command substituion or shell scripting
+ Limited PATH resolution (does not handle every edge case)
+ Other signals are ignored or not fully supported