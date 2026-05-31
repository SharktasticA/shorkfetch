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



#define CPUINFO_BUFFER_LEN  1024
#define UARCH_LEN           128
#define VENDOR_LEN          16
#define NAME_LEN            128



typedef enum
{
    UNKNOWN = 0,
    ARM,
    POWER,
    RISCV,
    X86
} CPU_ARCH;



typedef struct {
    // Major architecture (all)
    CPU_ARCH arch;
    // Micro architecture (RISC-V)
    char *uarch;
    // Vendor name (ARM, x86)
    char *vendor;
    // Model or architecture name (all)
    char *name;
    // Family number (x86)
    int family;
    // Model number (x86)
    int model;
    // Stepping number (x86)
    int stepping;
    // Clock frequency in MHz (POWER, some RISC-V, x86)
    float freq;
    // Processor index count (ARM, POWER, some RISC-V, x86)
    int index;
    // Maximum physical ID (x86)
    int maxPhysID;
    // Physical core count (x86)
    int cores;
    // Logical thread count (RISC-V, x86)
    int threads;
    // Cache size in KB (x86)
    int cacheSize;
    // Floating-point unit present (0=n;1=y) (x86)
    int hasFPU;
    // Hyper-Threading present (0=n;1=y) (x86)
    int hasHT;
} CPU_DATA;



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
    [0x61] = "Apple",
    [0x69] = "Intel",
    [0xC0] = "Ampere"
};

// Hardcoded search needles to use when looking for a GPU name inside a CPU
// name
static const char *GPU_FROM_CPU_NEEDLES[] = {
    "with Radeon",
    "w/ Radeon",
    " RADEON",
};
static const int GPU_FROM_CPU_NEEDLES_LEN = sizeof(GPU_FROM_CPU_NEEDLES) / sizeof(GPU_FROM_CPU_NEEDLES[0]);



char *cleanCPUName(const char *, size_t);
CPU_DATA *getCPU(char *, char **);
char *interpretCPU(CPU_DATA*);

#endif
