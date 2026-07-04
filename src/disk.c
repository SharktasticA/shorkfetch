/*
    ######################################################
    ##            SHORK UTILITY - SHORKFETCH            ##
    ######################################################
    ## Functions and data relating to handling disk and ##
    ## partition sizes                                  ##
    ######################################################
    ## Licence: GNU GENERAL PUBLIC LICENSE Version 3    ##
    ######################################################
    ## Kali (links.sharktastica.co.uk)                  ##
    ######################################################
*/



#include "general.h"
#include "globals.h"
#include "disk.h"

#include <sys/statvfs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



/**
 * @return String containing the root partition's used and total size amounts both numerically and as a percentage
 */
char *getRoot(void)
{
    const int rootSize = 64;
    char *root = malloc(rootSize);
    if (!root) return strdup("");
    root[0] = '\0';

    struct statvfs fs;
    if (statvfs("/", &fs) != 0)
        return root;

    long long total = (long long)fs.f_blocks * fs.f_frsize;
    // If total = 0, return the blank result string to flag that we have
    // nothing to show
    if (total == 0)
        return root;

    long long freeRoot  = (long long)fs.f_bfree * fs.f_frsize;
    long long used  = total - freeRoot;

    char *usedStr = bytesToReadable("B", used);
    char *totalStr = bytesToReadable("B", total);

    if (!COMPACT)
    {
        int pct = total ? (int)((used * 100) / total) : 0;
        snprintf(root, rootSize, "%s / %s (%d%%)", usedStr, totalStr, pct);
    }
    else snprintf(root, rootSize, "%s / %s", usedStr, totalStr);

    free(usedStr);
    free(totalStr);

    return root;
}
