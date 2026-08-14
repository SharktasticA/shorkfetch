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



/**
 * Gets a list of valid block device names and their total size.
 * @return DISKS pointer countaining the list and entry count
 */
DISKS *getDisks(void)
{
    // Get possible block devices 
    DIR *blockDir = opendir("/sys/block");
    if (!blockDir)
        return NULL;

    DISKS *result = malloc(sizeof(DISKS));
    if (!result)
        return NULL;
    result->count = 0;

    // Read possible block devices beforehand
    int noBlockDevs = 0;
    char blockDevs[DISKS_LEN][PATH_MAX];
    struct dirent *dirEntry;
    while ((dirEntry = readdir(blockDir)) != NULL)
    {
        if (noBlockDevs == DISKS_LEN)
            break;

        if (dirEntry->d_name[0] == '.')
            continue;

        if (strlen(dirEntry->d_name) < 3 || (strncmp(dirEntry->d_name, "hd", 2) != 0 && strncmp(dirEntry->d_name, "sd", 2) != 0 && strncmp(dirEntry->d_name, "nvm", 3) != 0))
            continue;

        snprintf(blockDevs[noBlockDevs++], PATH_MAX, "%s", dirEntry->d_name);
    }
    closedir(blockDir);

    if (!noBlockDevs)
        return result;

    // Naturally sort the block device names before processing
    qsort(blockDevs, noBlockDevs, sizeof(blockDevs[0]), natCmp);

    // Get and verify the size of each block device, saving into our result
    for (int i = 0; i < noBlockDevs; i++)
    {
        char sizePath[PATH_MAX];
        snprintf(sizePath, PATH_MAX, "/sys/block/%s/size", blockDevs[i]);

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
        snprintf(result->disks[result->count], DISK_LEN, "%s (%s)", sizeStr, blockDevs[i]);
        free(sizeStr);
        result->count++;
    }

    return result;
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
