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
// CPU_DATA.detectedAs string length
#define DETECTED_AS_LEN         128
// Flag value for PHYS_IDS.noUniquePhysIDs when PHYS_IDS.uniquePhysIDs is
// unreliable and should be ignored
#define IGNORE_UNIQUE_PHYS_IDS  -1
// CPU_DATA.flags string length
#define FLAGS_LEN               1536
// CPU_DATA.name string length
#define NAME_LEN                128
// CPU_DATA.platform string length
#define PLATFORM_LEN            128
// CPU_DATA.processor string length
#define PROCESSOR_LEN           128
// CPU_DATA.uarch string length
#define UARCH_LEN               128
// PHYS_IDS.uniquePhysIDs array size
#define UNIQUE_PHYS_IDS_SIZE    128
// CPU_DATA.vendor string length
#define VENDOR_LEN              16



// The CPU architectures supported by SHORKFETCH
typedef enum
{
    UNKNOWN = 0,
#ifndef X86_ONLY
    ARM,
    M68K,
    MIPS,
    POWER,
    RISCV,
#endif
    X86
} CPU_ARCH;



// Datapoints for x86 physical ID
typedef struct {
    // Unique physical IDs recorded
    int uniquePhysIDs[UNIQUE_PHYS_IDS_SIZE];
    // Number of unique physical IDs recorded
    int noUniquePhysIDs;
    // Highest phyiscal ID found
    int maxPhysID;
} PHYS_IDS;

// Datapoints for a CPU being read and processed
typedef struct {
    // Major architecture (all)
    CPU_ARCH arch;
#ifndef X86_ONLY
    // Processor name (ARM)
    char *processor;
    // Micro architecture (ARM, RISC-V)
    char *uarch;
    // Platform name (POWER)
    char *platform;
    // "detected as" result (POWER)
    char *detectedAs;
#endif
    // Vendor name (ARM, POWER, x86)
    char *vendor;
    // Model or architecture name (all)
    char *name;
    // Family number (x86)
    int family;
    // Model number (x86)
    int model;
    // Stepping number (x86)
    int stepping;
#ifndef X86_ONLY
    // Revision number (ARM)
    int revision;
#endif
    // Clock frequency in MHz (m68k, POWER, some RISC-V, x86)
    float freq;
    // Processor index count (some ARM, POWER, some RISC-V, x86)
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

// Maps a canonical/base CPU vendor name to an alias
typedef struct {
    const char *canonical;
    const char *alias;
} VENDOR_ALIAS;



#ifndef X86_ONLY

// Hardcoded ARM CPU implementer values to allow basic ARM CPU vendor
// identification
static const char *ARM_IMPLEMENTERS[193] = {
    [0x00] = "Reserved",
    [0x41] = "Arm",
    [0x42] = "Broadcom",
    [0x43] = "Cavium",
    [0x44] = "DEC",
    [0x46] = "Fujitsu",
    [0x48] = "HiSilicon",
    [0x49] = "Infineon",
    [0x4D] = "Motorola/Freescale",
    [0x4E] = "NVIDIA",
    [0x50] = "AMCC/Ampere",
    [0x51] = "Qualcomm",
    [0x56] = "Marvell",
    [0x61] = "Apple",
    [0x66] = "Faraday",
    [0x69] = "Intel",
    [0x6D] = "Microsoft",
    [0x70] = "Phytium",
    [0xC0] = "Ampere"
};

#endif

#ifndef NO_STR_CLEANING

// Hardcoded search needles to use when looking for a GPU name inside a CPU
// name
static const char *GPU_FROM_CPU_NEEDLES[] = {
    "with Radeon",
    "w/ Radeon",
    " RADEON",
};
static const int GPU_FROM_CPU_NEEDLES_LEN = sizeof(GPU_FROM_CPU_NEEDLES) / sizeof(GPU_FROM_CPU_NEEDLES[0]);

#endif

// Known CPU vendor canonical-alias name mappings
static const VENDOR_ALIAS VENDOR_ALIASES[] = {
    { "HiSilicon",  "HUAWEI" }
};
static const int VENDOR_ALIASES_LEN = sizeof(VENDOR_ALIASES) / sizeof(VENDOR_ALIASES[0]);



char *cleanCPUName(const CPU_ARCH, const char*, size_t);
CPU_DATA *getCPU(char*, char**);
int hasFlag(const CPU_DATA*, const char*);
char *interpretCPU(CPU_DATA*);

#endif
