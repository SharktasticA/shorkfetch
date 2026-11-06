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



#include "../shorkcommon/general.h"
#include "../shorkcommon/shorkmenu.h"

#include "globals.h"
#include "packages.h"

#include <dirent.h>
#include <dlfcn.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>



#ifndef SHORK_DISKETTE

/**
 * Gets a count of various Linux package standards including dpkg, emerge,
 * pacman, pkgtool, rpm, flat and snap.
 * @return String containing all of the found/applicable package counts
 */
char *getPackages(const char *os)
{
    // We know for sure SHORK doesn't have a package manager...
    if (os && strncmp(os, "SHORK", 5) == 0)
        return NULL;

    char *pkgs = malloc(PACKAGES_LEN);
    if (!pkgs)
        return NULL;
    pkgs[0] = '\0';

    int dpkgCount = 0;
    int emergeCount = 0;
    int pacmanCount = 0;
    int pkgtoolCount = 0;
    int rpmCount = 0;
    int flatCount = 0;
    int snapCount = 0;

    // Get Debian-style packages by counting inside /var/lib/dpkg/status
    FILE *dpkgStatus = fopen("/var/lib/dpkg/status", "r");
    if (dpkgStatus)
    {
        const char *needle = "Status: install ok installed";
        int needleLen = strlen(needle);
        char buffer[32];
        while (fgets(buffer, 32, dpkgStatus))
            if (strncmp(buffer, needle, needleLen) == 0)
                dpkgCount++;
        fclose(dpkgStatus);
    }

    // Get Gentoo packages by counting inside /var/db/pkg/<category>/
    DIR *dbPkg = opendir("/var/db/pkg");
    if (dbPkg)
    {
        struct dirent *catEntry;
        while ((catEntry = readdir(dbPkg)) != NULL)
        {
            if (catEntry->d_name[0] == '.')
                continue;

            char catPath[PATH_MAX];
            int catPathLen = snprintf(catPath, PATH_MAX,
                "/var/db/pkg/%s", catEntry->d_name);
            if (catPathLen < 0 || catPathLen >= PATH_MAX)
                continue;

            DIR *catDir = opendir(catPath);
            if (!catDir)
                continue;

            struct dirent *pkgEntry;
            while ((pkgEntry = readdir(catDir)) != NULL)
                if (pkgEntry->d_name[0] != '.' &&
                    pkgEntry->d_type == DT_DIR)
                    emergeCount++;
            closedir(catDir);
        }
        closedir(dbPkg);
    }

    // Get Arch-style packages by counting inside /var/lib/pacman/local
    DIR *pacmanLocal = opendir("/var/lib/pacman/local");
    if (pacmanLocal)
    {
        struct dirent *dirEntry;
        while ((dirEntry = readdir(pacmanLocal)) != NULL)
            if (dirEntry->d_name[0] != '.' &&
                strcmp(dirEntry->d_name, "ALPM_DB_VERSION") != 0)
                pacmanCount++;
        closedir(pacmanLocal);
    }

    // Get Slackware-style packages by counting inside
    // known .../packages directories
    for (int i = 0; i < PKGTOOL_DIRS_LEN; i++)
    {
        DIR *pkgtoolDir = opendir(PKGTOOL_DIRS[i]);
        if (!pkgtoolDir)
            continue;

        struct dirent *dirEntry;
        while ((dirEntry = readdir(pkgtoolDir)) != NULL)
            if (dirEntry->d_name[0] != '.' && dirEntry->d_type != DT_DIR)
                pkgtoolCount++;
        closedir(pkgtoolDir);

        if (pkgtoolCount > 0)
            break;
    }

    // Function pointers for dynamically-loaded SQLite functions
    int (*sOpen)(const char *, void **, int, const char *) = NULL;
    int (*sPrepare)(void *, const char *, int, void **,
        const char **) = NULL;
    int (*sStep)(void *) = NULL;
    int (*sColumnInt)(void *, int) = NULL;
    int (*sFinalize)(void *) = NULL;
    int (*sClose)(void *) = NULL;

    // Get Fedora-style packages by querying RPM databases with SQLite
    for (int i = 0; i < RPM_DBS_LEN && rpmCount == 0; i++)
    {
        if (access(RPM_DBS[i], R_OK) != 0)
            continue;

        // Attempt to dynamically load libsqlite3
        if (!sOpen)
        {
            void *lib = dlopen("libsqlite3.so.0", RTLD_LAZY);
            if (!lib)
                break;

            sOpen = (int (*)(const char *, void **, int, const char *))
                dlsym(lib, "sqlite3_open_v2");
            sPrepare = (int (*)(void *, const char *, int, void **,
                const char **))dlsym(lib, "sqlite3_prepare_v2");
            sStep = (int (*)(void *))dlsym(lib, "sqlite3_step");
            sColumnInt = (int (*)(void *, int))dlsym(lib,
                "sqlite3_column_int");
            sFinalize = (int (*)(void *))dlsym(lib, "sqlite3_finalize");
            sClose = (int (*)(void *))dlsym(lib, "sqlite3_close");

            if (!sOpen || !sPrepare || !sStep || !sColumnInt ||
                !sFinalize || !sClose)
                break;
        }

        char uri[PATH_MAX + 32];
        int uriLen = snprintf(uri, sizeof(uri), "file:%s?immutable=1",
            RPM_DBS[i]);
        if (uriLen < 0 || (size_t)uriLen >= sizeof(uri))
            continue;

        void *db = NULL;
        if (sOpen(uri, &db, SQL_OPEN_READONLY | SQL_OPEN_URI, NULL) !=
            SQL_OK)
        {
            sClose(db);
            continue;
        }

        void *stmt = NULL;
        if (sPrepare(db, "SELECT COUNT(*) FROM Packages;", -1, &stmt,
            NULL) == SQL_OK)
        {
            if (sStep(stmt) == SQL_ROW)
                rpmCount = sColumnInt(stmt, 0);
            sFinalize(stmt);
        }
        sClose(db);
    }

    // Get Fedora-style packages by capturing rpm's output (fallback)
    if (rpmCount == 0 && isProgramInstalled("rpm", 0))
    {
        FILE *fStream = popen("rpm -qa 2>/dev/null | wc -l", "r");
        if (fStream)
        {
            fscanf(fStream, "%d", &rpmCount);
            pclose(fStream);
        }
    }

    // Get Flatpak packages
    if (isProgramInstalled("flatpak", 0))
    {
        // Try quickly figuring the number out using the filesystem
        char userApp[PATH_MAX], userRuntime[PATH_MAX];
        snprintf(userApp, PATH_MAX, "%s/.local/share/flatpak/app",
            HOME);
        snprintf(userRuntime, PATH_MAX, "%s/.local/share/flatpak/runtime",
            HOME);

        // The directories we need to check - system and user apps and
        // runtimes
        const char *flatpakDirs[] = {
            "/var/lib/flatpak/app",
            "/var/lib/flatpak/runtime",
            userApp,
            userRuntime
        };

        // We are looking for "active" symbolic link files. The tree looks
        // like:
        // flatpakDir[i]/org.kde.Platform/x86_64/6.9   /active
        //              /name            /arch  /branch/BINGO
        for (int i = 0; i < 4; i++)
        {
            DIR *flatpakDir = opendir(flatpakDirs[i]);
            if (!flatpakDir)
                continue;
            int currFlatpakDirLen = strlen(flatpakDirs[i]);

            // Enter arch
            struct dirent *nameEntry;
            while ((nameEntry = readdir(flatpakDir)) != NULL)
            {
                if (nameEntry->d_name[0] == '.')
                    continue;

                char archPath[PATH_MAX];
                int archPathLen = snprintf(archPath, PATH_MAX, "%s/%s",
                    flatpakDirs[i], nameEntry->d_name);
                if (archPathLen < 0 ||
                    archPathLen >= PATH_MAX - currFlatpakDirLen)
                    continue;
                DIR *archDir = opendir(archPath);
                if (!archDir)
                    continue;

                // Enter branch
                struct dirent *archEntry;
                while ((archEntry = readdir(archDir)) != NULL)
                {
                    if (archEntry->d_name[0] == '.')
                        continue;

                    char branchPath[PATH_MAX];
                    int branchPathLen = snprintf(branchPath, PATH_MAX,
                        "%s/%s", archPath, archEntry->d_name);
                    if (branchPathLen < 0 ||
                        branchPathLen >= PATH_MAX - archPathLen)
                        continue;
                    DIR *branchDir = opendir(branchPath);
                    if (!branchDir)
                        continue;

                    // Look for out crucial "active" file
                    struct dirent *branchEntry;
                    while ((branchEntry = readdir(branchDir)) != NULL)
                    {
                        if (branchEntry->d_name[0] == '.')
                            continue;

                        char activePath[PATH_MAX];
                        int activePathLen = snprintf(activePath, PATH_MAX,
                            "%s/%s/active", branchPath,
                            branchEntry->d_name);
                        if (activePathLen < 0 ||
                            activePathLen >= PATH_MAX - branchPathLen)
                            continue;
                        if (access(activePath, F_OK) != 0)
                            continue;

                        // flatpak list seems to skip .Locale, so we do so
                        // to match its output
                        int nameLen = strlen(nameEntry->d_name);
                        if (nameLen > 7 &&
                            strcmp(nameEntry->d_name + nameLen - 7,
                                ".Locale") == 0)
                            continue;

                        flatCount++;
                    }
                    closedir(branchDir);
                }
                closedir(archDir);
            }
            closedir(flatpakDir);
        }
    }

    // Get Snap packages by counting inside snap directories
    for (int i = 0; i < SNAP_DIRS_LEN; i++)
    {
        DIR *snapDir = opendir(SNAP_DIRS[i]);
        if (!snapDir)
            continue;

        struct dirent *dirEntry;
        while ((dirEntry = readdir(snapDir)) != NULL)
            if (dirEntry->d_type == DT_DIR && dirEntry->d_name[0] != '.' &&
                strcmp(dirEntry->d_name, "bin") != 0)
                snapCount++;
        closedir(snapDir);

        if (snapCount > 0)
            break;
    }

    // Build the result string
    if (dpkgCount > 0)
        snprintf(pkgs, PACKAGES_LEN, COMPACT ? "%d(D)" : "%d (dpkg)",
            dpkgCount);
    if (emergeCount > 0)
        snprintf(pkgs + strlen(pkgs), PACKAGES_LEN - strlen(pkgs),
            COMPACT ? ":%d(E)" : ", %d (emerge)", emergeCount);
    if (pacmanCount > 0)
        snprintf(pkgs + strlen(pkgs), PACKAGES_LEN - strlen(pkgs),
            COMPACT ? ":%d(Pm)" : ", %d (pacman)", pacmanCount);
    if (pkgtoolCount > 0)
        snprintf(pkgs + strlen(pkgs), PACKAGES_LEN - strlen(pkgs),
            COMPACT ? ":%d(Pt)" : ", %d (pkgtool)", pkgtoolCount);
    if (rpmCount > 0)
        snprintf(pkgs + strlen(pkgs), PACKAGES_LEN - strlen(pkgs),
            COMPACT ? ":%d(R)" : ", %d (rpm)", rpmCount);
    if (flatCount > 0)
        snprintf(pkgs + strlen(pkgs), PACKAGES_LEN - strlen(pkgs),
            COMPACT ? ":%d(F)" : ", %d (flat)", flatCount);
    if (snapCount > 0)
        snprintf(pkgs + strlen(pkgs), PACKAGES_LEN - strlen(pkgs),
            COMPACT ? ":%d(S)" : ", %d (snap)", snapCount);

    // Make sure we don't start with ", " or ":"...
    int pkgsLen = strlen(pkgs);
    if (pkgsLen > 2)
    {
        if (COMPACT && pkgs[0] == ':')
            memmove(pkgs, pkgs + 1, pkgsLen);
        else if (pkgs[0] == ',' && pkgs[1] == ' ')
            memmove(pkgs, pkgs + 2, pkgsLen - 1);
    }

    return pkgs;
}

#else

char *getPackages(const char *os) { (void)os; return NULL; }

#endif
