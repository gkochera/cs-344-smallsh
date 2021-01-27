/*
Author: George Kochera
Date Creatd: 1/25/2021
Filename: smallsh.c
Description: Contains the core functions to display the smallsh shell and handle text input
*/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "smallsh_commands.h"

/*
Gets user input up to the enter key, discards the newline character at the end
*/
static char * getUserInput()
{
    char inputChar;
    int inputBufferSize = 4;
    int inputIndex = 0;
    char * input = (char*)calloc(inputBufferSize, sizeof(char));
    while((inputChar = fgetc(stdin)) != '\n')
    {
        input[inputIndex++] = inputChar;
        if (inputIndex - 1 >= inputBufferSize)
        {
            inputBufferSize *= 2;
            input = (char*)realloc(input, inputBufferSize * sizeof(char));
        }
    }
    input[inputIndex] = '\0';
    return input;
}

/*
Checks for the $$ expansion in user input
*/

// FIXME: Handle nested expansions with surrounding characters
static char * checkForPidExpansion (char* token)
{
    if (strstr(token, "$$") != NULL)
    {
        int smallshPid = getpid();
        char * smallshPidAsString = (char*)calloc(8, sizeof(char));
        sprintf(smallshPidAsString, "%d", smallshPid);
        return smallshPidAsString;
    }
    return token;
}


/*
Tokenizes the user input with whitespace as the delimeter
*/
char ** tokenizeUserInput(char* userInputAsLine)
{
    // https://stackoverflow.com/questions/9860671/double-pointer-char-operations
    
    int userInputTokensIndex = 0;
    int userInputTokensQuantity = 4;
    char ** userInputTokens = (char **)calloc(userInputTokensQuantity, sizeof(char*));
    char * remainderOfString = NULL;
    char * token = strtok_r(userInputAsLine, " ", &remainderOfString);

    // Handle the case where the user entered something
    while (token != NULL)
    {
        token = checkForPidExpansion(token);
        char * savedToken = (char *)calloc(strlen(token) + 1, sizeof(char));
        strcpy(savedToken, token);
        userInputTokens[userInputTokensIndex++] = savedToken;
        if (userInputTokensIndex == userInputTokensQuantity)
        {
            userInputTokensQuantity *= 2;
            userInputTokens = (char **)realloc(userInputTokens, userInputTokensQuantity * sizeof(char *));
        }
        token = strtok_r(NULL, " ", &remainderOfString);

    }
    
    // Handle the special case where the user just pushes the return key
    if (userInputTokensIndex == 0)
    {
        free(userInputTokens);
        return NULL;
    }
    return userInputTokens;
}

/*
Handle the user input
*/
static void handleUserInput(char ** userInputAsTokens, int* status)
{
    // Handle blank lines entered by user/script
    if (userInputAsTokens != NULL)
    {
        // Handle comments entered by user/script
        if (userInputAsTokens[0][0] != '#')
        {
            // Handle valid lines entered by user
            char * command = userInputAsTokens[0];
            if (!(strcmp(command, "exit")))
            {
                cmd_exit();
                *status = -1;
            }
            else if (!(strcmp(command, "status")))
            {
                cmd_status(status);
                
            }
            else if (!(strcmp(command, "cd")))
            {
                cmd_cd();
                *status = 0;
            }
            else
            {
                cmd_other(userInputAsTokens, status);
            }
        }

    }

}


/*
DEBUG FUNCTION
prints all the tokens from user input
*/
static void _test_tokens(char** tokens)
{
    int i = 0;
    while(tokens[i] != NULL)
    {
        printf("%s\n", tokens[i++]);
    }
}

void smallsh()
{
    int status = 0;
    char * input = NULL;
    char ** inputTokens = NULL;
    while (true)
    {
        // Print the shell prompt, gather user input, tokenize the input
        printf(": ");
        input = getUserInput();
        inputTokens = tokenizeUserInput(input);

        // now handle the tokens
        handleUserInput(inputTokens, &status); 
        free(input);
        free(inputTokens);
    }
}

