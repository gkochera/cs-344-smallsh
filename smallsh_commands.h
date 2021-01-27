/*
Author: George Kochera
Date Creatd: 1/26/2021
Filename: smallsh_commands.h
Description: Contains the function prototypes for handling the core commands of 
'exit', 'cd' and 'status' in smallsh
*/

void cmd_exit();
void cmd_cd();
void cmd_status(int* status);
void cmd_other(char ** tokens, int* status);