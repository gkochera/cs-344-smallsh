/*
Author: George Kochera
Date Creatd: 1/26/2021
Filename: smallsh_commands.h
Description: Contains the function prototypes for handling the core commands of 
'exit', 'cd' and 'status' in smallsh
*/
#include "smallsh_structs.h"

// These functions handle the base commands that are handled by smallsh
void cmd_exit();
void cmd_cd();
void cmd_status(char* status);
void cmd_other(char ** tokens, char* status, struct smallshFileInfo* smallshFileInfo, struct childPids* childPids);

// This function is required to handle input/output redirection
struct smallshFileInfo* findInputRedirectionInInput(char* userInput);

// These functions handle the childPids struct that is used to keep track of open childPIDs
struct childPids* initializeChildPids();
void pollChildPids(struct childPids* childPids);