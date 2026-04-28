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
    0x2776,     // 380 for 8086:2772
    0x2782,     // 380 for 8086:2582
    0x2792,     // 380 for 8086:2592
    0x27a6,     // 380 for 8086:27ae
    0x2973,     // 380 for 8086:2972
    0x2983,     // 380 for 8086:2982
    0x2993,     // 380 for 8086:2992
    0x29a3,     // 380 for 8086:29a2
    0x29b3,     // 380 for 8086:29b2
    0x29c3,     // 380 for 8086:29c2
    0x29d3,     // 380 for 8086:29d2
    0x2a03,     // 380 for 8086:2a02
    0x2a13,     // 380 for 8086:2a12
    0x2a43,     // 380 for 8086:2a42
    0x2e03,     // 380 for 8086:2e02
    0x2e13,     // 380 for 8086:2e12
    0x2e23,     // 380 for 8086:2e22
    0x2e33,     // 380 for 8086:2e32
    0x2e43,     // 380 for 8086:2e42
    0x2e93      // 380 for 8086:2e92
};
static const int EXCLUDED_PCI_DIDS_LEN = sizeof(EXCLUDED_PCI_DIDS) / sizeof(EXCLUDED_PCI_DIDS[0]);

#endif
