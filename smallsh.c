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
static char * checkForPidExpansion (char* token)
{
    char * expansionPosition;
    char * thisToken = (char *)calloc(strlen(token), sizeof(char));
    strcpy(thisToken, token);

    // Check to see if the $$ expansion appears at least once in the token
    if (((expansionPosition = strstr(thisToken, "$$")) != NULL))
    {

        // if it does, we get the pid for smallsh, create a string with it and
        // start the process of replacing it as many times as it occurs
        int smallshPid = getpid();
        char * smallshPidAsString = (char*)calloc(8, sizeof(char));
        sprintf(smallshPidAsString, "%d", smallshPid);

        // this will hold the new token we return
        char * newToken = (char*)calloc(strlen(token) + strlen(smallshPidAsString), sizeof(char));
        
        // We check to see if strstr() returned a position for $$ if it did, we replace it
        while (expansionPosition != NULL)
        {
            // cat the stuff before $$
            strncat(newToken, thisToken, (int)(expansionPosition - thisToken));

            // replace $$ with the pid of smallsh
            strncat(newToken, smallshPidAsString, strlen(smallshPidAsString));

            // cat everything after $$
            strcat(newToken, thisToken + (int)(expansionPosition - thisToken) + strlen("$$"));

            // see if there are any more occurences of $$ and if there are we repeat as necessary
            if ((expansionPosition = strstr(newToken, "$$")) != NULL)
            {
                // We copy over what we have expanded already
                thisToken = (char*)realloc(newToken, strlen(newToken));

                // and create space for the next expansion
                newToken = (char*)calloc(strlen(thisToken) + strlen(smallshPidAsString), sizeof(char));
            }
        }

        // free our dynamic memory and return the result
        free(thisToken);
        return newToken;
    } 

    // if strstr() didn't find $$ we just return the token as is
    else
    {
        return NULL;
    }
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
    char * checkedToken = NULL;
    bool dMemUsed = false;
    char * token = strtok_r(userInputAsLine, " ", &remainderOfString);

    // Handle the case where the user entered something
    while (token != NULL)
    {
        // Check the token, handle expansion of $$, set the dMemUsed flag to true if we had to expand
        // it because it uses dynamic memory.
        if ((checkedToken = checkForPidExpansion(token)) != NULL)
        {
            token = checkedToken;
            dMemUsed = true;
        }

        // Create a new pointer of length token + 1, and copy the token to it
        char * savedToken = (char *)calloc(strlen(token) + 1, sizeof(char));
        strcpy(savedToken, token);

        // Save the token to the array of tokens, and verify we dont need to expand the array
        // If we do, expand the array by double so we don't overflow.
        userInputTokens[userInputTokensIndex++] = savedToken;
        if (userInputTokensIndex == userInputTokensQuantity)
        {
            userInputTokensQuantity *= 2;
            userInputTokens = (char **)realloc(userInputTokens, userInputTokensQuantity * sizeof(char *));
        }

        // We are done with the token, so it can be freed if it used dynamic memory.
        if (dMemUsed)
        {
            free(token);
            dMemUsed = false;
        }

        // Get our next token
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
                cmd_cd(userInputAsTokens);
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
// static void _test_tokens(char** tokens)
// {
//     int i = 0;
//     while(tokens[i] != NULL)
//     {
//         printf("%s\n", tokens[i++]);
//     }
// }

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

