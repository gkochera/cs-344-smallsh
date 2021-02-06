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
#include <stdlib.h>

/*
Sets up the child pid list for keeping track of child processes
*/
struct childPids* initializeChildPids()
{

    // We use dynamic memory to create the struct that will hold the child pids and length of that list
    struct childPids* childPids = (struct childPids*)calloc(1, sizeof(struct childPids));

    // We start with a length of 2
    childPids->lengthOfPids = 2;

    // We then use more dynamic memory to store the list of pids, operates just like an array.
    childPids->pids = (int*)calloc(childPids->lengthOfPids, sizeof(int));

    // Return the pointer to the newly created childPids struct
    return childPids;
}

/*
Adds a PID to the child pid array and resizes it if necessary.
*/
void addChildPid(int pid, struct childPids* childPids)
{
    // Setup to increment using i, recordedPid will also be used to determine when we found a spot to
    // write the new pid to
    int i = 0;
    bool recordedPid = false;

    // Iterate up to the total length of the list, or stop if we recorded a pid
    while (i < childPids->lengthOfPids && !recordedPid)
    {
        // 0 indicates the cell is empty so we can record the pid there
        if (childPids->pids[i] == 0)
        {
            // Record the pid and set the recordedPid flag
            childPids->pids[i] = pid;
            recordedPid = true;
        }
        // The cell has something in it, a PID, so we need to skip it
        else
        {
            // Skip the cell
            i++;

            // If the incrementing value is equal to the length of pids, we need to grow the list
            if (i >= childPids->lengthOfPids)
            {
                // We double the size of the list and realloc it
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
    // We use an incrementing value to search for the pid, basic linear search
    int i = 0;

    // Search up to the end of the list
    while (i < childPids->lengthOfPids)
    {
        // If the pid in the current cell matches, we will set that value to zero
        // to indicate it can be used again for a new process
        if (childPids->pids[i] == pid)
        {
            childPids->pids[i] = 0;
        }
        i++;
    }
}

/*
Scans all the child PIDs in the given array and outputs a message if they are terminated.
This gets performed right before each new command prompt is displayed.
*/
void pollChildPids(struct childPids* childPids)
{
    // We iterate through the list of PIDS, childStatus holds the status value from waitpid
    int i = 0;
    int childStatus = 0;

    // Iterate through the total length of the array
    while (i < childPids->lengthOfPids)
    {
        // If the cell has a value over 0, we treat it as a valid pid
        if (childPids->pids[i] > 0)
        {
            // We check the status of the pid, but use NOHANG so smallsh doesnt wait for the child to finish
            while(waitpid(childPids->pids[i], &childStatus, WNOHANG) > 0)
            {
                // Check to see if the child terminated naturally, or was signalled to stop
                if (WIFEXITED(childStatus) || WIFSIGNALED(childStatus))
                {
                    // If it terminated naturally, we display the pid number, and exit value
                    if (WIFEXITED(childStatus))
                    {
                        printf("background pid %d is done: exit value %d\n", childPids->pids[i], WEXITSTATUS(childStatus));
                        fflush(stdout);
                    }

                    // If it was signalled to terminate or halt, we display the pid number and the termination signal
                    if (WIFSIGNALED(childStatus))
                    {
                        printf("background pid %d is done: terminated by signal %d\n", childPids->pids[i], WTERMSIG(childStatus));
                        fflush(stdout);
                    }

                    // Since the child no longer exists, we remove it from our struct
                    removeChildPid(childPids->pids[i], childPids);
                }
            }
        }
        i++;
    }
}

