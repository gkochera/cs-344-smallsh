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

bool FOREGROUND_ONLY = false;

void handleSIGINTNoExit(int signo)
{
    char * newline = "\n";
    write(STDOUT_FILENO, newline, strlen(newline));
    fflush(stdout);
}

void handleSIGINTExit(int signo)
{
    write(STDOUT_FILENO, "terminated by signal ", 21);
    write(STDOUT_FILENO, &signo, sizeof(signo));
    write(STDOUT_FILENO, "\n", 1);
    fflush(stdout);
    killpg(getpgrp(), signo);
}

void attachSIGINT(int handlerOption)
{
    struct sigaction SIGINT_action = {0};

    if (handlerOption == NOEXIT)
    {
        SIGINT_action.sa_handler = handleSIGINTNoExit;
    }
    else if (handlerOption == EXIT)
    {
        SIGINT_action.sa_handler = handleSIGINTExit;
    }

    sigfillset(&SIGINT_action.sa_mask);
    SIGINT_action.sa_flags = 0;
    sigaction(SIGINT, &SIGINT_action, NULL);
    fflush(stdout);
}


void handleSIGTSTPIgnore(int signo)
{

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
*/
void attachSIGTSTP(int hanlderOption)
{
    // Initialize SIGTSTP_action to be empty
    struct sigaction SIGTSTP_action = {0};
    
    // Attach the appropriate SIGTSTP handler
    if (hanlderOption == IGNORE)
    {
        SIGTSTP_action.sa_handler = handleSIGTSTPIgnore;
    }
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



