/*
    ######################################################
    ##            SHORK UTILITY - SHORKFETCH            ##
    ######################################################
    ## Functions and data relating to handle the Linux  ##
    ## kernel version                                   ##
    ######################################################
    ## Licence: GNU GENERAL PUBLIC LICENSE Version 3    ##
    ######################################################
    ## Kali (links.sharktastica.co.uk)                  ##
    ######################################################
*/



#include "globals.h"
#include "kernel.h"

#include <stdlib.h>
#include <string.h>
#include <sys/utsname.h>



/**
 * @param u Parsed uname data
 * @param uStatus The status returned from uname attempt
 * @return String containing the kernel version or "unknown" if
 *         undetermined/error
 */
char *getKernel(struct utsname u, int uStatus)
{
    if (uStatus != 0)
        return strdup("unknown");
    const char *src = u.release;

    if (!COMPACT)
    {
        char *release = malloc(strlen(src) + 1);
        if (!release)
            return strdup("unknown");
        strcpy(release, u.release);
        return release; 
    }
    else
    {
        // Strip out any suffixes
        int i = 0;
        while (src[i])
        {
            char c = src[i];
            if (!((c >= '0' && c <= '9') || c == '.'))
                break;
            i++;
        }

        char *release = malloc(i + 1);
        if (!release)
            return strdup("unknown");
        memcpy(release, src, i);
        release[i] = '\0';
        return release;
    }
}
