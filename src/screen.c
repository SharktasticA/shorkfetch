/*
    ######################################################
    ##            SHORK UTILITY - SHORKFETCH            ##
    ######################################################
    ## Functions and data relating to handling screens  ##
    ######################################################
    ## Licence: GNU GENERAL PUBLIC LICENSE Version 3    ##
    ######################################################
    ## Kali (links.sharktastica.co.uk)                  ##
    ######################################################
*/



#include "general.h"
#include "globals.h"
#include "screen.h"

#include <dirent.h>
#include <linux/limits.h>
#include <stdlib.h>
#include <string.h>



/**
 * @param count Number of screens detected (intended to be used by reference)
 * @return Pointer to Screen structs containing detected GPUs
 */
Screen *getScreens(int *count)
{
    if (!count) return NULL;

    // If we don't think we're in a graphical environment, time to leave...
    if (!WAYLAND_PRESENT && !X11_PRESENT)
        return NULL;

    Screen *screens = NULL;

    // Try getting screens with xrandr (X11)
    FILE *fStream = popen("xrandr --current 2>/dev/null", "r");
    if (fStream)
    {
        // What we use to read lines of xrandr output in to
        char buffer[512];
        // Flag when we're reading a connected screen
        int in = 0;

        while (fgets(buffer, 512, fStream))
        {
            // Only process lines for things "connected" or inside a "connected"'s block
            char *isConnected = strstr(buffer, " connected");
            if (isConnected) 
            {
                // Reallocate screens array to take into account new screen
                screens = realloc(screens, ((*count) + 1) * sizeof(Screen));
                memset(&screens[(*count)], 0, sizeof(Screen));

                // Parse connector name
                char pConnector[64] = {0};
                sscanf(buffer, "%63s", pConnector);
                screens[*count].connector = strdup(pConnector);

                // Replace "Virtual-" with "Virt-" if present
                char *virtNeedle = strstr(screens[*count].connector, "Virtual-");
                if (virtNeedle)
                {
                    memcpy(virtNeedle, "Virt-", 5);
                    memmove(virtNeedle + 5, virtNeedle + 8, strlen(virtNeedle + 8) + 1);
                }

                // Flag if connector is for primary screen
                screens[*count].isPrimary = strstr(buffer, " primary ") != NULL;

                // Parse physical size
                screens[*count].physX = 0.0;
                screens[*count].physY = 0.0;
                char *needle = strstr(buffer, "mm x");
                if (needle)
                {
                    char *start = needle;
                    while (start > buffer && *(start - 1) != ' ')
                        start--;

                    int pPhysX = 0, pPhysY = 0;
                    if (sscanf(start, "%dmm x %dmm", &pPhysX, &pPhysY) == 2)
                    {
                        screens[*count].physX = pPhysX;
                        screens[*count].physY = pPhysY;
                    }
                }

                // Parse resolution
                needle = isConnected;
                int pResX = 0, pResY = 0;
                while (*needle && (*needle < '0' || *needle > '9')) needle++;
                sscanf(needle, "%dx%d", &pResX, &pResY);

                screens[*count].resX = pResX;
                screens[*count].resY = pResY;

                // Flag that we are in a block and thus should search for more info on other lines
                in = 1;
            }
            else if (strstr(buffer, "disconnected") || !in)
            {
                in = 0;
                continue;
            }

            // Look for current refresh rate
            if (strchr(buffer, '*'))
            {
                char *start = strchr(buffer, '*');
                while (start > buffer)
                {
                    char c = *(start - 1);
                    if ((c >= '0' && c <= '9') || c == '.') start--;
                    else break;
                }
                screens[*count].refresh = (int)(atof(start) + 0.5);

                // At this point, we got everything we can for this screen, so let's go
                (*count)++;
                in = 0;
                continue;
            }
        }

        if (in) (*count)++;

        pclose(fStream);
    }

    // If we still have nothing, we can try DRM as an X11/Wayland agnostic fallback
    if (!screens)
    {
        DIR *dirStream = opendir("/sys/class/drm");

        if (dirStream)
        {
            struct dirent *entry;
            while ((entry = readdir(dirStream)))
            {
                // Skip non-connector entries
                if (strstr(entry->d_name, "-") == NULL)
                    continue;

                // Prepare to test connector status
                char path[PATH_MAX];
                snprintf(path, PATH_MAX, "/sys/class/drm/%s/status", entry->d_name);

                FILE *fileStream = fopen(path, "r");
                if (!fileStream) continue;

                // Check status
                char status[64] = {0};
                fgets(status, 64, fileStream);
                fclose(fileStream);

                // Move on if anything but "connected"
                if (strncmp(status, "connected", 9) != 0)
                    continue;

                // Prepare to parse mode for resolution
                snprintf(path, PATH_MAX, "/sys/class/drm/%s/modes", entry->d_name);
                fileStream = fopen(path, "r");
                if (!fileStream) continue;

                char mode[64] = {0};
                fgets(mode, 64, fileStream);
                fclose(fileStream);

                // Parse mode for resolution
                int pResX = 0, pResY = 0;
                sscanf(mode, "%dx%d", &pResX, &pResY);

                // If we got nothing, no point of continuing...
                if (pResX <= 0 || pResY <= 0) continue;

                // Reallocate screens array to take into account new screen
                screens = realloc(screens, ((*count) + 1) * sizeof(Screen));
                memset(&screens[(*count)], 0, sizeof(Screen));

                // Populate screen data
                screens[(*count)].connector = strdup(entry->d_name);
                screens[(*count)].physX = 0.0;
                screens[(*count)].physY = 0.0;
                screens[(*count)].resX = pResX;
                screens[(*count)].resY = pResY;
                screens[(*count)].refresh = 0;

                // Remove "cardX-" prefix if present
                if (strncmp(screens[(*count)].connector, "card", 4) == 0)
                    memmove(screens[(*count)].connector, screens[(*count)].connector + 6, strlen(screens[(*count)].connector + 6) + 1);

                // Replace "Virtual-" with "Virt-" if present
                char *virtNeedle = strstr(screens[*count].connector, "Virtual-");
                if (virtNeedle)
                {
                    memcpy(virtNeedle, "Virt-", 5);
                    memmove(virtNeedle + 5, virtNeedle + 8, strlen(virtNeedle + 8) + 1);
                }

                (*count)++;
            }

            closedir(dirStream);
        }
    }

    return screens;
}

/**
 * @param screen Screen struct containing raw specifications for the given
 *               screen
 * @return String containing the screen's assembled specifications
 */
char *interpretScreen(Screen *screen)
{
    // Quick check to make sure we have something to work with...
    if (screen->resX <= 0 || screen->resY <= 0)
        return NULL;

    const int SCREEN_SIZE = 128;
    char *screenStr = malloc(SCREEN_SIZE);

    // Prepare physical screen size (NOT PRESENTLY USED PENDING ACCURACY
    // IMPROVEMENTS)
    /*char physSize[32] = "";
    if (screen->physX > 0.0 && screen->physY > 0.0)
    {
        float diagMm = fSqrt(screen->physX * screen->physX + screen->physY * screen->physY);
        float diagIn = diagMm / 25.4f;

        if ((int)(diagIn * 10.0f) % 10 == 0)
            snprintf(physSize, 32, "%d\" ", (int)diagIn);
        else
        {
            float diagInRounded = (float)(int)(diagIn * 10.0f + 0.5f) / 10.0f;
            if (diagInRounded == (int)diagInRounded)
                snprintf(physSize, 32, "%d\" ", (int)diagInRounded);
            else
                snprintf(physSize, 32, "%.1f\" ", diagInRounded);
        }
    }*/

    // Prepare refresh rate
    char refresh[32] = "";
    if (screen->refresh > 0)
    {
        if (COMPACT)
            snprintf(refresh, 32, "@%d", screen->refresh);
        else
            snprintf(refresh, 32, " @ %dHz", screen->refresh);
    }

    // Prepare connector name
    char connector[32] = "";
    if (screen->connector[0] != '\0')
        snprintf(connector, 32, " (%s)", screen->connector);
    free(screen->connector);

    // Assemble the string
    if (!COMPACT)
        snprintf(screenStr, SCREEN_SIZE, "%dx%d%s%s", screen->resX, screen->resY, refresh, connector);
    else
        snprintf(screenStr, SCREEN_SIZE, "%dx%d%s", screen->resX, screen->resY, refresh);

    return screenStr;
}
