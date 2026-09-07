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
#include "colours.h"

#include <string.h>



char CHAR_BULLET = '*';
char *COL_ACCENT = NULL;
char *COL_BULLET = NULL;
char *COL_PCT_HIGH = NULL;
char *COL_PCT_LOW = NULL;
char *COL_PCT_MED = NULL;
int COMPACT = 0;
char *HOME;
char MAX_UNIT = 'p';
int NO_ESC = 0;
int SHORK_LINE = 0;
int SHOW_SHORK = 1;
struct winsize TERM_SIZE;
int WAYLAND_PRESENT;
int X11_PRESENT;
char *XDG_CURRENT_DESKTOP;
