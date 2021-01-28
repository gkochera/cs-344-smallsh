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
#include <string.h>

/*
Helper function for cd
Detokenizes the user input back into its original string minus the initial command
*/
char * detokenizeUserInputAfterCommand(char** userInputTokens)
{
    int i = 1;
    char * detokenizedUserInput = NULL;
    while (userInputTokens[i] != NULL)
    {
        if (i == 1)
        {
            detokenizedUserInput =(char*) calloc(strlen(userInputTokens[i] + 3), sizeof(char));
        }
        else
        {
            detokenizedUserInput =(char*) realloc(detokenizedUserInput, (strlen(detokenizedUserInput) + strlen(userInputTokens[i] + 3) * sizeof(char)));
        }
        strcat(detokenizedUserInput, userInputTokens[i]);
        i++;
        if (userInputTokens[i] != NULL)
        {
            strcat(detokenizedUserInput, " ");
        }
    }
    return detokenizedUserInput;
}

/*
Main entry point for the exit command
*/
void cmd_exit()
{
    exit(EXIT_SUCCESS);
}


/*
Main entry point for the cd command
*/
//TODO: Need to handle paths with spaces
void cmd_cd(char ** userInputAsTokens)
{
    char * path = detokenizeUserInputAfterCommand(userInputAsTokens);
    if(path == NULL)
    {
        chdir(getenv("HOME"));
    }
    else if(chdir(path))
    {
        printf("Directory does not exist.\n");
    }
}


/*
Main entry point for the status command
*/
void cmd_status(int* status)
{
    printf("exit status %d\n", *status);
}


/*
Main entry point for all other commands
*/
void cmd_other(char ** tokens, int* status)
{
    pid_t newPid = -5;
    int childStatus = 0;
    newPid = fork();
    switch (newPid)
    {
        case -1:
            printf("Could not fork!\n");
            break;
        case 0:
            execvp(tokens[0], tokens);
            exit(1);
            break;
        default:
            newPid = waitpid(newPid, &childStatus, 0);
            while (!WIFEXITED(childStatus) && !WIFSIGNALED(childStatus))
            {

            }
            *status = WEXITSTATUS(childStatus);
            break;
    }
}