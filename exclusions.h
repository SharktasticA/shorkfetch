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
    0x2973,     // 380 for Intel iGPU 2972
    0x2983,     // 380 for Intel iGPU 2982
    0x2993,     // 380 for Intel iGPU 2992
    0x29a3,     // 380 for Intel iGPU 29a2
    0x29b3,     // 380 for Intel iGPU 29b2
    0x29c3,     // 380 for Intel iGPU 29c2
    0x29d3,     // 380 for Intel iGPU 29d2
    0x2a03,     // 380 for Intel iGPU 2a02
    0x2a13      // 380 for Intel iGPU 2a12
};
static const int EXCLUDED_PCI_DIDS_LEN = sizeof(EXCLUDED_PCI_DIDS) / sizeof(EXCLUDED_PCI_DIDS[0]);

#endif
