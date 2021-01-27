/*
Author: George Kochera
Date Creatd: 1/26/2021
Filename: smallsh_commands.c
Description: Contains the functions for handling the core commands of 'exit', 'cd' and 'status' in smallsh
*/

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

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
    if (newPid == 0)
    {
        execvp(tokens[0], tokens + 1);
        waitpid(newPid, &childStatus, 0);
    }

}