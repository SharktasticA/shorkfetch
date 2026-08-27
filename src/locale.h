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



#ifndef LOCALE
#define LOCALE

#define LOCALES_LEN     128

typedef struct
{
    char *locales;
    int count;
} LOCALES;



LOCALES *getLocales(void);

#endif
