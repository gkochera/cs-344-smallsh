/*
Author: George Kochera
Date Creatd: 2/5/2021
Filename: smallsh_childpids.c
Description: Contains the functions for handling the child PID struct, and list of pids contained within
*/

#include "smallsh_structs.h"
#include <stdbool.h>
#include <stdio.h>
#include <wait.h>

/*
Sets up the child pid list for keeping track of child processes
*/
struct childPids* initializeChildPids()
{
    struct childPids* childPids = (struct childPids*)calloc(1, sizeof(struct childPids));
    childPids->lengthOfPids = 2;
    childPids->pids = (int*)calloc(childPids->lengthOfPids, sizeof(int));
    return childPids;
}

/*
Adds a PID to the child pid array and resizes it if necessary.
*/
void addChildPid(int pid, struct childPids* childPids)
{
    int i = 0;
    bool recordedPid = false;
    while (i < childPids->lengthOfPids && !recordedPid)
    {
        if (childPids->pids[i] == 0)
        {
            childPids->pids[i] = pid;
            recordedPid = true;
        }
        else
        {
            i++;
            if (i >= childPids->lengthOfPids)
            {
                childPids->lengthOfPids *= 2;
                childPids->pids = (int*)realloc(childPids->pids, childPids->lengthOfPids);
            }
        }
        
    }
}

/*
Removes a child PID from the array
*/
static void removeChildPid(int pid, struct childPids* childPids)
{
    int i = 0;
    while (i < childPids->lengthOfPids)
    {
        if (childPids->pids[i] == pid)
        {
            childPids->pids[i] = 0;
        }
        i++;
    }
}

/*
Scans all the child PIDs in the given array and outputs a message if they are terminated
*/
void pollChildPids(struct childPids* childPids)
{
    int i = 0;
    int childStatus = 0;
    while (i < childPids->lengthOfPids)
    {
        if (childPids->pids[i] > 0)
        {
            while(waitpid(childPids->pids[i], &childStatus, WNOHANG) > 0)
            {
                if (WIFEXITED(childStatus) || WIFSIGNALED(childStatus))
                {
                    if (WIFEXITED(childStatus))
                    {
                        printf("background pid %d is done: exit value %d\n", childPids->pids[i], WEXITSTATUS(childStatus));
                        fflush(stdout);
                    }
                    if (WIFSIGNALED(childStatus))
                    {
                        printf("background pid %d is done: terminated by signal %d\n", childPids->pids[i], WTERMSIG(childStatus));
                        fflush(stdout);
                    }
                    removeChildPid(childPids->pids[i], childPids);
                }
            }
        }
        i++;
    }
}

