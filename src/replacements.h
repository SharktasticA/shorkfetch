/*
    ######################################################
    ##            SHORK UTILITY - SHORKFETCH            ##
    ######################################################
    ## CPU and GPU name substring replacements used to  ##
    ## help clean up messy data from /proc/cpuinfo or   ##
    ## pci.ids in cleanCPUName and cleanGPUName.        ##
    ## Regarding CPUs, cpu.c also does CPU name         ##
    ## cleaning, but only at the vendor-name level or   ##
    ## if targetting specific family, model and/or      ##
    ## stepping numbers. Otherwise, it goes in here.    ##
    ######################################################
    ## Licence: GNU GENERAL PUBLIC LICENSE Version 3    ##
    ######################################################
    ## Kali (links.sharktastica.co.uk)                  ##
    ######################################################
*/



#ifndef REPLACEMENTS
#define REPLACEMENTS

struct Replacement {
    const char *match;
    const char *replacement;
    const int standalone;
};



static const struct Replacement AMD_REPLACES[] = {
    { " 486 DX",                " Am486DX",         0 },
    { "Am486DX/2",              "Am486DX2",         0 },
    { "Am486DX/4",              "Am486DX4",         0 },
    { "Am5x86-WT",              "Am5x86",           0 },
    { "K6 3D+",                 "K6-III",           0 },
    { "K6 3D",                  "K6-2",             0 },
    { "K6-III ",                "K6-2+/K6-III+",    1 },
    { "AMD-",                   "AMD ",             0 },
    { "Dual Core ",             "",                 0 },    // See Athlon 64 X2 4800+
    { "Dual-Core ",             "",                 0 },    // See Opteron 1216
    { "Quad Core ",             "",                 0 },    // See Athlon X4 760K
    { "Quad-Core ",             "",                 0 },    // See FX-4300, Opteron 2350 HE
    { "Six-Core ",              "",                 0 },    // See FX-6300
    { "Eight-Core ",            "",                 0 },    // See FX-8320
    { "RYZEN AI MAX",           "Ryzen AI Max",     0 }     // See Ryzen AI Max+ 395
};
static const int AMD_REPLACES_LEN = sizeof(AMD_REPLACES) / sizeof(AMD_REPLACES[0]);



static const struct Replacement COMPACT_CPU_REPLACES[] = {
    { "Core Solo",              "CS",           0 },
    { "Core Duo",               "CD",           0 },
    { "Core 2 Duo",             "C2D",          0 },
    { "Core 2 Extreme",         "C2E",          0 },
    { "Core 2 Quad",            "C2Q",          0 },
    { "Core i",                 "i",            0 },
    { "Core m",                 "m",            0 },
    { "Core Ultra ",            "CU ",          0 },
    { " for ",                  " ",            0 },
    { "Mobile ",                "",             0 },
    { "OverDrive",              "OD",           0 },
    { "Ryzen 3",                "R3",           0 },
    { "Ryzen 5",                "R5",           0 },
    { "Ryzen 7",                "R7",           0 },
    { "Ryzen 9",                "R9",           0 }
};
static const int COMPACT_CPU_REPLACES_LEN = sizeof(COMPACT_CPU_REPLACES) / sizeof(COMPACT_CPU_REPLACES[0]);



static const struct Replacement COMPACT_GPU_REPLACES[] = {
    { " Adapter",               "",             0 },
    { "Express ",               "",             0 },
    { "Extreme Graphics",       "ET",           0 },
    { " for ",                  " ",            0 },
    { " GeForce2",              " 2",           0 },
    { " GeForce3",              " 3",           0 },
    { " GeForce4",              " 4",           0 },
    { " GeForce",               "",             0 },
    { "Graphics Technology",    "GT",           0 },
    { " Graphics",              "",             0 },
    { " Integrated",            "",             0 },
    { "Quadro ",                "",             0 },
    { "Max-Q",                  "M-Q",          0 },
    { "Mobile ",                "",             0 },
    { " Radeon",                "",             0 },
    { "Vega/Vega",              "Vega",         0 }
};
static const int COMPACT_GPU_REPLACES_LEN = sizeof(COMPACT_GPU_REPLACES) / sizeof(COMPACT_GPU_REPLACES[0]);



static const struct Replacement COMPACT_OS_REPLACES[] = {
    { " GNU/Linux", "",     0 },
    { " INDEV",     "",     0 },
    { " Linux",     "",     0 }
};
static const int COMPACT_OS_REPLACES_LEN = sizeof(COMPACT_OS_REPLACES) / sizeof(COMPACT_OS_REPLACES[0]);



static const char *DELETIONS[] =
{
    ", Inc.",
    ", Inc",
    " Inc",
    "(R)",
    "(tm)",
    "(tm )",                            // For AMD Duron
    "tm",
    "(TM)",
    " APU",
    " Controller",
    " Corporation",
    " CPU",
    " Family",
    "Genuine ",
    " Graphics Adapter",
    " Ltd.",
    " processor",
    " Processor",
    " Technologies",                    // For VIA
    " Technology LLC",                  // For Loongson
    " 2x Core/Bus Clock",               // For Cyrix 6x86
    " 3x Core/Bus Clock"                // For Cyrix 5x86
};
static const int DELETIONS_LEN = sizeof(DELETIONS) / sizeof(DELETIONS[0]);



static const struct Replacement INTEL_REPLACES[] = {
    { " 486 SX",                " i486SX",              0 },
    { "486SX/2",                "486SX2",               0 },
    { " 486 DX/4",              "DX4",                  0 },
    { " 486 DX",                " i486DX",              0 },
    { "486DX-50",               "486DX",                0 },
    { "486DX/2",                "486DX2",               0 },
    { "Pentium 60/66",          "Pentium (P5)",         0 },
    { "Pentium 75 - 200",       "Pentium (P54C)",       0 },
    { "Pentium 4 - M",          "Pentium 4-M",          0 },
    { "Core2",                  "Core 2",               0 },
    { "Generation Core",        "Gen Core",             0 },
    { " Dual-Core",             "",                     0 }     // See Pentium E5300
};
static const int INTEL_REPLACES_LEN = sizeof(INTEL_REPLACES) / sizeof(INTEL_REPLACES[0]);



static const struct Replacement IDT_REPLACES[] = {
    { " 05/04",         " WinChip",     0 },
    { "WinChip 2-3D",   "WinChip 2/3",  0 }
};
static const int IDT_REPLACES_LEN = sizeof(IDT_REPLACES) / sizeof(IDT_REPLACES[0]);



static const struct Replacement VIA_REPLACES[] = {
    { "Samuel", "Cyrix III",    0 }
};
static const int VIA_REPLACES_LEN = sizeof(VIA_REPLACES) / sizeof(VIA_REPLACES[0]);

#endif
