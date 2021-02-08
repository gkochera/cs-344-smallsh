# Assignment 3: Smallsh
*  Written by: George Kochera
*  Date: 2/6/2021
*  For: OSU - CS 344 (Winter 2021)

# Overview

This program is an interactive shell. It's a simple shell with only 3 commands. cd allows you to change the directory, exit allows you to exit the program and status will display the last status of the last executed command. The shell will also allow the user to run other commands that are available on the system. It does this by forking off a child process and then executing that process in the child.

Program has been run by itself and with the p3testscript on both OS1 and my local machine. Confirmed working and passing on both.

# Program Operation

The command prompt is displayed as a simple ': ' followed by a blinking cursor. Simply enter the command you wish to run at the prompt and press enter to execute. The command prompt will display a new prompt when the command is done executing in the foreground. The command prompt will display immediately after execution of a background process.

The general structure of a command is as follows:

command [- command arguments] [< input redirection target] [> output redirection target] [&]

Command arguments are organic command arguments used when running the command. An example of this would be 'ls -la' where '-la' is the argument.

The input/output redirection targets are used to redirect input and output to a command. They must have whitespace on both sides of them to work. An example of this would be

cat test.txt > output.txt

In this example, we take the normal output of 'cat test.txt' which would be displayed on the screen, and instead, write it to the file 'output.txt'.

Lastly, the & operator, indicates the command should be run as a background process. This only works when the program is not in FOREGROUND ONLY mode.

The program additionally can handle signals from the terminal. It's programmed to handle SIGINT (normally CTRL + C), and SIGTSTP (normally CTRL + Z).

When the user signals SIGINT:

- The shell will ignore it.
- Background children will ignore it.
- Foreground children will self-terminate and a message will immediately be displayed on the screen.

When the user signals SIGTSTP:

- The shell will display an informative message indicating the FOREGROUND ONLY mode status. Sending another signal will continue to toggle this mode.
- All children will ignore SIGTSTP.

# Building

Program can be built 2 ways from the Linux terminal:

## Make

```bash
make smallsh
```

## GCC

```bash
gcc --std=gnu99 -o smallsh main.c smallsh.c smallsh_commands.c smallsh_sighandler.c smallsh_childpids.c
```

# Running

Simply running the command once the files are compiled is sufficient

```bash
./smallsh
```