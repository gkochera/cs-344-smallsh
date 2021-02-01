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