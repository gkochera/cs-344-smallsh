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

void handleSIGINT(int signo)
{
    char* message = "Caught SIGINT, sleeping for 3 seconds\n";
    write(STDOUT_FILENO, message, 38);
    sleep(3);
}

void attachSIGINTHandler()
{
    struct sigaction SIGINT_action = {0};

    SIGINT_action.sa_handler = handleSIGINT;
    sigfillset(&SIGINT_action.sa_mask);
    SIGINT_action.sa_flags = 0;
    sigaction(SIGINT, &SIGINT_action, NULL);
    fflush(stdout);
}