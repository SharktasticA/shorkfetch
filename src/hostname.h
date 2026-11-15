/*
    ######################################################
    ##            SHORK UTILITY - SHORKFETCH            ##
    ######################################################
    ## Functions and data relating to handling the      ##
    ## system's hostname                                ##
    ######################################################
    ## Licence: GNU GENERAL PUBLIC LICENSE Version 3    ##
    ######################################################
    ## Kali (links.sharktastica.co.uk)                  ##
    ######################################################
*/



#ifndef HOSTNAME
#define HOSTNAME

#include <sys/utsname.h>



char *getHostname(struct utsname, int);

#endif
