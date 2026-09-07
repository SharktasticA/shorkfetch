/*
    ######################################################
    ##            SHORK UTILITY - SHORKFETCH            ##
    ######################################################
    ## Functions and data relating to handling system   ##
    ## main memory and swap memory                      ##
    ######################################################
    ## Licence: GNU GENERAL PUBLIC LICENSE Version 3    ##
    ######################################################
    ## Kali (links.sharktastica.co.uk)                  ##
    ######################################################
*/



#include "colours.h"
#include "general.h"
#include "globals.h"
#include "memory.h"

#include <stdlib.h>
#include <string.h>



/**
 * Reads memory and swap data from /proc/meminfo into a MemInfo struct.
 * @return populated MemInfo struct
 */
MemInfo getMemInfo(void)
{
    MemInfo mi = {0};

    FILE *fStream = fopen("/proc/meminfo", "r");
    if (fStream)
    {
        char buffer[128];
        int parsed = 0;
        while (fgets(buffer, sizeof(buffer), fStream) && parsed < 7)
        {
            if (sscanf(buffer, "MemTotal: %ld", &mi.memTotal) == 1)
                { parsed++; continue; }
            else if (sscanf(buffer, "MemFree: %ld", &mi.memFree) == 1)
                { parsed++; continue; }
            else if (sscanf(buffer, "MemAvailable: %ld",
                &mi.memAvailable) == 1)
                { parsed++; continue; }
            else if (sscanf(buffer, "Buffers: %ld", &mi.buffers) == 1)
                { parsed++; continue; }
            else if (sscanf(buffer, "Cached: %ld", &mi.cached) == 1)
                { parsed++; continue; }
            else if (sscanf(buffer, "SwapTotal: %ld", &mi.swapTotal) == 1)
                { parsed++; continue; }
            else if (sscanf(buffer, "SwapFree: %ld", &mi.swapFree) == 1)
                { parsed++; continue; }
        }
        fclose(fStream);
    }

    return mi;
}

/**
 * @param mi /proc/meminfo data
 * @return String containing the system memory used and total amounts both
 *         numerically and as a percentage
 */
char *getRAM(MemInfo mi, char *colPctLow, char *colPctMed, char *colPctHigh,
    const char *colReset)
{
    const int ramSize = 64;
    char *ram = malloc(ramSize);
    if (!ram)
        return NULL;
    ram[0] = '\0';

    long used = mi.memTotal - mi.memAvailable;
    char *usedStr = bytesToReadable("KiB", used, MAX_UNIT);
    char *totalStr = bytesToReadable("KiB", mi.memTotal, MAX_UNIT);

    if (!COMPACT)
    {
        int pct = mi.memTotal ? (int)((used * 100) / mi.memTotal) : 0;
        char *col = colPctLow;
        if (pct >= 80)
            col = colPctHigh;
        else if (pct >= 50)
            col = colPctMed;
        snprintf(ram, ramSize, "%s / %s (%s%d%%%s)", usedStr, totalStr, col,
            pct, colReset);
    }
    else
        snprintf(ram, ramSize, "%s / %s", usedStr, totalStr);
    
    free(usedStr);
    free(totalStr);

    return ram;
}

/**
 * @param mi /proc/meminfo data
 * @return String containing the system swap used and total amounts both
 *         numerically and as a percentage
 */
char *getSwap(MemInfo mi, char *colPctLow, char *colPctMed,
    char *colPctHigh, const char *colReset)
{
    if (mi.swapTotal == 0)
        return strdup("");

    const int swapSize = 64;
    char *swap = malloc(swapSize);
    if (!swap)
        return strdup("");
    swap[0] = '\0';

    long used = mi.swapTotal - mi.swapFree;
    char *usedStr = bytesToReadable("KiB", used, MAX_UNIT);
    char *totalStr = bytesToReadable("KiB", mi.swapTotal, MAX_UNIT);

    if (!COMPACT)
    {
        int pct = mi.swapTotal ? (int)((used * 100) / mi.swapTotal) : 0;
        char *col = colPctLow;
        if (pct >= 80)
            col = colPctHigh;
        else if (pct >= 50)
            col = colPctMed;
        snprintf(swap, swapSize, "%s / %s (%s%d%%%s)", usedStr, totalStr,
            col, pct, colReset);
    }
    else snprintf(swap, swapSize, "%s / %s", usedStr, totalStr);

    free(usedStr);
    free(totalStr);

    return swap;
}
