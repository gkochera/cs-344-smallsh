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

}

void attachSIGTSTPIgnore()
{
    struct sigaction SIGTSTP_action = {0};

    SIGTSTP_action.sa_handler = handleSIGTSTPIgnore;
    sigfillset(&SIGTSTP_action.sa_mask);
    SIGTSTP_action.sa_flags = 0;
    sigaction(SIGTSTP, &SIGTSTP_action, NULL);
    fflush(stdout);
}

void handleSIGTSTPNoIgnore(int signo)
{
    FOREGROUND_ONLY = !(FOREGROUND_ONLY);
    if(FOREGROUND_ONLY)
    {
        write(STDOUT_FILENO, "\nEntering foreground-only mode (& is now ignored)\n", 51);
    }
    else
    {
        write(STDOUT_FILENO, "\nExiting foreground-only mode\n", 31);
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

void handleSIGCHLD()
{
    pid_t pid;
    int status;

    while (1)
    {
        pid = wait(status)
        if (pid == 0)
        {
            return;
        }
        else if (pid == -1)
        {
            return;
        }
        else
        {
            printf("Process %d ended with status: %d", pid, WSTOPSIG(status));
        }
    }
}

void attachSIGCHLD()
{
    struct sigaction SIGCHLD_action = {0};

    SIGCHLD_action.sa_handler = handleSIGCHLD;
    sigaddset(&SIGCHLD_action.sa_mask);
    SIGCHLD_action.sa_flags = 0;
    sigaction(SIGCHLD, &SIGCHLD_action, NULL);
    fflush(stdout);
}