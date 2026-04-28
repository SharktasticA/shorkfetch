/*
    ######################################################
    ##            SHORK UTILITY - SHORKFETCH            ##
    ######################################################
    ## Hard-coded PCI DEVICE IDs that should be         ##
    ## excluded from any processing.                    ##
    ######################################################
    ## Licence: GNU GENERAL PUBLIC LICENSE Version 3    ##
    ######################################################
    ## Kali (links.sharktastica.co.uk)                  ##
    ######################################################
*/



#ifndef EXCLUSIONS
#define EXCLUSIONS

static const int EXCLUDED_PCI_DIDS[] = {
    0x2a03  // Mobile GM965/GL960 Integrated Graphics Controller (secondary)
};
static const int EXCLUDED_PCI_DIDS_LEN = sizeof(EXCLUDED_PCI_DIDS) / sizeof(EXCLUDED_PCI_DIDS[0]);

#endif
