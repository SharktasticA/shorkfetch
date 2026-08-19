/*
    ######################################################
    ##            SHORK UTILITY - SHORKFETCH            ##
    ######################################################
    ## Functions and data relating to handle the Linux  ##
    ## kernel version                                   ##
    ######################################################
    ## Licence: GNU GENERAL PUBLIC LICENSE Version 3    ##
    ######################################################
    ## Kali (links.sharktastica.co.uk)                  ##
    ######################################################
*/



#ifndef KERNEL
#define KERNEL

#include <sys/utsname.h>



char *getKernel(struct utsname, int);

#endif
