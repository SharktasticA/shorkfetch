/*
    ######################################################
    ##            SHORK UTILITY - SHORKFETCH            ##
    ######################################################
    ## Functions and data relating to handling system   ##
    ## locale                                           ##
    ######################################################
    ## Licence: GNU GENERAL PUBLIC LICENSE Version 3    ##
    ######################################################
    ## Kali (links.sharktastica.co.uk)                  ##
    ######################################################
*/



#include "locale.h"

#include <locale.h>
#include <stdlib.h>
#include <string.h>



/**
 * Gets the system locale.
 * @return String containing the locale; NULL if no locale/system doesn't
 *         support locales
 */
char *getLocale(void)
{
    setlocale(LC_ALL, "");
    const char *result = setlocale(LC_ALL, NULL);
    return result ? strdup(result) : NULL;
}
