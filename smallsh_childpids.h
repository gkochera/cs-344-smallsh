/*
Author: George Kochera
Date Creatd: 2/5/2021
Filename: smallsh_childpids.c
Description: Contains the functions for handling the child PID struct, and list of pids contained within
*/

#include "smallsh_structs.h"
#include <stdbool.h>

struct childPids* initializeChildPids();
void addChildPid(int pid, struct childPids* childPids);
void pollChildPids(struct childPids* childPids);