/*
    ######################################################
    ##            SHORK UTILITY - SHORKFETCH            ##
    ######################################################
    ## Functions and data relating to handling CPUs     ##
    ######################################################
    ## Licence: GNU GENERAL PUBLIC LICENSE Version 3    ##
    ######################################################
    ## Kali (links.sharktastica.co.uk)                  ##
    ######################################################
*/



#ifndef CPU
#define CPU

#include <stdio.h>



typedef enum
{
    UNKNOWN,
    ARM,
    POWER,
    RISCV,
    X86
} CPUArch;



// Hardcoded ARM CPU implementer values to allow basic ARM CPU vendor
// identification
static const char *ARM_IMPLEMENTERS[193] = {
    [0x00] = "Reserved",
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
    [0xC0] = "Ampere"
};



char *cleanCPUName(const char *, size_t, int);
char *getCPU(char *, char **);

#endif
