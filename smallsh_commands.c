/*
Author: George Kochera
Date Creatd: 1/26/2021
Filename: smallsh_commands.c
Description: Contains the functions for handling the core commands of 'exit', 'cd' and 'status' in smallsh. Additionally contains the
functions required for determining redirection, tokeninizing and detokenizing user input and checking to see if a command should
execute in the background.
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
#include "smallsh_childpids.h"

/*
Check a set of tokens for the & operator which is used to determine if something
will run in the back ground

Returns:
true: run the command in the background
false: run the command in the foreground
*/
bool runCommandInBackground(char** userInputTokens)
{
    // Setup for iteration
    int i = 0;

    // Iterate through the entire list of user input tokens
    while(userInputTokens[i] != NULL)
    {
        // If we get a & token, that means & has whitepaces around it, and if the next token is null, that means
        // & was at the end of the user input so this is valid
        if ( (!(strcmp(userInputTokens[i], "&"))) && userInputTokens[i + 1] == NULL)
        {
            // We remove the & because we don't want to pass it on to execvp() and return true
            userInputTokens[i] = NULL;
            return true;
        }
        i++;
    }
    // If we didn't find a trailing &, we return false indicating the program should NOT run in the background.
    return false;
}


/*
Helper function for cd
Detokenizes the user input back into its original string minus the initial command.
NOTE: This only detokenizes the token after the first, so it skips the initial command.
Returns a char * containing the detokenized user input.
*/
char * detokenizeUserInputAfterCommand(char** userInputTokens)
{

    // We setup a standard iterator for the process of detokenizing user input, the final string
    // will go in detokenizedUserInput
    int i = 1;
    char * detokenizedUserInput = NULL;

    // Iterate through every token until we reach null which means there are no more tokens
    while (userInputTokens[i] != NULL)
    {
        // If we are on the first token, dynamically allocate some memory to hold the user input
        if (i == 1)
        {
            detokenizedUserInput =(char*) calloc(strlen(userInputTokens[i] + 3), sizeof(char));
        }

        // If we are any subsequent token, we realloc the string to accomodate the added string length
        else
        {
            detokenizedUserInput =(char*) realloc(detokenizedUserInput, (strlen(detokenizedUserInput) + strlen(userInputTokens[i] + 3) * sizeof(char)));
        }

        // We then concatenate the next token
        strcat(detokenizedUserInput, userInputTokens[i]);

        // Advance the iterator
        i++;

        // We add spaces in between each of the concatenated tokens since that was what was removed in the first place, though
        // we do not add a space at the end.
        if (userInputTokens[i] != NULL)
        {
            strcat(detokenizedUserInput, " ");
        }
    }

    // Return the string pointer when we are done.
    return detokenizedUserInput;
}


/*
Function to find and assess output and input redirection. The function accepts the untokenized user input
and then evaluates the entire length of user input for attempts at redirection. It does this by looking for the
'<' or '>' characters surrounded by spaces. If they don't exist, the function does nothing. If either exist,
a smallshFileInfo struct is created with the relevant portions of redirection saved to the input and output
attributes of the struct.
*/
struct smallshFileInfo* findInputRedirectionInInput(char* userInput)
{
    /* 
    We create a few variables to help us with parsing the user input string. We create the struct to hold the desired
    redirection. xRedirectionLocation holds the location of the < or > character in userinput. xLength holds the length
    of the string minus padding whitespaces of the actual redirection target and the input/output attributes of smallshFileInfo
    hold the redirection target as a string.
    */
    struct smallshFileInfo* smallshFileInfo = (struct smallshFileInfo*)calloc(1, sizeof(struct smallshFileInfo));
    char * inputRedirectionLocation = NULL;
    char * outputRedirectionLocation = NULL;
    int inputLength = 0;
    int outputLength = 0;
    smallshFileInfo->input = NULL;
    smallshFileInfo->output = NULL;

    // Look for the input redirection operator and save its location
    if ((inputRedirectionLocation = strrchr(userInput, '<')) != NULL)
    {
        // See if the input redirection operator has spaces padding it
        if (((*(inputRedirectionLocation + 1) == ' ') && (*(inputRedirectionLocation - 1) == ' ')))
        {
            // See if there is additional output redirection following it
            if ((outputRedirectionLocation = strrchr(inputRedirectionLocation, '>')) != NULL)
            {
                // If there is more redirection, we record the input redirection length up to the > operator
                // accounting for whitespace
                if (((*(outputRedirectionLocation + 1) == ' ') && (*(outputRedirectionLocation - 1) == ' ')))
                {
                    inputLength = outputRedirectionLocation - inputRedirectionLocation - 2;
                }
            }
            // If there is no additional redirection, we set the input length to be one less than the 
            // remaining string, accounting for whitespace
            else
            {
                inputLength = strlen(inputRedirectionLocation) - 1;
            }
        }
    }
    
    // If we found something, we copy it to our struct
    if (inputLength > 0)
    {
        // Create a place to put the string
        smallshFileInfo->input = (char*)calloc(inputLength, sizeof(char));

        // Copy the string, minus any whitespace.
        strncpy(smallshFileInfo->input, inputRedirectionLocation + 2, inputLength - 1);
    }

    // We know repeat the entire process, just as before, but as the inverse of how we looked for 
    // input redirection... line for line its the same, so there is an opportunity here for some code
    // consolidation but its a touchy algorithm and I couldn't come up with a cut and dry way of handling it.
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

    // If we found some output redirection, we copy that to our smallshFileInfo struct
    if (outputLength > 0)
    {
        smallshFileInfo->output = (char*)calloc(outputLength, sizeof(char));
        strncpy(smallshFileInfo->output, outputRedirectionLocation + 2, outputLength - 1);
    }

    // We then return the new struct
    return smallshFileInfo;
}


/*
Handle input redirection if requested (used with child process)
SOURCE: https://stackoverflow.com/questions/14846768/in-c-how-do-i-redirect-stdout-fileno-to-dev-null-using-dup2-and-then-redirect
Returns:

0 on success
-1 on error.
*/
int handleInputRedirection(struct smallshFileInfo* smallshFileInfo)
{
    // We setup some variables to hold our FILE* and input file number descriptor.
    FILE* inputFile;
    int inputFileDescriptor;

    // We examine the targetted input and see if it was specified
    if (smallshFileInfo->input != NULL)
    {

        // We handle input redirection to /dev/null in a slightly different manner
        // see SOURCE for details
        if(!strcmp(smallshFileInfo->input, "/dev/null"))
        {
            inputFileDescriptor = open("/dev/null", O_RDWR);
            dup2(inputFileDescriptor, STDIN_FILENO);
        }

        // If we are taking input redirection from anywhere else, we follow this path.
        else
        {
            // We attempt to open the targetted file for input redirection
            if((inputFile = fopen(smallshFileInfo->input, "r")) != NULL)
            {
                // If we are successful, we duplicate STDIN_FILENO to the new
                // file descriptor and return 0
                int inputFileDescriptor = fileno(inputFile);
                dup2(inputFileDescriptor, STDIN_FILENO);
                return 0;
            }

            // If we couldn't open the file for any particular reason, we display
            // an error message, flush STDOUT and return -1
            else
            {
                printf("%s: no such file or directory\n", smallshFileInfo->input);
                fflush(stdout);
                return -1;
            }
        }
        
    }
    // ... if it wasn't specified, we just return 0.
    return 0;
}


/*
Handle output redirection if requested (used with child process)
SOURCE: https://stackoverflow.com/questions/14846768/in-c-how-do-i-redirect-stdout-fileno-to-dev-null-using-dup2-and-then-redirect
The source was helpful in determining how to handle the /dev/null redirection.

Returns: 0 on success
*/
int handleOutputRedirection(struct smallshFileInfo* smallshFileInfo)
{
    // We set a few variables to hold the FILE* and an integer for the output
    // file descriptor
    FILE* outputFile;
    int outputFileDescriptor;

    // We examine the output redirection target in the smallshFileInfo struct for 
    // redirection and handle it as necessary.
    if (smallshFileInfo->output != NULL)
    {
        // If we redirect to /dev/null, we handle that in a slightly different manner
        if(!strcmp(smallshFileInfo->output, "/dev/null"))
        {
            outputFileDescriptor = open("/dev/null", O_RDWR);
            dup2(outputFileDescriptor, STDOUT_FILENO);
            return 0;
        }

        // If we direct to anything else, we just attempt to open the file in append mode, get the
        // file number and then duplicate the STDOUT_FILENO to the new file number for the opened file
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
Remove redirection clauses from tokens, accepts the tokens and returns nothing.
*/
void cleanRedirectionFromTokens(char ** tokens)
{
    // We set a flag for if we found redirection and an integer for iteration
    bool foundRedirection = false;
    int i = 0;

    // We then iterate through the entire set of user tokens looking for the redirection operators
    // if we find one, we set the found redirection flag, and then set the rest of the tokens to NULL
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
Main entry point for the exit command, we never return from this function. 
The program will attempt to close all open childPids and then will exit gracefully
*/
void cmd_exit(struct childPids* childPids)
{

    // We iterate through the child pids list for the entire list, killing each child
    int i = 0;
    while (i < childPids->lengthOfPids)
    {
        if (childPids->pids[i] > 0)
        {
            kill(childPids->pids[i], SIGKILL);
        }
        i++;
    }

    // Exit gracefully.
    exit(EXIT_SUCCESS);
}


/*
Main entry point for the cd command, to process it we just need the user input as tokens
*/
void cmd_cd(char ** userInputAsTokens)
{
    // We need to detokenize the userinput so that we can pass the target path as one string
    char * path = detokenizeUserInputAfterCommand(userInputAsTokens);

    // detokenizeUserInputAfterCommand will return NULL if the user just types 'CD' and the 
    // desired behavior of this is to simply navigate to the home directory.
    if(path == NULL)
    {
        chdir(getenv("HOME"));
    }

    // If the user specified something after 'CD', we use chdir() to attempt to navigate to it
    // if the directory does not exist, we print a message
    else if(chdir(path))
    {
        printf("Directory does not exist.\n");
        fflush(stdout);
    }
}


/*
Main entry point for the status command, simply requires the current status
for the smallsh program
*/
void cmd_status(char* status)
{
    // Print the status to the screen and flush the standard output buffer
    printf("%s", status);
    fflush(stdout);
}


/*
For background processes, see if there is redirection, if not, set redirection to '/dev/null'.
We need the smallshFileInfo struct to set redirection to /dev/null and the flag for running in the background
*/
void setBackgroundProcessRedirection(struct smallshFileInfo* smallshFileInfo, bool runInBackground)
{
    // We check to see if the process should be run in the background
    if (runInBackground)
    {
        // We then examine the smallshFileInfo struct to see if redirection was specified for input,
        // if there is none, we specify /dev/null
        if (smallshFileInfo->input == NULL)
        {
            smallshFileInfo->input = (char*)calloc(strlen("/dev/null") + 1, sizeof(char));
            strcpy(smallshFileInfo->input, "/dev/null");
        }
        
        // We repeat the same as we did for input, on the output redirection.
        if (smallshFileInfo->output == NULL)
        {
            smallshFileInfo->output = (char*)calloc(strlen("/dev/null") + 1, sizeof(char));
            strcpy(smallshFileInfo->output, "/dev/null");
        }
    }
}


/*
Main entry point for all other commands, if it isn't CD, STATUS or EXIT, this function executes
We need the tokens, the status char buffer, the smallshFileInfo struct for redirection and child pids incase we
need to add new pids to it
*/
void cmd_other(char ** tokens, char* status, struct smallshFileInfo* smallshFileInfo, struct childPids* childPids)
{
    // We set a few variables to track the created pid, childStatus, determine if the command should be run in the
    // background and then we fork off the the new process.
    pid_t newPid = -5;
    int childStatus = 0;
    bool runInBackground = runCommandInBackground(tokens);
    newPid = fork();
    
    // This is the crux of the function
    // - If the fork fails, we print out that the fork failed
    // - The child will take case 0
    // - The parent will follow default.
    switch (newPid)
    {

        // Case in which the fork fails, just print a message that it didn't work.
        case -1:
            printf("Could not fork!\n");
            fflush(stdout);
            break;

        // Child case - We handle background process redirection, basically checking to see if redirection was 
        // specified and if not, we tell it to redirect input and output to /dev/null
        case 0:
            setBackgroundProcessRedirection(smallshFileInfo, runInBackground);

            // Children running in the background should ignore SIGINT and SIGTSTP
            if (runInBackground && !FOREGROUND_ONLY)
            {
                attachSIGINT(NOEXIT);
                attachSIGTSTP(IGNORE);
            }

            // Children running in the foreground should not ignore SIGINT, but should ignore SIGTSTP
            else
            {
                attachSIGINT(EXIT);
                attachSIGTSTP(IGNORE);
            }

            // Input redirection will return -1 if we couldn't open the input redirection file so we handle that case
            if (!handleInputRedirection(smallshFileInfo))
            {
                // Output redirection will always work since if the file doesn't exist, it is simply created
                handleOutputRedirection(smallshFileInfo);

                // Clear out the redirection from the tokens since we dont want to pass those to exec.
                cleanRedirectionFromTokens(tokens);

                // Then we call exec using the remaining tokens. If it fails the if statement body executes
                if(execvp(tokens[0], tokens) == -1)
                {
                    // In the event that exec fails, we print out a message saying the file doesn't exist
                    // flush the output buffer and exit gracefully with a status code 1.
                    printf("%s: no such file or directory\n", tokens[0]);
                    fflush(stdout);
                    exit(1);
                };
            }

            // In the event that input redirection failed, we have to gracefully fail, so we do that by simply
            // exiting the child process with a status code 1.
            else
            {
                // Handle the case where someone tries to redirect a file, that cannot be accessed
                // or doesn't exist as input to a program.
                exit(1);
            }
            break;
        default:

            // We check to see if we are in foreground only mode or if we explicitly are running in the foreground
            // which is checked by examining the tokens for a trailing &
            if (!runInBackground || FOREGROUND_ONLY)
            {
                // RUN IN FOREGROUND
                // Wait for the foreground process to terminate normally or via a signal
                newPid = waitpid(newPid, &childStatus, 0);

                // Reset the status
                for (int i = 0; i < 512; i++)
                {
                    status[i] = 0;
                }

                // If terminated by a signal, set that status message and display it, flushing the output when
                // we are done
                if (WIFSIGNALED(childStatus))
                {
                    strcat(status, "terminated by signal ");
                    char stopSignal[20];
                    sprintf(stopSignal, "%d\n", WTERMSIG(childStatus));
                    strcat(status, stopSignal);
                    printf("%s", status);
                    fflush(stdout);
                }

                // If exited normally, set that status message containing the exit status
                if (WIFEXITED(childStatus))
                {
                    strcat(status, "exit status ");
                    char exitStatus[20];
                    sprintf(exitStatus, "%d\n", WEXITSTATUS(childStatus));
                    strcat(status, exitStatus);
                }
                break;
            }
            else
            {
                // RUN IN BACKGROUND
                // We print the background PID, flush the output buffer, add the PID to our list
                // and gracefully return to the main smallsh loop
                printf("Background PID is %d\n", newPid);
                fflush(stdout);
                addChildPid(newPid, childPids);
                break;
            }
            
    }
}