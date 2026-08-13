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

#include <fcntl.h>
#include <linux/fs.h>
#include <linux/limits.h>
#include <sys/statvfs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>



DISKS *getDisks(void)
{
    // Get possible block devices 
    DIR *blockDir = opendir("/sys/block");
    if (!blockDir)
        return NULL;

    DISKS *disks = malloc(sizeof(DISKS));
    if (!disks)
        return NULL;
    disks->count = 0;

    struct dirent *dirEntry;
    while ((dirEntry = readdir(blockDir)) != NULL)
    {
        if (disks->count >= DISKS_LEN)
            break;

        if (dirEntry->d_name[0] == '.')
            continue;

        if (strstr(dirEntry->d_name, "dm-") != 0 ||
            strstr(dirEntry->d_name, "sr") != 0)
            continue;

        char sizePath[PATH_MAX];
        snprintf(sizePath, PATH_MAX, "/sys/block/%s/size", dirEntry->d_name);

        // Get size
        FILE *file = fopen(sizePath, "r");
        if (!file)
            continue;
        unsigned long long sectors = 0;
        int scanned = fscanf(file, "%llu", &sectors);
        fclose(file);
        if (scanned != 1 || sectors == 0)
            continue;
        unsigned long long size = sectors * 512ULL;

        // Convert size to str with appropriate unit
        char *sizeStr = bytesToReadable("B", size);
        if (!sizeStr || sizeStr[0] == '\0')
        {
            free(sizeStr);
            continue;
        }

        // Add to disks
        snprintf(disks->disks[disks->count], DISK_LEN, "%s (%s)", sizeStr, dirEntry->d_name);
        free(sizeStr);
        disks->count++;
    }
    closedir(blockDir);

    return disks;
}

/**
 * @return String containing the root partition's used and total size amounts both numerically and as a percentage
 */
char *getRoot(void)
{
    char *root = malloc(ROOT_LEN);
    if (!root)
        return strdup("");
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
        snprintf(root, ROOT_LEN, "%s / %s (%d%%)", usedStr, totalStr, pct);
    }
    else snprintf(root, ROOT_LEN, "%s / %s", usedStr, totalStr);

    free(usedStr);
    free(totalStr);

    return root;
}
