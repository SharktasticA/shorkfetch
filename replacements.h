/*
    ######################################################
    ##            SHORK UTILITY - SHORKFETCH            ##
    ######################################################
    ## CPU and GPU name substring replacements used to  ##
    ## help clean up messy data from /proc/cpuinfo or   ##
    ## pci.ids.                                         ##
    ######################################################
    ## Licence: GNU GENERAL PUBLIC LICENSE Version 3    ##
    ######################################################
    ## Kali (links.sharktastica.co.uk)                  ##
    ######################################################
*/



#ifndef CPUS
#define CPUS

struct Replacement {
    const char *match;
    const char *replacement;
    const int standalone;
};



static const struct Replacement AMD_REPLACES[] = {
    { " 486 DX",        " Am486DX",         0 },
    { "Am486DX/2",      "Am486DX",          0 },
    { "Am486DX/4",      "Am486DX4/Am5x86",  0 },
    { "K6 3D+",         "K6-III",           0 },
    { "K6 3D",          "K6-2",             0 },
    { "K6-III",         "K6-2+/K6-III+",    1 }
};
static const int AMD_REPLACES_LEN = sizeof(AMD_REPLACES) / sizeof(AMD_REPLACES[0]);



const char *DELETIONS[] =
{
    "AMD-",                             // For AMD K6
    ", Inc.",
    ", Inc",
    "(R)",
    "(tm)",
    "(tm )",                            // For AMD Duron
    "tm",
    "(TM)",
    " APU",
    " Dual Core",                       // For AMD Athlon
    " Controller",
    " Corporation",
    " CPU",
    " Eight-Core",                      // For AMD FX
    " Electronics Systems",             // For Matrox
    " Family",
    " Interactive",                     // For 3dfx
    " Ltd.",
    " Microsystems",                    // For Trident
    " processor",
    " Processor",
    " Quad Core",                       // For AMD Athlon
    " Quad-Core",                       // For AMD FX
    " Six-Core",                        // For AMD FX
    " Technologies",                    // For VIA
    " Technology LLC",                  // For Loongson
    " w/ multimedia extensions",        // For AMD K6
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
    { "OverDrive PODP5V63",     "Pentium OverDrive",    0 },
    { "OverDrive PODP5V83",     "Pentium OverDrive",    0 },
    { "Pentium 60/66",          "Pentium (P5)",         0 },
    { "Pentium 75 - 200",       "Pentium (P54C)",       0 },
    { "Pentium 4 - M",          "Pentium 4-M",          0 },
    { "Core2",                  "Core 2",               0 },
    { "Generation Core",        "Gen Core",             0 }
};
static const int INTEL_REPLACES_LEN = sizeof(INTEL_REPLACES) / sizeof(INTEL_REPLACES[0]);



static const struct Replacement IDT_REPLACES[] = {
    { " 05/04",         " WinChip",     0 },
    { "WinChip 2-3D",   "WinChip 2/3",  0 }
};
static const int IDT_REPLACES_LEN = sizeof(IDT_REPLACES) / sizeof(IDT_REPLACES[0]);

#endif
