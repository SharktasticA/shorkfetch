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
#include "../shorkcommon/shorkmenu.h"
#include "cpu.h"

#include "host.h"
#ifndef NO_STR_CLEANING
#include "replacements.h"
#endif

#include <ctype.h>
#include <linux/limits.h>
#include <stdlib.h>
#include <string.h>



char *getHost(void)
{
    char *host = malloc(HOST_LEN);
    if (!host)
        return NULL;
    host[0] = '\0';

    int skipCleaning = 0;

    // Quick test if we're in Windows Subsystem for Linux
    int wsl = inWSL();
    if (wsl > 0)
    {
        if (wsl == 1)
            snprintf(host, HOST_LEN, "Windows Subsystem for Linux");
        else
            snprintf(host, HOST_LEN, "Windows Subsystem for Linux 2");
        skipCleaning = 1;
    }
    else
    {
        size_t len = 0;
        char vendorBuffer[VAL_BUFFER_LEN] = { '\0' };
        char prodFamilyBuffer[VAL_BUFFER_LEN] = { '\0' };
        char boardNameBuffer[VAL_BUFFER_LEN] = { '\0' };

        // Try system vendor
        if (readFile("/sys/class/dmi/id/sys_vendor", vendorBuffer,
                VAL_BUFFER_LEN) == 0 &&
            !isDMIValJunk(vendorBuffer))
        {
            len += snprintf(host + len, HOST_LEN - len, "%s", vendorBuffer);
            if (len > HOST_LEN - 1)
                len = HOST_LEN - 1;
        }

        // Try board vendor
        if (len == 0 && readFile("/sys/class/dmi/id/board_vendor",
                vendorBuffer, VAL_BUFFER_LEN) == 0 &&
            !isDMIValJunk(vendorBuffer))
        {
            len += snprintf(host + len, HOST_LEN - len, "%s", vendorBuffer);
            if (len > HOST_LEN - 1)
                len = HOST_LEN - 1;
        }

        // Check if we're in a virtualisation platform
        if (!strncasecmp(host, "vmware", 6))
        {
            snprintf(host, HOST_LEN, "VMware virtual machine");
            skipCleaning = 1;
        }
        else
        {
            // Get both product family and board name so we can compare them
            readFile("/sys/class/dmi/id/product_family", prodFamilyBuffer,
                VAL_BUFFER_LEN);
            int prodFamilyJunk = isDMIValJunk(prodFamilyBuffer);
            readFile("/sys/class/dmi/id/board_name", boardNameBuffer,
                VAL_BUFFER_LEN);
            int boardNameJunk = isDMIValJunk(boardNameBuffer);

            if (!prodFamilyJunk && boardNameJunk)
            {
                len += snprintf(host + len, HOST_LEN - len,
                    "%s%s", len ? " " : "", prodFamilyBuffer);
                if (len > HOST_LEN - 1)
                    len = HOST_LEN - 1;
            }
            else if (prodFamilyJunk && !boardNameJunk)
            {
                len += snprintf(host + len, HOST_LEN - len,
                    "%s%s", len ? " " : "", boardNameBuffer);
                if (len > HOST_LEN - 1)
                    len = HOST_LEN - 1;
            }
            // If both aren't "junk", we see if they are similar then pick
            // the longest. Otherwise we default to product family.
            else if (!prodFamilyJunk && !boardNameJunk)
            {
                const char *chosenBuffer = prodFamilyBuffer;
                if (strncasecmp(prodFamilyBuffer, boardNameBuffer, 4) ==
                    0 && strlen(boardNameBuffer) > strlen(prodFamilyBuffer))
                    chosenBuffer = boardNameBuffer;

                len += snprintf(host + len, HOST_LEN - len, "%s%s",
                    len ? " " : "", chosenBuffer);
                if (len > HOST_LEN - 1)
                    len = HOST_LEN - 1;
            }

#ifndef NO_STR_CLEANING
            skipCleaning = isIBMAndChanged(vendorBuffer, host);
#endif
        }
    }

#ifndef NO_STR_CLEANING
    if (!skipCleaning)
    {
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
    }

    // Apply compact replacements
    if (COMPACT)
    {
        int replaces = 0;
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
    }
#endif

    return host;
}

/**
 * Tests if we're running inside a Windows Subsystem for Linux environment
 * by checking the kernel version, and returns the WSL version.
 * @return 1 if inside WSL1; 2 if inside WSL2; 0 if not; -1 if error
 */
int inWSL(void)
{
    char buffer[256] = { 0 };
    FILE *fStream = fopen("/proc/version", "r");
    if (!fStream)
        return -1;

    size_t len = fread(buffer, 1, 255, fStream);
    fclose(fStream);
    buffer[len] = '\0';

    // WSL kernels should always have "Microsoft" or "microsoft"
    if (!strcasestr(buffer, "microsoft"))
        return 0;

    // WSL2's looks like "6.18.58.1-microsoft-standard-WSL2"
    if (strcasestr(buffer, "wsl2"))
        return 2;

    // WSL1's looks like "4.4.0-43-Microsoft"
    if (strstr(buffer, "-Microsoft"))
        return 1;

    return 0;
}

#ifndef NO_STR_CLEANING
/**
 * Checks if the given vendor is "IBM", and if it is, tries to override the
 * given host name via an IBM type number dictionary.
 * @param vendor Vendor name buffer to test for "IBM"
 * @param host Host name to potentially replace
 * @return 1 if IBM and host replaced; 0 if not IBM or host was not replaced
 */
int isIBMAndChanged(const char *vendor, char *host)
{
    // Only proceed if vendor and host strings are present, and vendor
    // starts  with "IBM"
    if (!vendor || !host || strncmp(vendor, "IBM", 3) != 0)
        return 0;

    // Only proceed if host is at least 4 chars (the size of an IBM type
    // number) and doesn't already contain "ThinkPad"
    size_t hostLen = strlen(host);
    if (hostLen < 4 || strcasestr(host, "ThinkPad"))
        return 0;

    // Make sure the 4 chars are numeric
    for (int i = 0; i < 4; i++)
        if (!isdigit((unsigned char)host[i]))
            return 0;

    char type[5];
    memcpy(type, host, 4);
    type[4] = '\0';

    // If host is exactly 7 chars, we should have type+model
    /*char model[4];
    if (hostLen == 7)
    {
        memcpy(model, host + 4, 3);
        model[3] = '\0';
    }*/

    for (int i = 0; i < IBM_TYPES_LEN; i++)
    {
        if (strcmp(type, IBM_TYPES[i].type) == 0)
        {
            char family[VAL_BUFFER_LEN] = {0};
            if (IBM_TYPES[i].family == PF_THINKPAD)
                snprintf(family, VAL_BUFFER_LEN, "ThinkPad ");
            snprintf(host, HOST_LEN, "IBM %s%s", family, IBM_TYPES[i].name);
            return 1;
        }
    }

    return 0;
}
#endif

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
