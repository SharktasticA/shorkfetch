/*
    ######################################################
    ##            SHORK UTILITY - SHORKFETCH            ##
    ######################################################
    ## Hard-coded CPU implementer values to allow basic ##
    ## vendor identification for ARM CPUs.              ##
    ######################################################
    ## Licence: GNU GENERAL PUBLIC LICENSE Version 3    ##
    ######################################################
    ## Kali (links.sharktastica.co.uk)                  ##
    ######################################################
*/



#ifndef IMPLEMENTERS
#define IMPLEMENTERS

static const char* ARM_IMPLEMENTERS[256] = {
    [0x00] = "Reserved",
    [0xC0] = "Ampere",
    [0x41] = "Arm",
    [0x42] = "Broadcom",
    [0x43] = "Cavium",
    [0x44] = "DEC",
    [0x46] = "Fujitsu",
    [0x49] = "Infineon",
    [0x4D] = "Motorola/Freescale",
    [0x4E] = "NVIDIA",
    [0x50] = "AMCC",
    [0x51] = "Qualcomm",
    [0x56] = "Marvell",
    [0x69] = "Intel",
};

#endif
