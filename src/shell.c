/*
    ######################################################
    ##            SHORK UTILITY - SHORKFETCH            ##
    ######################################################
    ## Functions and data relating to handling the      ##
    ## terminal's shell                                 ##
    ######################################################
    ## Licence: GNU GENERAL PUBLIC LICENSE Version 3    ##
    ######################################################
    ## Kali (links.sharktastica.co.uk)                  ##
    ######################################################
*/



#include "shell.h"

#include <libgen.h>
#include <stdlib.h>
#include <string.h>



/**
 * @return String containing the shell's name or "unknown" if
 *         undetermined/error
 */
char *getShell(void)
{
    char *shell = getenv("SHELL");
    if (!shell || shell[0] == '\0') 
        shell = strdup("unknown");
    else
        shell = strdup(basename(shell));
    return shell;
}
