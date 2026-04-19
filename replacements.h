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
};



static const struct Replacement amdReplacements[] = {
    { " 486 DX", " Am486DX" },
    { "Am486DX/2", "Am486DX" },
    { "Am486DX/4", "Am486DX4/Am5x86" }
};
static const int amdReplacementsLen = sizeof(amdReplacements) / sizeof(amdReplacements[0]);



static const struct Replacement intelReplacements[] = {
    { " 486 SX", " i486SX" },
    { "486SX/2", "486SX2" },
    { " 486 DX/4", "DX4" },
    { " 486 DX", " i486DX" },
    { "486DX-50", "486DX" },
    { "486DX/2", "486DX2" },
    { "OverDrive PODP5V63", "Pentium OverDrive" },
    { "OverDrive PODP5V83", "Pentium OverDrive" },
    { "Pentium 60/66", "Pentium (P5)" },
    { "Pentium 75 - 200", "Pentium (P54C)" },
    { "Pentium 4 - M", "Pentium 4-M" },
    { "Core2", "Core 2" },
    { "Generation Core", "Gen Core" }
};
static const int intelReplacementsLen = sizeof(intelReplacements) / sizeof(intelReplacements[0]);



static const struct Replacement idtReplacements[] = {
    { " 05/04", " WinChip" },
    { "WinChip 2-3D", "WinChip 2/3" }
};
static const int idtReplacementsLen = sizeof(idtReplacements) / sizeof(idtReplacements[0]);

#endif
