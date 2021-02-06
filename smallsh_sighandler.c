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

bool FOREGROUND_ONLY = false;

void handleSIGINTNoExit(int signo)
{

}

void attachSIGINTNoExit()
{
    struct sigaction SIGINT_action = {0};

    SIGINT_action.sa_handler = handleSIGINTNoExit;
    sigfillset(&SIGINT_action.sa_mask);
    SIGINT_action.sa_flags = 0;
    sigaction(SIGINT, &SIGINT_action, NULL);
    fflush(stdout);
}

void handleSIGINTExit(int signo)
{
    kill(getpid(), signo);
    write(STDOUT_FILENO, "terminated by signal ", 21);
    write(STDOUT_FILENO, &signo, sizeof(signo));
    write(STDOUT_FILENO, "\n", 1);
    fflush(stdout);
}

void attachSIGINTExit()
{
    struct sigaction SIGINT_action = {0};

    SIGINT_action.sa_handler = handleSIGINTExit;
    sigfillset(&SIGINT_action.sa_mask);
    SIGINT_action.sa_flags = 0;
    sigaction(SIGINT, &SIGINT_action, NULL);
    fflush(stdout);
}

void handleSIGTSTPIgnore(int signo)
{
    return;
}

void attachSIGTSTPIgnore()
{
    struct sigaction SIGTSTP_action = {0};
    sigemptyset(&SIGTSTP_action.sa_mask);
    sigaddset(&SIGTSTP_action.sa_mask, SIGTSTP);
    SIGTSTP_action.sa_handler = handleSIGTSTPIgnore;
    SIGTSTP_action.sa_flags = 0;
    sigaction(SIGTSTP, &SIGTSTP_action, NULL);
    fflush(stdout);
}

void handleSIGTSTPNoIgnore(int signo)
{
    FOREGROUND_ONLY = !(FOREGROUND_ONLY);
    if(FOREGROUND_ONLY)
    {
        char * message = "\nEntering foreground-only mode (& is now ignored)\n";
        write(STDOUT_FILENO, message, strlen(message));
        fflush(stdout);
    }
    else
    {
        char * message = "\nExiting foreground-only mode\n";
        write(STDOUT_FILENO, message, strlen(message));
        fflush(stdout);
    }
    

    
}

void attachSIGTSTPNoIgnore()
{
    struct sigaction SIGTSTP_action = {0};

    SIGTSTP_action.sa_handler = handleSIGTSTPNoIgnore;
    sigfillset(&SIGTSTP_action.sa_mask);
    SIGTSTP_action.sa_flags = 0;
    sigaction(SIGTSTP, &SIGTSTP_action, NULL);
    fflush(stdout);
}



