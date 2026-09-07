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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



/**
 * Gets the system locale(s).
 * @return LOCALES pointer containing the locales and locale count; NULL if
 *         no locales found or invalid
 */
LOCALES *getLocales(void)
{
    LOCALES *result = malloc(sizeof(LOCALES));
    if (!result)
        return NULL;
    result->count = 0;

    // Get raw locales value from setlocale
    setlocale(LC_ALL, "");
    const char *slRet = setlocale(LC_ALL, NULL);
    if (!slRet)
    {
        free(result);
        return NULL;
    }
    char *localesRaw = strdup(slRet);

    // If localesRaw contains '=' or ';', it likely contains multiple
    // locales
    if (strpbrk(localesRaw, "=;") != NULL)
    {
        result->locales = calloc(1, LOCALES_LEN);
        if (!result->locales)
        {
            free(localesRaw);
            return NULL;
        }

        char *prevPos;
        char *semiTok = strtok_r(localesRaw, ";", &prevPos);
        while (semiTok)
        {
            char *equalsTok = strchr(semiTok, '=');
            if (equalsTok)
            {
                // Skip LC_COLLATE so we don't get "C" as a locale
                if (strncmp(semiTok, "LC_COLLATE=", 11) == 0)
                {
                    semiTok = strtok_r(NULL, ";", &prevPos);
                    continue;
                }

                char needle[128];
                snprintf(needle, sizeof(needle), "%s, ", equalsTok + 1);
                if (strstr(result->locales, needle) == NULL)
                {
                    strcat(result->locales, needle);
                    result->count++;
                }
            }
            semiTok = strtok_r(NULL, ";", &prevPos);
        }

        size_t len = strlen(result->locales);
        if (len >= 2)
            result->locales[len - 2] = '\0';

        free(localesRaw);
    }
    // ...if not, it should only contain one locale
    else
    {
        result->locales = localesRaw;
        result->count = 1;
    }

    if (result->count == 0)
    {
        free(result->locales);
        free(result);
        return NULL;
    }
    return result;
}
