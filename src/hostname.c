/*
    ######################################################
    ##            SHORK UTILITY - SHORKFETCH            ##
    ######################################################
    ## Functions and data relating to handle the        ##
    ## system's hostname                                ##
    ######################################################
    ## Licence: GNU GENERAL PUBLIC LICENSE Version 3    ##
    ######################################################
    ## Kali (links.sharktastica.co.uk)                  ##
    ######################################################
*/



#include "hostname.h"

#include <string.h>



/**
 * @param u Parsed uname data
 * @param uStatus The status returned from uname attempt
 * @return String containing the hostname; "unknown" if undetermined/error
 */
char *getHostname(struct utsname u, int uStatus)
{
    if (uStatus == 0)
        return strdup(u.nodename);
    return strdup("unknown");
}
