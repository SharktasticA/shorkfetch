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



#include "../shorkcommon/colours.h"
#include "../shorkcommon/general.h"
#include "../shorkcommon/shorkmenu.h"

#include "globals.h"
#include "palette.h"

#include <stdio.h>
#include <sys/ioctl.h>



/**
 * @return ColourPalette struct containing completed strings for printing a
 *         "normal" and "bold" line of colours
 */
ColourPalette getColourPalette(void)
{
    ColourPalette palette;

    // size = number of chars per colour box
    int size = 3;
    if (!CONFIG.noArt)
    {
        if ((COMPACT && TERM_SIZE.ws_col < 32) ||
            (!COMPACT && TERM_SIZE.ws_col < 35))
            size = 1;
        else if (COMPACT || (!COMPACT && TERM_SIZE.ws_col < 43))
            size = 2;
    }
    else
    {
        if (TERM_SIZE.ws_col < 16)
            size = 1;
        else if (COMPACT || (!COMPACT && TERM_SIZE.ws_col < 24))
            size = 2;
    }

    if (size == 1)
    {
        snprintf(palette.baseCols, 128,
            "\033[%sm \033[%sm \033[%sm \033[%sm \033[%sm \033[%sm "
            "\033[%sm \033[%sm \033[%sm", 
            COL_BAK_BLACK,
            COL_BAK_RED,
            COL_BAK_GREEN,
            COL_BAK_YELLOW,
            COL_BAK_BLUE,
            COL_BAK_MAGENTA,
            COL_BAK_CYAN,
            COL_BAK_WHITE,
            COL_RESET);
        snprintf(palette.brightCols, 128,
            "\033[%sm \033[%sm \033[%sm \033[%sm \033[%sm \033[%sm "
            "\033[%sm \033[%sm \033[%sm", 
            COL_BAK_BRIGHT_BLACK,
            COL_BAK_BRIGHT_RED,
            COL_BAK_BRIGHT_GREEN,
            COL_BAK_BRIGHT_YELLOW,
            COL_BAK_BRIGHT_BLUE,
            COL_BAK_BRIGHT_MAGENTA,
            COL_BAK_BRIGHT_CYAN,
            COL_BAK_BRIGHT_WHITE,
            COL_RESET);
    }
    else if (size == 2)
    {
        snprintf(palette.baseCols, 128,
            "\033[%sm  \033[%sm  \033[%sm  \033[%sm  \033[%sm  \033[%sm  "
            "\033[%sm  \033[%sm  \033[%sm", 
            COL_BAK_BLACK,
            COL_BAK_RED,
            COL_BAK_GREEN,
            COL_BAK_YELLOW,
            COL_BAK_BLUE,
            COL_BAK_MAGENTA,
            COL_BAK_CYAN,
            COL_BAK_WHITE,
            COL_RESET);
        snprintf(palette.brightCols, 128,
            "\033[%sm  \033[%sm  \033[%sm  \033[%sm  \033[%sm  \033[%sm  "
            "\033[%sm  \033[%sm  \033[%sm", 
            COL_BAK_BRIGHT_BLACK,
            COL_BAK_BRIGHT_RED,
            COL_BAK_BRIGHT_GREEN,
            COL_BAK_BRIGHT_YELLOW,
            COL_BAK_BRIGHT_BLUE,
            COL_BAK_BRIGHT_MAGENTA,
            COL_BAK_BRIGHT_CYAN,
            COL_BAK_BRIGHT_WHITE,
            COL_RESET);
    }
    else
    {
        snprintf(palette.baseCols, 128,
            "\033[%sm   \033[%sm   \033[%sm   \033[%sm   \033[%sm   "
            "\033[%sm   \033[%sm   \033[%sm   \033[%sm", 
            COL_BAK_BLACK,
            COL_BAK_RED,
            COL_BAK_GREEN,
            COL_BAK_YELLOW,
            COL_BAK_BLUE,
            COL_BAK_MAGENTA,
            COL_BAK_CYAN,
            COL_BAK_WHITE,
            COL_RESET);
        snprintf(palette.brightCols, 128,
            "\033[%sm   \033[%sm   \033[%sm   \033[%sm   \033[%sm   "
            "\033[%sm   \033[%sm   \033[%sm   \033[%sm", 
            COL_BAK_BRIGHT_BLACK,
            COL_BAK_BRIGHT_RED,
            COL_BAK_BRIGHT_GREEN,
            COL_BAK_BRIGHT_YELLOW,
            COL_BAK_BRIGHT_BLUE,
            COL_BAK_BRIGHT_MAGENTA,
            COL_BAK_BRIGHT_CYAN,
            COL_BAK_BRIGHT_WHITE,
            COL_RESET);
    }

    return palette;
}
