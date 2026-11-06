/*
    ######################################################
    ##            SHORK UTILITY - SHORKFETCH            ##
    ######################################################
    ## Functions and data relating to handling IP       ##
    ## addresses                                        ##
    ######################################################
    ## Licence: GNU GENERAL PUBLIC LICENSE Version 3    ##
    ######################################################
    ## Kali (links.sharktastica.co.uk)                  ##
    ######################################################
*/



#ifndef IP
#define IP

#define REQ_TIMEOUT     1000



static const char *STUN_SRVS[][2] = {
    { "stun.cloudflare.com",    "3478" },
    { "stun.l.google.com",      "19302" },
    { "stun.nextcloud.com",     "443" }
};
static const int STUN_SRVS_LEN = sizeof(STUN_SRVS) / sizeof(STUN_SRVS[0]);



char *getExternalIP(void);
char *getLocalIP(void);

#endif
