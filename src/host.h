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



#ifndef HOST
#define HOST

#define HOST_LEN        256
#define VAL_BUFFER_LEN  128



static const char *DMI_JUNK[] = {
    "To be filled by O.E.M.",
    "Not Available",
    "None",
    "No Asset Tag",
    "Default string"
};
static const int DMI_JUNK_LEN = sizeof(DMI_JUNK) / sizeof(DMI_JUNK[0]);



char *getHost(void);
int inWSL(void);
int isDMIValJunk(const char*);

#endif
