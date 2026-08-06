/*
    ######################################################
    ##                  SHORK UTILITY                   ##
    ######################################################
    ## General, utility functions for SHORK Utilities & ##
    ## SHORK ENTERTAINMENT                              ##
    ######################################################
    ## Licence: GNU GENERAL PUBLIC LICENSE Version 3    ##
    ######################################################
    ## Kali (links.sharktastica.co.uk)                  ##
    ######################################################
*/



#ifndef GENERAL
#define GENERAL

#include <dirent.h>
#include <stdio.h>



typedef struct {
    int pid;
    char name[256];
} Process;



#define TASK_COMM_LEN       24



char *bytesToReadable(const char *, const long long);
char *captureProgramOutput(const char *, const size_t);
char *extractFromPoint(char *, size_t, char, int);
int fileExists(const char*);
char *findErase(const char *, const size_t, const char *);
char *findReplace(const char *, const size_t, const char *, const char *);
int formatNewLines(char *, int, char *, int);
float fSqrt(float);
char *getBinDir(void);
Process getParentProcess(int);
struct winsize getTerminalSize(void);
int isFileExecutable(char*, struct dirent*);
int isNumeric(const char*, const int);
int isProgramInstalled(char*, int);
int iSqrt(int);
void limitLines(char*, const int);
int loadCSVLine(char*, char *[], int);
int natCmp(const void*, const void*);
int procExists(const char*, const int);
int readHexFile(const char*);
char *removeBrackets(const char*, const size_t);
void splitText(char*, char*[], int);

#endif
