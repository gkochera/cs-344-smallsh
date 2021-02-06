/*
Author: George Kochera
Date Creatd: 2/1/2021
Filename: smallsh_structs.h
Description: Contains all the structures used in smallsh
*/

/*
Struct to hold file information for redirection
*/
#ifndef SMALLSHFILEINFO
#define SMALLSHFILEINFO
struct smallshFileInfo {
    char* input;
    char* output;
};
#endif

#ifndef CHILDPIDS
#define CHILDPIDS
struct childPids {
    int * pids;
    int lengthOfPids;
};
#endif
