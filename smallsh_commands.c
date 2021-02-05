/*
Author: George Kochera
Date Creatd: 1/26/2021
Filename: smallsh_commands.c
Description: Contains the functions for handling the core commands of 'exit', 'cd' and 'status' in smallsh
*/

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "smallsh_sighandler.h"
#include "smallsh_structs.h"

/*
Check a set of tokens for the & operator which is used to determine if something
will run in the back ground
*/
bool runCommandInBackground(char** userInputTokens)
{
    int i = 0;
    while(userInputTokens[i] != NULL)
    {
        if ( (!(strcmp(userInputTokens[i], "&"))) && userInputTokens[i + 1] == NULL)
        {
            userInputTokens[i] = NULL;
            return true;
        }
        i++;
    }
    return false;
}

/*
Print the background process status when a message appears, then clear it.
*/
void displayTerminatedBackgroundProcess()
{
    if (SIGNAL_MESSAGE != NULL)
    {
        printf("%s", SIGNAL_MESSAGE);
        free(SIGNAL_MESSAGE);
        SIGNAL_MESSAGE = NULL;
    }
}

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
Function to find and asses output and input redirection
*/
struct smallshFileInfo* findInputRedirectionInInput(char* userInput)
{
    struct smallshFileInfo* smallshFileInfo = (struct smallshFileInfo*)calloc(1, sizeof(struct smallshFileInfo));
    char * inputRedirectionLocation = NULL;
    char * outputRedirectionLocation = NULL;
    int inputLength = 0;
    int outputLength = 0;
    smallshFileInfo->input = NULL;
    smallshFileInfo->output = NULL;

    if ((inputRedirectionLocation = strrchr(userInput, '<')) != NULL)
    {
        if (((*(inputRedirectionLocation + 1) == ' ') && (*(inputRedirectionLocation - 1) == ' ')))
        {
            if ((outputRedirectionLocation = strrchr(inputRedirectionLocation, '>')) != NULL)
            {
                if (((*(outputRedirectionLocation + 1) == ' ') && (*(outputRedirectionLocation - 1) == ' ')))
                {
                    inputLength = outputRedirectionLocation - inputRedirectionLocation - 2;
                }
            }
            else
            {
                inputLength = strlen(inputRedirectionLocation) - 1;
            }
        }
    }
    if (inputLength > 0)
    {
        smallshFileInfo->input = (char*)calloc(inputLength, sizeof(char));
        strncpy(smallshFileInfo->input, inputRedirectionLocation + 2, inputLength - 1);
    }
    if ((outputRedirectionLocation = strrchr(userInput, '>')) != NULL)
    {
        if (((*(outputRedirectionLocation + 1) == ' ') && (*(outputRedirectionLocation - 1) == ' ')))
        {
            if ((inputRedirectionLocation = strchr(outputRedirectionLocation, '<')) != NULL)
            {
                if (((*(inputRedirectionLocation + 1) == ' ') && (*(inputRedirectionLocation - 1) == ' ')))
                {
                    outputLength = inputRedirectionLocation - outputRedirectionLocation - 2;
                }
            }
            else
            {
                outputLength = strlen(outputRedirectionLocation) - 1;
            }
        }
    }
    if (outputLength > 0)
    {
        smallshFileInfo->output = (char*)calloc(outputLength, sizeof(char));
        strncpy(smallshFileInfo->output, outputRedirectionLocation + 2, outputLength - 1);
    }
    return smallshFileInfo;
}


/*
Handle input redirection if requested (used with child process)
Returns 0 on success, -1 on error.
*/
int handleInputRedirection(struct smallshFileInfo* smallshFileInfo)
{
    FILE* inputFile;
    int inputFileDescriptor;
    if (smallshFileInfo->input != NULL)
    {
        if(!strcmp(smallshFileInfo->input, "/dev/null"))
        {
            inputFileDescriptor = open("/dev/null", O_RDWR);
            dup2(inputFileDescriptor, STDIN_FILENO);
        }
        else
        {
            if((inputFile = fopen(smallshFileInfo->input, "r")) != NULL)
            {
                int inputFileDescriptor = fileno(inputFile);
                dup2(inputFileDescriptor, STDIN_FILENO);
                return 0;
            }
            else
            {
                printf("%s: no such file or directory\n", smallshFileInfo->input);
                return -1;
            }
        }
        
    }
    return 0;
}


/*
Handle output redirection if requested (used with child process)
*/
int handleOutputRedirection(struct smallshFileInfo* smallshFileInfo)
{
    FILE* outputFile;
    int outputFileDescriptor;
    if (smallshFileInfo->output != NULL)
    {
        if(!strcmp(smallshFileInfo->output, "/dev/null"))
        {
            outputFileDescriptor = open("/dev/null", O_RDWR);
            dup2(outputFileDescriptor, STDOUT_FILENO);
            return 0;
        }
        else
        {
            outputFile = fopen(smallshFileInfo->output, "a");
            int outputFileDescriptor = fileno(outputFile);
            dup2(outputFileDescriptor, STDOUT_FILENO);
            return 0;
        }
        
    }
    return 0;
}


/*
Remove redirection clauses from tokens
*/
void cleanRedirectionFromTokens(char ** tokens)
{
    bool foundRedirection = false;
    int i = 0;
    while (tokens[i] != NULL)
    {
        if ((!(strcmp(tokens[i], "<"))) || (!(strcmp(tokens[i], ">")) || foundRedirection ))
        {
            tokens[i] = NULL;
            foundRedirection = true;
        } 
        i++;
    }
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
For background processes, see if there is redirection, if not, set redirection to '/dev/null'.
*/
void setBackgroundProcessRedirection(struct smallshFileInfo* smallshFileInfo, bool runInBackground)
{
    if (runInBackground)
    {
        if (smallshFileInfo->input == NULL)
        {
            smallshFileInfo->input = (char*)calloc(strlen("/dev/null") + 1, sizeof(char));
            strcpy(smallshFileInfo->input, "/dev/null");
        }
        
        if (smallshFileInfo->output == NULL)
        {
            smallshFileInfo->output = (char*)calloc(strlen("/dev/null") + 1, sizeof(char));
            strcpy(smallshFileInfo->output, "/dev/null");
        }
    }
}

/*
Main entry point for all other commands
*/
void cmd_other(char ** tokens, int* status, struct smallshFileInfo* smallshFileInfo)
{
    pid_t newPid = -5;
    int childStatus = 0;
    bool runInBackground = runCommandInBackground(tokens);
    newPid = fork();
    
    switch (newPid)
    {
        case -1:
            printf("Could not fork!\n");
            break;
        case 0:
            setBackgroundProcessRedirection(smallshFileInfo, runInBackground);
            if (!handleInputRedirection(smallshFileInfo))
            {
                handleOutputRedirection(smallshFileInfo);
                cleanRedirectionFromTokens(tokens);
                if(execvp(tokens[0], tokens) == -1)
                {
                    printf("%s: no such file or directory\n", tokens[0]);
                    exit(1);
                };
            }
            break;
        default:
            if (!runInBackground || FOREGROUND_ONLY)
            {
                newPid = waitpid(newPid, &childStatus, 0);
                while (!WIFEXITED(childStatus) && !WIFSIGNALED(childStatus))
                {

                }
                *status = WEXITSTATUS(childStatus);
                break;
            }
            else
            {
                printf("Background PID is %d\n", newPid);
                break;
            }
            
    }
}