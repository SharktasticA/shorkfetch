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



#include "globals.h"
#include "conf.h"



Config CONFIG = {
    '*',
    "bright_cyan",
    "cyan",
    "red",
    "green",
    "yellow",
    0,
    "head,---,os,krn,upt,pkgs,loc,scn,de,wm,trm,sh,cpu,gpu,ram,swap,dsk,root,lip, ,clrs, ",
    'p',
    NORMAL,
    0,
    0,
    0,
    0
};
char *HOME;
int SHORK_LINE = 0;
int WAYLAND_PRESENT;
int X11_PRESENT;
char *XDG_CURRENT_DESKTOP;
