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



// /proc/cpuinfo read buffer string length
#define CPUINFO_BUFFER_LEN      4096
// Flag value for PHYS_IDS.noUniquePhysIDs when PHYS_IDS.uniquePhysIDs is
// unreliable and should be ignored
#define IGNORE_UNIQUE_PHYS_IDS   -1
// CPU_DATA.flags string length
#define FLAGS_LEN               1536
// CPU_DATA.name string length
#define NAME_LEN                128
// CPU_DATA.uarch string length
#define UARCH_LEN               128
// PHYS_IDS.uniquePhysIDs array size
#define UNIQUE_PHYS_IDS_SIZE     128
// CPU_DATA.vendor string length
#define VENDOR_LEN              16



typedef enum
{
    UNKNOWN = 0,
    ARM,
    POWER,
    RISCV,
    X86
} CPU_ARCH;



typedef struct {
    // Unique physical IDs recorded
    int uniquePhysIDs[UNIQUE_PHYS_IDS_SIZE];
    // Number of unique physical IDs recorded
    int noUniquePhysIDs;
    // Highest phyiscal ID found
    int maxPhysID;
} PHYS_IDS;

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
    // Physical IDs (x86)
    PHYS_IDS physIDs;
    // Physical core count (x86)
    int cores;
    // Logical thread count (RISC-V, x86)
    int threads;
    // Cache size in KB (x86)
    int cacheSize;
    // CPU flags (x86)
    char flags[FLAGS_LEN];
    // Physical address size (x86)
    int physAddrSize;
    // Virtual address size (x86)
    int virtAddrSize;
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

#ifndef EMBEDDED

// Hardcoded search needles to use when looking for a GPU name inside a CPU
// name
static const char *GPU_FROM_CPU_NEEDLES[] = {
    "with Radeon",
    "w/ Radeon",
    " RADEON",
};
static const int GPU_FROM_CPU_NEEDLES_LEN = sizeof(GPU_FROM_CPU_NEEDLES) / sizeof(GPU_FROM_CPU_NEEDLES[0]);

#endif



char *cleanCPUName(const char *, size_t);
CPU_DATA *getCPU(char *, char **);
int hasFlag(const CPU_DATA*, const char*);
char *interpretCPU(CPU_DATA*);

#endif
