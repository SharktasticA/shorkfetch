/*
    ######################################################
    ##            SHORK UTILITY - SHORKFETCH            ##
    ######################################################
    ## Functions and data relating to handle the        ##
    ## operating system's name                          ##
    ######################################################
    ## Licence: GNU GENERAL PUBLIC LICENSE Version 3    ##
    ######################################################
    ## Kali (links.sharktastica.co.uk)                  ##
    ######################################################
*/



#ifndef OS
#define OS

#include <sys/utsname.h>



#define OS_LEN              128



char *getOS(struct utsname, int, char*);
char *getOSArch(struct utsname, int);

#endif
