/*
    ######################################################
    ##            SHORK UTILITY - SHORKFETCH            ##
    ######################################################
    ## Global variables                                 ##
    ######################################################
    ## Licence: GNU GENERAL PUBLIC LICENSE Version 3    ##
    ######################################################
    ## Kali (links.sharktastica.co.uk)                  ##
    ######################################################
*/



#ifndef GLOBALS
#define GLOBALS

#include "conf.h"

#include <sys/ioctl.h>
#include <stdlib.h>



#define OUTPUT_LEN  8192



extern Config CONFIG;
extern char *HOME;
extern int SHORK_LINE;
extern int WAYLAND_PRESENT;
extern int X11_PRESENT;
extern char *XDG_CURRENT_DESKTOP;

#endif
