/*
    ######################################################
    ##            SHORK UTILITY - SHORKFETCH            ##
    ######################################################
    ## Functions and data relating to handling the      ##
    ## operating system's name                          ##
    ######################################################
    ## Licence: GNU GENERAL PUBLIC LICENSE Version 3    ##
    ######################################################
    ## Kali (links.sharktastica.co.uk)                  ##
    ######################################################
*/



#include "../shorkcommon/general.h"
#include "../shorkcommon/shorkmenu.h"

#include "globals.h"
#ifndef NO_STR_CLEANING
#include "replacements.h"
#endif
#include "os.h"

#include <stdlib.h>
#include <string.h>



/**
 * @param u Parsed uname data
 * @param uStatus The status returned from uname attempt
 * @param osArch OS architecture substr to add (can be NULL to skip)
 * @return String containing the OS/Linux distro's name; NULL if
 *         undetermined/error
 */
char *getOS(struct utsname u, int uStatus, char *osArch)
{
    char *os = malloc(OS_LEN);
    if (!os)
        return NULL;
    os[0] = '\0';

    // Try os-release
    FILE *fStream = fopen("/etc/os-release", "r");
    if (fStream)
    {
        char buffer[OS_LEN];
        while (fgets(buffer, sizeof(buffer), fStream))
        {
            if (strncmp(buffer, "PRETTY_NAME=", 12) == 0)
            {
                char *extract = extractFromPoint(buffer, OS_LEN, '=');
                strncpy(os, extract, OS_LEN - 1);
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
            char buffer[OS_LEN];
            if (fgets(buffer, sizeof(buffer), fStream))
            {
                int len = strlen(buffer);
                if (len > 0 && buffer[len - 1] == '\n')
                    buffer[len - 1] = '\0';
                char *p = strchr(buffer, '\\');
                if (p)
                    *p = '\0';
                strncpy(os, buffer, OS_LEN - 1);
                os[OS_LEN - 1] = '\0';
            }
            fclose(fStream);
        }
    }

    // Try uname -o
    if (os[0] == '\0')
        if (uStatus == 0)
            strncpy(os, u.sysname, OS_LEN - 1);

    // Fallback
    if (os[0] == '\0')
    {
        free(os);
        return NULL;
    }

    // If the name is wrapped in apostrophes, remove them
    if (os[0] == '\'')
    {
        int osLen = strlen(os);
        if (osLen >= 2 && os[osLen - 1] == '\'')
        {
            memmove(os, os + 1, osLen - 2);
            os[osLen - 2] = '\0';
        }
    }
    
    if (COMPACT)
    {
        // Remove trailing bracketed substring if present
        int osLen = strlen(os);
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

#ifndef NO_STR_CLEANING
        int replaces = 0;
        for (int i = 0; i < COMPACT_OS_REPLACES_LEN; i++)
        {
            if (COMPACT_OS_REPLACES[i].standalone && replaces > 0)
                continue;
            else if (strstr(os, COMPACT_OS_REPLACES[i].match))
            {
                char *tmp = findReplace(os, OS_LEN,
                    COMPACT_OS_REPLACES[i].match,
                    COMPACT_OS_REPLACES[i].replacement);
                strncpy(os, tmp, OS_LEN - 1);
                os[OS_LEN - 1] = '\0';
                free(tmp);
                replaces++;
            }
        }
#endif
    }
    
    // Append kernel architecture (if present)
    if (osArch && osArch[0] != '\0')
    {
        size_t prevLen = strlen(os);
        size_t newLen = prevLen + strlen(osArch) + 2;

        char *tmp = realloc(os, newLen);
        if (tmp)
        {
            os = tmp;
            snprintf(os + prevLen, newLen - prevLen, " %s", osArch);
        }
    }

    return os;
}

/**
 * @param u Parsed uname data
 * @param uStatus The status returned from uname attempt
 * @return String containing the kernel's architecture; NULL if
 *         undetermined/error
 */
char *getOSArch(struct utsname u, int uStatus)
{
    size_t len = 0;
    if (uStatus == 0)
        len = strnlen(u.machine, sizeof u.machine);

    char *osArch = malloc(len + 1);
    if (!osArch)
        return NULL;

    if (len > 0)
        memcpy(osArch, u.machine, len);
    osArch[len] = '\0';
    return osArch;
}
