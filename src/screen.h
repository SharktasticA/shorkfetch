/*
    ######################################################
    ##            SHORK UTILITY - SHORKFETCH            ##
    ######################################################
    ## Functions and data relating to handling screens  ##
    ######################################################
    ## Licence: GNU GENERAL PUBLIC LICENSE Version 3    ##
    ######################################################
    ## Kali (links.sharktastica.co.uk)                  ##
    ######################################################
*/



#ifndef SCREEN
#define SCREEN

#define SCREEN_LEN  128



typedef struct {
    // Connector name (e.g., DP-1)
    char *connector;
    // Flags if this is the primary screen
    int isPrimary;
    // Physical width (mm)
    float physX;
    // Physical height (mm)
    float physY;
    // Resolution width (px)
    int resX;
    // Resolution height (px)
    int resY;
    // Refresh rate (Hz)
    int refresh;
} Screen;



Screen *getScreens(int*);
char *interpretScreen(Screen*);

#endif
