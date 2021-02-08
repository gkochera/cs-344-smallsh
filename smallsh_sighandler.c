/*
Author: George Kochera
Date Creatd: 1/29/2021
Filename: smallsh_sighandler.c
Description: Contains the functions necessary for handling signals SIGIN and SIGTSPT
SIGINT is CTRL + C and SIGTSTP is CTRL + Z
*/

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <wait.h>
#include "smallsh_sighandler.h"

// This variable is a global variable that tracks whether the program is running in 
// FOREGROUND ONLY mode or not. When true, the shell cannot execute commands in the
// background and will ignore
bool FOREGROUND_ONLY = false;


/*
This is our custom signal handler for honoring the CTRL + C / SIGINT signal.
*/
void handleSIGINTExit(int signo)
{
    // In keeping with using reentrant functions, we write a message to the terminal indicating
    // that SIGINT killed the process by signal X where X is the number of the signal that fired
    // causing termiantion. We then kill the process.
    char message[300]; 
    strcat(message, "terminated by signal ");
    char signalNumber[10];
    sprintf(signalNumber, "%d", signo);
    strcat(message, signalNumber); 
    write(STDOUT_FILENO, message, strlen(message));
    fflush(stdout);
    killpg(getpgrp(), signo);
}


/*
This function attaches the appropriate signal handler for SIGINT (CTRL + C)
SOURCE: https://linux.die.net/man/2/signal
*/
void attachSIGINT(int handlerOption)
{
    // Initialize SIGINT_action to be empty
    struct sigaction SIGINT_action = {{0}};

    // In the event we need to ignore SIGINT, we will use the SIG_IGN signal handler
    if (handlerOption == NOEXIT)
    {
        SIGINT_action.sa_handler = SIG_IGN;
    }

    // We want to block all catchable signals while our handler is running
    sigfillset(&SIGINT_action.sa_mask);

    // We dont want to set any flags
    SIGINT_action.sa_flags = 0;

    // We install the selected handler for the SIGINT signal on the current process
    sigaction(SIGINT, &SIGINT_action, NULL);
}


/*
This is the signal handler for SIGTSTP in cases it should NOT be IGNORED
*/
void handleSIGTSTPNoIgnore(int signo)
{
    // Toggle the value for FOREGROUND_ONLY which is an extern variable visible globally
    FOREGROUND_ONLY = !(FOREGROUND_ONLY);

    // If we entered FOREGROUND ONLY mode, we display that message.
    if(FOREGROUND_ONLY)
    {
        char * message = "\nEntering foreground-only mode (& is now ignored)\n";
        write(STDOUT_FILENO, message, strlen(message));
        fflush(stdout);
    }

    // If we exited FOREGROUND ONLY mode, we display that message.
    else
    {
        char * message = "\nExiting foreground-only mode\n";
        write(STDOUT_FILENO, message, strlen(message));
        fflush(stdout);
    }
}


/*
This attaches the signal handler which ACTS on CTRL + Z to enforce foreground
only mode.
SOURCES: https://linux.die.net/man/2/signal
*/
void attachSIGTSTP(int hanlderOption)
{
    // Initialize SIGTSTP_action to be empty
    struct sigaction SIGTSTP_action = {{0}};
    
    // In the event we need to ignore SIGTSTP, we attach SIG_IGN to ignore the signal
    if (hanlderOption == IGNORE)
    {
        SIGTSTP_action.sa_handler = SIG_IGN;
    }

    // In the event we need to handle SIGTSTP, we attach handleSIGTSTPNoIgnore
    else if (hanlderOption == NOIGNORE)
    {
        SIGTSTP_action.sa_handler = handleSIGTSTPNoIgnore;
    }
    
    // Block all catchable signals while handleSIGTSTPNoIgnore is running
    sigfillset(&SIGTSTP_action.sa_mask);

    // Don't set any flags
    SIGTSTP_action.sa_flags = 0;

    // Install handleSIGTSTPNoIgnore as the SIGTSTP handler
    sigaction(SIGTSTP, &SIGTSTP_action, NULL);
}

