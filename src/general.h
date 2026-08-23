/*
    ######################################################
    ##                  SHORK UTILITY                   ##
    ######################################################
    ## General, utility functions for SHORK Utilities & ##
    ## SHORK ENTERTAINMENT                              ##
    ######################################################
    ## Revision B                                       ##
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
} PROCESS;

typedef struct
{
    char *str;
    int len;
    int lines;
} WORD_WRAPPED;



#define BREAK_CHARS_LEN     9
#define MAX_CMD_ARGS        8
#define TASK_COMM_LEN       24



// What characters general functions like wordWrap can use as places to make
// a soft wrap
static const char BREAK_CHARS[BREAK_CHARS_LEN] = { " _-+,./\\" };



char *bytesToReadable(const char *, const long long);
char *captureProgramOutput(const char *, const int);
int countSubstrs(const char *, const char *);
int csvAppend(char*, int, const char*);
int csvRemove(char*, const char*);
char *extractFromPoint(char *, int, char);
int fileExists(const char*);
char *findErase(const char *, const int, const char *);
char *findReplace(const char *, const int, const char *, const char *);
int formatNewLines(char *, int, char *, int);
float fSqrt(float);
char *getBinDir(void);
PROCESS getParentProcess(int);
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
char *removeBrackets(const char*, const int);
int runCmd(const char*, ...);
void splitText(char*, char*[], int);
WORD_WRAPPED *wordWrap(char*, int, char*, int, int);

#endif
