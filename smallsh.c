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

static char * getUserInput()
{
    char inputChar;
    int inputBufferSize = 128;
    int inputIndex = 0;
    char * input = (char*)calloc(inputBufferSize, sizeof(char));
    while((inputChar = fgetc(stdin)) != '\0')
    {
        input[inputIndex++] = inputChar;
        if (inputIndex >= inputBufferSize)
        {
            inputBufferSize *= 2;
            input = (char*)realloc(input, inputBufferSize * sizeof(char));
        }
    }
    return input;
}

void smallsh()
{
    bool running = true;
    char * input = NULL;
    while (running)
    {
        printf(": ");
        input = getUserInput();
        printf("You entered %s\n", input);
        free(input);
    }
}

