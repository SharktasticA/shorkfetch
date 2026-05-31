/*
    ######################################################
    ##            SHORK UTILITY - SHORKFETCH            ##
    ######################################################
    ## Functions and data relating to handle system     ##
    ## packages                                         ##
    ######################################################
    ## Licence: GNU GENERAL PUBLIC LICENSE Version 3    ##
    ######################################################
    ## Kali (links.sharktastica.co.uk)                  ##
    ######################################################
*/



#include "general.h"
#include "globals.h"
#include "packages.h"

#include <dirent.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>



/**
 * @return String containing counts of various packages including dpkg, pacman,
 * rpm, flatpak and snap.
 */
char *getPackages(const char *os)
{
    // We know for sure SHORK doesn't have a package manager...
    if (os && strncmp(os, "SHORK", 5) == 0)
        return NULL;

    const int PKGS_SIZE = 256;
    char *pkgs = malloc(PKGS_SIZE);
    if (!pkgs) return NULL;
    pkgs[0] = '\0';

    int dCount = 0;
    int pCount = 0;
    int rCount = 0;
    int fCount = 0;
    int sCount = 0;

    // Get Debian-style packages by counting inside /var/lib/dpkg/status
    FILE *dpkgStatus = fopen("/var/lib/dpkg/status", "r");
    if (dpkgStatus)
    {
        const char *needle = "Status: install ok installed";
        size_t needleLen = strlen(needle);
        char buffer[512];
        while (fgets(buffer, 512, dpkgStatus))
            if (strncmp(buffer, needle, needleLen) == 0)
                dCount++;
        fclose(dpkgStatus);
    }

    // Get Arch-style packages by counting inside /var/lib/pacman/local
    DIR *pacmanLocal = opendir("/var/lib/pacman/local");
    if (pacmanLocal)
    {
        struct dirent *dirEntry;
        while ((dirEntry = readdir(pacmanLocal)) != NULL)
            if (dirEntry->d_name[0] != '.' && strcmp(dirEntry->d_name, "ALPM_DB_VERSION") != 0)
                pCount++;
        closedir(pacmanLocal);
    }

    // Get Fedora-style packages
    if (isProgramInstalled("rpm", 0))
    {
        // Try rpm for now (it's slow, we should find a better way...)
        FILE *fStream = popen("rpm -qa 2>/dev/null | wc -l", "r");
        if (fStream)
        {
            fscanf(fStream, "%d", &rCount);
            pclose(fStream);
        }
    }

    // Get Flatpak packages
    if (isProgramInstalled("flatpak", 0))
    {
        // Try quickly figuring the number out using the filesystem
        char userApp[PATH_MAX], userRuntime[PATH_MAX];
        snprintf(userApp, PATH_MAX, "%s/.local/share/flatpak/app", HOME);
        snprintf(userRuntime, PATH_MAX, "%s/.local/share/flatpak/runtime", HOME);

        // The directories we need to check - system and user apps and runtimes
        const char *flatpakDirs[] = {
            "/var/lib/flatpak/app",
            "/var/lib/flatpak/runtime",
            userApp,
            userRuntime
        };

        // We are looking for "active" symbolic link files. The tree looks like:
        // flatpakDir[i]/org.kde.Platform/x86_64/6.9   /active
        //              /name            /arch  /branch/BINGO
        for (int i = 0; i < 4; i++)
        {
            DIR *flatpakDir = opendir(flatpakDirs[i]);
            if (!flatpakDir) continue;
            size_t currFlatpakDirLen = strlen(flatpakDirs[i]);

            // Enter arch
            struct dirent *nameEntry;
            while ((nameEntry = readdir(flatpakDir)) != NULL)
            {
                if (nameEntry->d_name[0] == '.') continue;

                char archPath[PATH_MAX];
                int archPathLen = snprintf(archPath, PATH_MAX, "%s/%s", flatpakDirs[i], nameEntry->d_name);
                if (archPathLen < 0 || archPathLen >= PATH_MAX - currFlatpakDirLen)
                    continue;
                DIR *archDir = opendir(archPath);
                if (!archDir) continue;

                // Enter branch
                struct dirent *archEntry;
                while ((archEntry = readdir(archDir)) != NULL)
                {
                    if (archEntry->d_name[0] == '.') continue;

                    char branchPath[PATH_MAX];
                    int branchPathLen = snprintf(branchPath, PATH_MAX, "%s/%s", archPath, archEntry->d_name);
                    if (branchPathLen < 0 || branchPathLen >= PATH_MAX - archPathLen)
                        continue;
                    DIR *branchDir = opendir(branchPath);
                    if (!branchDir) continue;

                    // Look for out crucial "active" file
                    struct dirent *branchEntry;
                    while ((branchEntry = readdir(branchDir)) != NULL)
                    {
                        if (branchEntry->d_name[0] == '.') continue;

                        char activePath[PATH_MAX];
                        int activePathLen = snprintf(activePath, PATH_MAX, "%s/%s/active", branchPath, branchEntry->d_name);
                        if (activePathLen < 0 || activePathLen >= PATH_MAX - branchPathLen)
                            continue;
                        if (access(activePath, F_OK) != 0)
                            continue;

                        // flatpak list seems to skip .Locale, so we do so to
                        // match its output
                        size_t nameLen = strlen(nameEntry->d_name);
                        if (nameLen > 7 && strcmp(nameEntry->d_name + nameLen - 7, ".Locale") == 0)
                            continue;

                        fCount++;
                    }
                    closedir(branchDir);
                }
                closedir(archDir);
            }
            closedir(flatpakDir);
        }
    }

    // Get Snap packages by counting inside /snap or /var/lib/snapd/snap
    const char *snapDirs[] = {"/snap", "/var/lib/snapd/snap"};
    for (int i = 0; i < 2; i++)
    {
        DIR *snapDir = opendir(snapDirs[i]);
        if (!snapDir) continue;

        struct dirent *dirEntry;
        while ((dirEntry = readdir(snapDir)) != NULL)
            if (dirEntry->d_type == DT_DIR && dirEntry->d_name[0] != '.' && strcmp(dirEntry->d_name, "bin") != 0)
                sCount++;
        closedir(snapDir);

        if (sCount > 0) break;
    }

    // Build the result string
    if (dCount > 0)
        snprintf(pkgs, PKGS_SIZE, COMPACT ? "%d(D)" : "%d (dpkg)", dCount);
    if (pCount > 0)
        snprintf(pkgs + strlen(pkgs), PKGS_SIZE - strlen(pkgs), COMPACT ? ":%d(P)" : ", %d (pacman)", pCount);
    if (rCount > 0)
        snprintf(pkgs + strlen(pkgs), PKGS_SIZE - strlen(pkgs), COMPACT ? ":%d(R)" : ", %d (rpm)", rCount);
    if (fCount > 0)
        snprintf(pkgs + strlen(pkgs), PKGS_SIZE - strlen(pkgs), COMPACT ? ":%d(F)" : ", %d (flat)", fCount);
    if (sCount > 0)
        snprintf(pkgs + strlen(pkgs), PKGS_SIZE - strlen(pkgs), COMPACT ? ":%d(S)" : ", %d (snap)", sCount);

    // Make sure we don't start with ", "...
    size_t pkgsLen = strlen(pkgs);
    if (pkgsLen > 2 && pkgs[0] == ',' && pkgs[1] == ' ')
        memmove(pkgs, pkgs + 2, pkgsLen - 1);

    return pkgs;
}
