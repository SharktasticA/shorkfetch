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
#define IGNORE_UNIQUE_PHYS_IDS  -1
// CPU_DATA.flags string length
#define FLAGS_LEN               1536
// CPU_DATA.machine string length
#define MACHINE_LEN             128
// Highest processor index no. to try processing
#define MAX_INDEX               1024
// CPU_DATA.name string length
#define NAME_LEN                128
// CPU_DATA.platform string length
#define PLATFORM_LEN            128
// CPU_DATA.processor string length
#define PROCESSOR_LEN           128
// CPU_DATA.revisionStr string length
#define REVISION_STR_LEN        128
// CPU_DATA.uarch string length
#define UARCH_LEN               128
// PHYS_IDS.uniquePhysIDs array size
#define UNIQUE_PHYS_IDS_SIZE    128
// CPU_DATA.vendor string length
#define VENDOR_LEN              16



// The CPU architectures supported by SHORKFETCH
typedef enum
{
    CPU_ARCH_UNKNOWN,
#ifndef X86_ONLY
    ARM,
    M68K,
    MIPS,
    POWER,
    RISCV,
#endif
    X86
} CPU_ARCH;

#ifndef X86_ONLY

// PowerPC generation
typedef enum
{
    PPC_GEN_UNKNOWN = 0,
    // PowerPC 6xx
    PPC_6XX = 1,
    // PowerPC 7xx/G3
    PPC_7XX = 3,
    // PowerPC 74xx/G4
    PPC_74XX = 4,
    // PowerPC 9xx/G5
    PPC_9XX = 5
} PPC_GEN;

#endif



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
    // Machine name (POWER)
    char *machine;
#endif
    // Vendor name (ARM, POWER, x86)
    char *vendor;
    // Model or architecture name (all)
    char *name;
#ifndef X86_ONLY
    // Part name (ARM)
    char *part;
#endif
    // Family number (x86)
    int family;
    // Model number (x86)
    int model;
    // Stepping number (x86)
    int stepping;
#ifndef X86_ONLY
    // Revision number (ARM)
    int revisionNo;
    // Revision string (POWER)
    char *revisionStr;
#endif
    // Clock frequency in MHz (m68k, POWER, some RISC-V, x86)
    float freq;
    // Processor index count (some ARM, POWER, some RISC-V, x86)
    int index;
    // Physical IDs (x86)
    PHYS_IDS physIDs;
    // Physical core count (x86)
    int cores;
    // Physical low-power Efficient-core count (x86-IHA)
    int lpeCores;
    // Physical Efficient-core count (x86-IHA)
    int eCores;
    // Physical Performance-core count (x86-IHA)
    int pCores;
    // Physical floating-point unit count (x86-CMT)
    int fUnits;
    // Physical integer unit count (x86-CMT)
    int iUnits;
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

// Maps an unsigned ID number to some name
typedef struct {
    unsigned id;
    const char *name;
} ID_NAME;

// Maps a canonical/base CPU vendor name to an alias
typedef struct {
    const char *canonical;
    const char *alias;
} VENDOR_ALIAS;



#ifndef X86_ONLY

// Hardcoded Apple M-series ARM CPU part numbers to allow specific model
// identification
// Sources: * https://github.com/AsahiLinux/m1n1/blob/main/src/midr.h
//          * https://asahilinux.org/docs/hw/soc/soc-codenames/
static const ID_NAME APPLE_M_PARTS[] = {
    { 0x22, "M1" },             // T8103 Icestorm
    { 0x23, "M1" },             // T8103 Firestorm
    { 0x24, "M1 Pro" },         // T6000 Icestorm
    { 0x25, "M1 Pro" },         // T6000 Firestorm
    { 0x28, "M1 Max/Ultra" },   // T6001/T6002 Icestorm
    { 0x29, "M1 Max/Ultra" },   // T6001/T6002 Firestorm
    { 0x32, "M2" },             // T8112 Blizzard
    { 0x33, "M2" },             // T8112 Avalanche
    { 0x34, "M2 Pro" },         // T6020 Blizzard
    { 0x35, "M2 Pro" },         // T6020 Avalanche
    { 0x38, "M2 Max/Ultra" },   // T6021/T6022 Blizzard
    { 0x39, "M2 Max/Ultra" },   // T6021/T6022 Avalanche
    { 0x42, "M3" },             // T8122 Sawtooth
    { 0x43, "M3" },             // T8122 Everest
    { 0x44, "M3 Pro" },         // T6030 Sawtooth
    { 0x45, "M3 Pro" },         // T6030 Everest
    { 0x48, "M3 Max" },         // T6031/T6034 Sawtooth
    { 0x49, "M3 Max" },         // T6031/T6034 Everest
    { 0x52, "M4" },             // T8132 Donan E-core
    { 0x53, "M4" },             // T8132 Donan P-core
    { 0x54, "M4 Pro" },         // T6040 Brava Chop E-core
    { 0x55, "M4 Pro" },         // T6040 Brava Chop P-core
    { 0x58, "M4 Max" },         // T6041 Brava E-core
    { 0x59, "M4 Max" },         // T6041 Brava P-core
    { 0x62, "M5" },             // T8142 Hidra E-core
    { 0x63, "M5" },             // T8142 Hidra P-core
    { 0x64, "M5 Pro" },         // T6050 Sotra M-core
    { 0x65, "M5 Pro" },         // T6050 Sotra P-core
    { 0x68, "M5 Max" },         // T6051 Sotrac M-core
    { 0x69, "M5 Max" }          // T6051 Sotrac P-core
};
static const int APPLE_M_PARTS_LEN = sizeof(APPLE_M_PARTS) /
    sizeof(APPLE_M_PARTS[0]);

// Hardcoded ARM CPU implementer values to allow basic ARM CPU vendor
// identification
static const ID_NAME ARM_IMPLEMENTERS[] = {
    { 0x00, "Reserved" },
    { 0x41, "Arm" },
    { 0x42, "Broadcom" },
    { 0x43, "Cavium" },
    { 0x44, "DEC" },
    { 0x46, "Fujitsu" },
    { 0x48, "HiSilicon" },
    { 0x49, "Infineon" },
    { 0x4D, "Motorola/Freescale" },
    { 0x4E, "NVIDIA" },
    { 0x50, "AMCC/Ampere" },
    { 0x51, "Qualcomm" },
    { 0x56, "Marvell" },
    { 0x61, "Apple" },
    { 0x66, "Faraday" },
    { 0x69, "Intel" },
    { 0x6D, "Microsoft" },
    { 0x70, "Phytium" },
    { 0xC0, "Ampere" }
};
static const int ARM_IMPLEMENTERS_LEN = sizeof(ARM_IMPLEMENTERS) /
    sizeof(ARM_IMPLEMENTERS[0]);

#endif

#ifndef NO_STR_CLEANING

// Hardcoded search needles to use when looking for a GPU name inside a CPU
// name
static const char *GPU_FROM_CPU_NEEDLES[] = {
    "with AMD Radeon",
    "with Radeon",
    "w/ Radeon",
    " RADEON",
    " Radeon",
};
static const int GPU_FROM_CPU_NEEDLES_LEN = sizeof(GPU_FROM_CPU_NEEDLES) /
    sizeof(GPU_FROM_CPU_NEEDLES[0]);

#endif

// Known CPU vendor canonical-alias name mappings
static const VENDOR_ALIAS VENDOR_ALIASES[] = {
    { "Intel",          "GenuineIntel" },
    { "Intel",          "GenuineIotel" },
    { "AMD",            "AuthenticAMD" },
    { "Cyrix",          "CyrixInstead" },
    { "Centaur/IDT",    "CentaurHauls" },
    { "Centaur",        "CentaurHauls" },
    { "IDT",            "CentaurHauls" },
    { "VIA",            "CentaurHauls" },
    { "Transmeta",      "GenuineTMx86" },
    { "Transmeta",      "TransmetaCPU" },
#ifndef X86_ONLY
    { "HiSilicon",      "HUAWEI" }
#endif
};
static const int VENDOR_ALIASES_LEN = sizeof(VENDOR_ALIASES) /
    sizeof(VENDOR_ALIASES[0]);



char *cleanCPUName(const CPU_ARCH, const char*, int);
CPU_DATA *getCPU(char*, char**);
int getIntelHybridCPU(CPU_DATA*);
int hasFlag(const CPU_DATA*, const char*);
char *interpretCPU(CPU_DATA*, char**);

#endif
