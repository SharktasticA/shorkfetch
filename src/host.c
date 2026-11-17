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
    char venderBuffer[VAL_BUFFER_LEN] = { '\0' };
    char prodFamilyBuffer[VAL_BUFFER_LEN] = { '\0' };
    char boardNameBuffer[VAL_BUFFER_LEN] = { '\0' };

    // Try system vendor
    if (readFile("/sys/class/dmi/id/sys_vendor", venderBuffer,
            VAL_BUFFER_LEN) == 0 &&
        !isDMIValJunk(venderBuffer))
    {
        len += snprintf(host + len, HOST_LEN - len, "%s", venderBuffer);
        if (len > HOST_LEN - 1)
            len = HOST_LEN - 1;
    }

    // Try board vendor
    if (len == 0 && readFile("/sys/class/dmi/id/board_vendor",
            venderBuffer, VAL_BUFFER_LEN) == 0 &&
        !isDMIValJunk(venderBuffer))
    {
        len += snprintf(host + len, HOST_LEN - len, "%s", venderBuffer);
        if (len > HOST_LEN - 1)
            len = HOST_LEN - 1;
    }

    // Check if we're in a virtualisation platform
    if (!strncasecmp(host, "vmware", 6))
    {
        snprintf(host, HOST_LEN, "VMware virtual machine");
        return host;
    }

    // Get both product family and board name so we can compare them
    readFile("/sys/class/dmi/id/product_family", prodFamilyBuffer,
        VAL_BUFFER_LEN);
    int prodFamilyJunk = isDMIValJunk(prodFamilyBuffer);
    readFile("/sys/class/dmi/id/board_name", boardNameBuffer,
        VAL_BUFFER_LEN);
    int boardNameJunk = isDMIValJunk(boardNameBuffer);

    if (!prodFamilyJunk && boardNameJunk)
    {
        len += snprintf(host + len, HOST_LEN - len, "%s%s", len ? " " : "",
            prodFamilyBuffer);
        if (len > HOST_LEN - 1)
            len = HOST_LEN - 1;
    }
    else if (prodFamilyJunk && !boardNameJunk)
    {
        len += snprintf(host + len, HOST_LEN - len, "%s%s", len ? " " : "",
            boardNameBuffer);
        if (len > HOST_LEN - 1)
            len = HOST_LEN - 1;
    }
    // If both aren't "junk", we see if they are similar then pick the
    // longest. Otherwise we default to product family.
    else if (!prodFamilyJunk && !boardNameJunk)
    {
        const char *chosenBuffer = prodFamilyBuffer;
        if (strncasecmp(prodFamilyBuffer, boardNameBuffer, 4) == 0 &&
            strlen(boardNameBuffer) > strlen(prodFamilyBuffer))
            chosenBuffer = boardNameBuffer;

        len += snprintf(host + len, HOST_LEN - len, "%s%s", len ? " " : "",
            chosenBuffer);
        if (len > HOST_LEN - 1)
            len = HOST_LEN - 1;
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

    // Apply compact replacements
    replaces = 0;
    for (int i = 0; i < COMPACT_HOST_REPLACES_LEN; i++)
    {
        if (COMPACT_HOST_REPLACES[i].standalone && replaces > 0)
            continue;
        else if (strstr(host, COMPACT_HOST_REPLACES[i].match))
        {
            char *tmp = findReplace(host, HOST_LEN,
                COMPACT_HOST_REPLACES[i].match,
                COMPACT_HOST_REPLACES[i].replacement);
            strncpy(host, tmp, HOST_LEN - 1);
            host[HOST_LEN - 1] = '\0';
            free(tmp);
            replaces++;
        }
    }
#endif

    return host;
}

/**
 * Tests if a given /sys/class/dmi/id/... value is "junk".
 * @param str String to test
 * @return 1 if junk or empty; 0 if not
 */
int isDMIValJunk(const char *str)
{
    if (!str || str[0] == '\0')
        return 1;

    for (int i = 0; i < DMI_JUNK_LEN; i++)
        if (strcasecmp(str, DMI_JUNK[i]) == 0)
            return 1;
    return 0;
}
