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



#include "../shorkcommon/general.h"
#include "../shorkcommon/shorkmenu.h"

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
    char *ram = malloc(RAM_LEN);
    if (!ram)
        return NULL;
    ram[0] = '\0';

    long used = mi.memTotal - mi.memAvailable;
    char *usedStr = bytesToReadable("KiB", used, CONFIG.maxUnit);
    char *totalStr = bytesToReadable("KiB", mi.memTotal, CONFIG.maxUnit);

    if (!COMPACT)
    {
        int pct = mi.memTotal ? (int)((used * 100) / mi.memTotal) : 0;
        char *col = colPctLow;
        if (pct >= 80)
            col = colPctHigh;
        else if (pct >= 50)
            col = colPctMed;
        snprintf(ram, RAM_LEN, "%s / %s (%s%d%%%s)", usedStr, totalStr, col,
            pct, colReset);
    }
    else
        snprintf(ram, RAM_LEN, "%s / %s", usedStr, totalStr);
    
    free(usedStr);
    free(totalStr);

    return ram;
}

#ifndef SHORK_DISKETTE

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

    char *swap = malloc(RAM_LEN);
    if (!swap)
        return strdup("");
    swap[0] = '\0';

    long used = mi.swapTotal - mi.swapFree;
    char *usedStr = bytesToReadable("KiB", used, CONFIG.maxUnit);
    char *totalStr = bytesToReadable("KiB", mi.swapTotal, CONFIG.maxUnit);

    if (!COMPACT)
    {
        int pct = mi.swapTotal ? (int)((used * 100) / mi.swapTotal) : 0;
        char *col = colPctLow;
        if (pct >= 80)
            col = colPctHigh;
        else if (pct >= 50)
            col = colPctMed;
        snprintf(swap, RAM_LEN, "%s / %s (%s%d%%%s)", usedStr, totalStr,
            col, pct, colReset);
    }
    else snprintf(swap, RAM_LEN, "%s / %s", usedStr, totalStr);

    free(usedStr);
    free(totalStr);

    return swap;
}

#else

char *getSwap(MemInfo mi, char *colPctLow, char *colPctMed,
    char *colPctHigh, const char *colReset)
{
    (void)mi; (void)colPctLow; (void)colPctMed; (void)colPctHigh;
    (void)colReset; return NULL;
}

#endif
