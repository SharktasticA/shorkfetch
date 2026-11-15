/*
    ######################################################
    ##            SHORK UTILITY - SHORKFETCH            ##
    ######################################################
    ## Functions and data relating to handling the      ##
    ## computer's (host) name                           ##
    ######################################################
    ## Licence: GNU GENERAL PUBLIC LICENSE Version 3    ##
    ######################################################
    ## Kali (links.sharktastica.co.uk)                  ##
    ######################################################
*/



#include "../shorkcommon/general.h"

#include "host.h"
#ifndef NO_STR_CLEANING
#include "replacements.h"
#endif

#include <linux/limits.h>
#include <stdlib.h>
#include <string.h>



char *getHost(void)
{
    char *host = malloc(HOST_LEN);
    if (!host)
        return NULL;
    host[0] = '\0';

    size_t len = 0;
    char valBuffer[VAL_BUFFER_LEN];
    int gotVendor = 0;
    int gotName = 0;

    // Try system vendor
    if (readFile("/sys/class/dmi/id/sys_vendor", valBuffer,
            VAL_BUFFER_LEN) == 0 &&
        !isDMIValJunk(valBuffer))
    {
        len += snprintf(host + len, HOST_LEN - len, "%s", valBuffer);
        if (len > HOST_LEN - 1)
            len = HOST_LEN - 1;
        gotVendor = 1;
    }

    // Try board vendor
    if (!gotVendor && readFile("/sys/class/dmi/id/board_vendor", valBuffer,
            VAL_BUFFER_LEN) == 0 &&
        !isDMIValJunk(valBuffer))
    {
        len += snprintf(host + len, HOST_LEN - len, "%s", valBuffer);
        if (len > HOST_LEN - 1)
            len = HOST_LEN - 1;
        gotVendor = 1;
    }

    // Try product family
    if (readFile("/sys/class/dmi/id/product_family", valBuffer,
            VAL_BUFFER_LEN) == 0 &&
        !isDMIValJunk(valBuffer))
    {
        len += snprintf(host + len, HOST_LEN - len, "%s%s", len ? " " : "",
            valBuffer);
        if (len > HOST_LEN - 1)
            len = HOST_LEN - 1;
        gotName = 1;
    }

    // Try board name
    if (!gotName && readFile("/sys/class/dmi/id/board_name", valBuffer,
            VAL_BUFFER_LEN) == 0 &&
        !isDMIValJunk(valBuffer))
    {
        len += snprintf(host + len, HOST_LEN - len, "%s%s", len ? " " : "",
            valBuffer);
        if (len > HOST_LEN - 1)
            len = HOST_LEN - 1;
        gotName = 1;
    }

    // Try product name
    if (!gotName && readFile("/sys/class/dmi/id/product_name", valBuffer,
            VAL_BUFFER_LEN) == 0 &&
        !isDMIValJunk(valBuffer))
    {
        len += snprintf(host + len, HOST_LEN - len, "%s%s", len ? " " : "",
            valBuffer);
        if (len > HOST_LEN - 1)
            len = HOST_LEN - 1;
        gotName = 1;
    }

#ifndef NO_STR_CLEANING
    // Apply company name replacements
    int replaces = 0;
    for (int i = 0; i < COMPANY_REPLACES_LEN; i++)
    {
        if (COMPANY_REPLACES[i].standalone && replaces > 0)
            continue;
        else if (strstr(host, COMPANY_REPLACES[i].match))
        {
            char *tmp = findReplace(host, HOST_LEN,
                    COMPANY_REPLACES[i].match,
                    COMPANY_REPLACES[i].replacement);
            if (tmp)
            {
                strncpy(host, tmp, HOST_LEN - 1);
                host[HOST_LEN - 1] = '\0';
                free(tmp);
                replaces++;
            }
        }
    }

    // Apply generic deletions
    for (int i = 0; i < DELETIONS_LEN; i++)
    {
        const char *pattern = DELETIONS[i];
        if (!strstr(host, pattern))
            continue;
        char *tmp = findErase(host, HOST_LEN, pattern);
        if (tmp)
        {
            strncpy(host, tmp, HOST_LEN - 1);
            host[HOST_LEN - 1] = '\0';
            free(tmp);
        }
    }
#endif

    return host;
}

/**
 * Tests if a given /sys/class/dmi/id/... value is "junk".
 * @param str String to test
 * @return 1 if junk; 0 if not
 */
int isDMIValJunk(const char *str)
{
    for (int i = 0; i < DMI_JUNK_LEN; i++)
        if (strcasecmp(str, DMI_JUNK[i]) == 0)
            return 1;
    return 0;
}
