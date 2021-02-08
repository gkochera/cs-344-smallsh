/*
Author: George Kochera
Date Creatd: 1/29/2021
Filename: smallsh_sighandler.h
Description: Contains the function prototypes for signal handling
*/

void attachSIGINT(int handlerOption);
void attachSIGTSTP(int handlerOption);

// SOURCE: http://sgeos.github.io/unix/c/signals/2016/02/24/passing-values-to-c-signal-handlers.html
// Used an external variable to keep track of the status of CTRL + Z.
#include <stdbool.h>
extern bool FOREGROUND_ONLY;

// Used as consstants for the various signal calls to eliminate some code duplication
#define IGNORE 0
#define NOIGNORE 1
#define EXIT 0
#define NOEXIT 1