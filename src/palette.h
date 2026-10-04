/*
    ######################################################
    ##            SHORK UTILITY - SHORKFETCH            ##
    ######################################################
    ## Functions and data relating to handling ANSI     ##
    ## escape code colour palettes                      ##
    ######################################################
    ## Licence: GNU GENERAL PUBLIC LICENSE Version 3    ##
    ######################################################
    ## Kali (links.sharktastica.co.uk)                  ##
    ######################################################
*/



#ifndef PALLETE
#define PALLETE

typedef struct {
    char baseCols[128];
    char brightCols[128];
} ColourPalette;



ColourPalette getColourPalette(void);

#endif
