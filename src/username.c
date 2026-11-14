/*
    ######################################################
    ##            SHORK UTILITY - SHORKFETCH            ##
    ######################################################
    ## Functions and data relating to handle the user's ##
    ## username                                         ##
    ######################################################
    ## Licence: GNU GENERAL PUBLIC LICENSE Version 3    ##
    ######################################################
    ## Kali (links.sharktastica.co.uk)                  ##
    ######################################################
*/



#include <stdlib.h>
#include <string.h>



/**
 * @return String containing the current username; "unknown" if
 *         undetermined/error
 */
char *getUsername(void)
{
    char *username = getenv("USER");
    if (!username || username[0] == '\0')
        username = getenv("LOGNAME");
    if (!username || username[0] == '\0') 
        username = strdup("unknown");
    return username;
}
