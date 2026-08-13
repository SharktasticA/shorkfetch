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
        if (dirEntry->d_name[0] == '.')
            continue;

        char blockDev[PATH_MAX];
        snprintf(blockDev, PATH_MAX, "/dev/%s", dirEntry->d_name);

        // Get size
        int fd = open(blockDev, O_RDONLY);
        if (fd == -1)
            continue;
        unsigned long long size = 0;
        int ioctlRet = ioctl(fd, BLKGETSIZE64, &size);
        close(fd);
        if (ioctlRet == -1 || size == 0)
            continue;

        // Convert size to str with appropriate unit
        char *sizeStr = bytesToReadable("B", size);
        if (!sizeStr || sizeStr[0] == '\0')
        {
            free(sizeStr);
            continue;
        }

        // Add to disks
        snprintf(disks->disks[disks->count], DISK_LEN, "%s (%s)", sizeStr, dirEntry->d_name);
        disks->count++;
    }

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
