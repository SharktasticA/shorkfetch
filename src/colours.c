/*
    ######################################################
    ##            SHORK UTILITY - SHORKFETCH            ##
    ######################################################
    ## ANSI escape code colour definitions              ##
    ######################################################
    ## Licence: GNU GENERAL PUBLIC LICENSE Version 3    ##
    ######################################################
    ## Kali (links.sharktastica.co.uk)                  ##
    ######################################################
*/



#include "colours.h"
#include "general.h"
#include "globals.h"

#include <stdlib.h>
#include <string.h>



/**
 * @return ColourPalette struct containing completed strings for printing a
 *         "normal" and "bold" line of colours
 */
ColourPalette getColourPalette(void)
{
    ColourPalette palette;
    if (COMPACT)
    {
        snprintf(palette.normalCols, 128, "\033[%sm  \033[%sm  \033[%sm  \033[%sm  \033[%sm  \033[%sm  \033[%sm  \033[%sm  \033[%sm", 
            COL_BAK_BLACK,
            COL_BAK_RED,
            COL_BAK_GREEN,
            COL_BAK_YELLOW,
            COL_BAK_BLUE,
            COL_BAK_MAGENTA,
            COL_BAK_CYAN,
            COL_BAK_WHITE,
            COL_RESET);
        snprintf(palette.boldCols, 128, "\033[%sm  \033[%sm  \033[%sm  \033[%sm  \033[%sm  \033[%sm  \033[%sm  \033[%sm  \033[%sm", 
            COL_BAK_BOLD_BLACK,
            COL_BAK_BOLD_RED,
            COL_BAK_BOLD_GREEN,
            COL_BAK_BOLD_YELLOW,
            COL_BAK_BOLD_BLUE,
            COL_BAK_BOLD_MAGENTA,
            COL_BAK_BOLD_CYAN,
            COL_BAK_BOLD_WHITE,
            COL_RESET);
    }
    else
    {
        snprintf(palette.normalCols, 128, "\033[%sm   \033[%sm   \033[%sm   \033[%sm   \033[%sm   \033[%sm   \033[%sm   \033[%sm   \033[%sm", 
            COL_BAK_BLACK,
            COL_BAK_RED,
            COL_BAK_GREEN,
            COL_BAK_YELLOW,
            COL_BAK_BLUE,
            COL_BAK_MAGENTA,
            COL_BAK_CYAN,
            COL_BAK_WHITE,
            COL_RESET);
        snprintf(palette.boldCols, 128, "\033[%sm   \033[%sm   \033[%sm   \033[%sm   \033[%sm   \033[%sm   \033[%sm   \033[%sm   \033[%sm", 
            COL_BAK_BOLD_BLACK,
            COL_BAK_BOLD_RED,
            COL_BAK_BOLD_GREEN,
            COL_BAK_BOLD_YELLOW,
            COL_BAK_BOLD_BLUE,
            COL_BAK_BOLD_MAGENTA,
            COL_BAK_BOLD_CYAN,
            COL_BAK_BOLD_WHITE,
            COL_RESET);
    }
    return palette;
}

/**
 * Validates if the given potential accent colour option supplied matches a
 * known colour.
 * @return ANSI escape code for colour; NULL if not found
 */
char *validateColour(char *input)
{
    char *colour = NULL;

    if (strcmp(input, "black") == 0)
        colour = strdup("\033[" COL_FOR_BLACK "m");
    else if (strcmp(input, "blue") == 0)
        colour = strdup("\033[" COL_FOR_BLUE "m");
    else if (strcmp(input, "bold_blue") == 0)
        colour = strdup("\033[" COL_FOR_BOLD_BLUE "m");
    else if (strcmp(input, "bold_cyan") == 0)
        colour = strdup("\033[" COL_FOR_BOLD_CYAN "m");
    else if (strcmp(input, "bold_green") == 0)
        colour = strdup("\033[" COL_FOR_BOLD_GREEN "m");
    else if (strcmp(input, "bold_magenta") == 0)
        colour = strdup("\033[" COL_FOR_BOLD_MAGENTA "m");
    else if (strcmp(input, "bold_red") == 0)
        colour = strdup("\033[" COL_FOR_BOLD_RED "m");
    else if (strcmp(input, "bold_white") == 0)
        colour = strdup("\033[" COL_FOR_BOLD_WHITE "m");
    else if (strcmp(input, "bold_yellow") == 0)
        colour = strdup("\033[" COL_FOR_BOLD_YELLOW "m");
    else if (strcmp(input, "cyan") == 0)
        colour = strdup("\033[" COL_FOR_CYAN "m");
    else if (strcmp(input, "green") == 0)
        colour = strdup("\033[" COL_FOR_GREEN "m");
    else if (strcmp(input, "grey") == 0)
        colour = strdup("\033[" COL_FOR_GREY "m");
    else if (strcmp(input, "magenta") == 0)
        colour = strdup("\033[" COL_FOR_MAGENTA "m");
    else if (strcmp(input, "off") == 0)
        colour = strdup("");
    else if (strcmp(input, "red") == 0)
        colour = strdup("\033[" COL_FOR_RED "m");
    else if (strcmp(input, "white") == 0)
        colour = strdup("\033[" COL_FOR_WHITE "m");
    else if (strcmp(input, "yellow") == 0)
        colour = strdup("\033[" COL_FOR_YELLOW "m");

    return colour;
}
