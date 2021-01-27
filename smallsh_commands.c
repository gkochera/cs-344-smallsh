/*
Author: George Kochera
Date Creatd: 1/26/2021
Filename: smallsh_commands.c
Description: Contains the functions for handling the core commands of 'exit', 'cd' and 'status' in smallsh
*/

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

void cmd_exit()
{
    
}

void cmd_cd()
{

}

void cmd_status()
{

}

void cmd_other(char ** tokens)
{
    pid_t newPid = -5;
    int childStatus = 0;
    newPid = fork();
    switch (newPid)
    {
        case -1:
            printf("Invalid command!\n");
            break;
        case 0:
            execvp(tokens[0], tokens);
            exit(2);
            break;
        default:
            newPid = waitpid(newPid, &childStatus, 0);
            break;
    }

}