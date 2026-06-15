/*
    ######################################################
    ##            SHORK UTILITY - SHORKFETCH            ##
    ######################################################
    ## Functions and data relating to handle the        ##
    ## operating system's name                          ##
    ######################################################
    ## Licence: GNU GENERAL PUBLIC LICENSE Version 3    ##
    ######################################################
    ## Kali (links.sharktastica.co.uk)                  ##
    ######################################################
*/



#include "general.h"
#include "globals.h"
#ifndef EMBEDDED
#include "replacements.h"
#endif
#include "os.h"

#include <stdlib.h>
#include <string.h>



/**
 * @param u Parsed uname data
 * @param uStatus The status returned from uname attempt
 * @return String containing the OS/Linux distro's name or "unknown" if undetermined/error
 */
char *getOS(struct utsname u, int uStatus)
{
    const int osSize = 128;
    char *os = malloc(osSize);
    if (!os) return strdup("unknown");
    os[0] = '\0';

    // Try os-release
    FILE *fStream = fopen("/etc/os-release", "r");
    if (fStream)
    {
        char buffer[128];
        while (fgets(buffer, sizeof(buffer), fStream))
        {
            if (strncmp(buffer, "PRETTY_NAME=", 12) == 0)
            {
                char *extract = extractFromPoint(buffer, osSize, '=', 1);
                strncpy(os, extract, osSize - 1);
                free(extract);
                break;
            }
        }
        fclose(fStream);
    }

    // Try issue
    if (os[0] == '\0')
    {
        fStream = fopen("/etc/issue", "r");
        if (fStream)
        {
            char buffer[osSize];
            if (fgets(buffer, sizeof(buffer), fStream))
            {
                size_t len = strlen(buffer);
                if (len > 0 && buffer[len - 1] == '\n') buffer[len - 1] = '\0';
                char *p = strchr(buffer, '\\');
                if (p) *p = '\0';
                strncpy(os, buffer, osSize - 1);
                os[osSize - 1] = '\0';
            }
            fclose(fStream);
        }
    }

    // Try uname -o
    if (os[0] == '\0')
        if (uStatus == 0)
            strncpy(os, u.sysname, osSize - 1);

    // Fallback
    if (os[0] == '\0')
        strcpy(os, "unknown");

    // If the name is wrapped in apostrophes, remove them
    if (os[0] == '\'')
    {
        size_t osLen = strlen(os);
        if (osLen >= 2 && os[osLen - 1] == '\'')
        {
            memmove(os, os + 1, osLen - 2);
            os[osLen - 2] = '\0';
        }
    }
    
    if (COMPACT)
    {
        // Remove trailing bracketed substring if present
        size_t osLen = strlen(os);
        if (osLen > 0 && os[osLen - 1] == ')')
        {
            for (int i = osLen - 1; i > 0; i--)
            {
                if (os[i] == '(' && i > 0 && os[i - 1] == ' ')
                {
                    os[i - 1] = '\0';
                    break;
                }
            }
        }

#ifndef EMBEDDED

        int replaces = 0;
        for (int i = 0; i < COMPACT_OS_REPLACES_LEN; i++)
        {
            if (COMPACT_OS_REPLACES[i].standalone && replaces > 0) continue;
            else if (strstr(os, COMPACT_OS_REPLACES[i].match))
            {
                char *tmp = findReplace(os, osSize, COMPACT_OS_REPLACES[i].match, COMPACT_OS_REPLACES[i].replacement);
                strncpy(os, tmp, osSize - 1);
                os[osSize - 1] = '\0';
                free(tmp);
                replaces++;
            }
        }

#endif

    }

    return os;
}
