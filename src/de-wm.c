/*
    ######################################################
    ##            SHORK UTILITY - SHORKFETCH            ##
    ######################################################
    ## Functions and data relating to handling desktop  ##
    ## environments and window managers                 ##
    ######################################################
    ## Licence: GNU GENERAL PUBLIC LICENSE Version 3    ##
    ######################################################
    ## Kali (links.sharktastica.co.uk)                  ##
    ######################################################
*/



#include "de-wm.h"
#include "general.h"
#include "globals.h"

#include <stdlib.h>
#include <string.h>



/**
 * @return String containing the active display environment's name; NULL if
 *         not found/applicable
 */
char *getDE(void)
{
    // If we don't think we're in a graphical environment, time to leave...
    if (!WAYLAND_PRESENT && !X11_PRESENT)
        return NULL;

    char *de = NULL;

    // Test standardised DE environment var
    if (XDG_CURRENT_DESKTOP && XDG_CURRENT_DESKTOP[0] != '\0')
        de = strdup(XDG_CURRENT_DESKTOP);

    // Do some cleaning if needed
    if (de)
    {
        // Remove "ubuntu" in "ubuntu:GNOME"
        if (strncmp(de, "ubuntu:", 7) == 0)
            memmove(de, de + 7, strlen(de + 7) + 1);

        // Remove "X-" in "X-Cinnamon"
        if (strncmp(de, "X-", 2) == 0)
            memmove(de, de + 2, strlen(de + 2) + 1);

        // "Prettify" XFCE to Xfce
        if (strncmp(de, "XFCE", 4) == 0)
        {
            free(de);
            de = strdup("Xfce");
        }

        // Discard ":Unity7:ubuntu" from "Unity:Unity7:ubuntu" (etc.)
        if (strncmp(de, "Unity", 5) == 0)
        {
            char *needle = strchr(de, ':');
            if (needle) *needle = '\0';
        }
        
        // Discard ":wlroots" from "sway:wlroots"
        if (strncmp(de, "sway", 4) == 0)
        {
            char *needle = strchr(de, ':');
            if (needle) *needle = '\0';
        }
    }

    return de;
}

/**
 * @param de Desktop enivornment's name
 * @return String containing the active window manager's name; NULL if not
 *         found/applicable
 */
char *getWM(char **de)
{
    // If we don't think we're in a graphical environment, time to leave...
    if (!WAYLAND_PRESENT && !X11_PRESENT)
        return NULL;

    // Cinnamon's WM (Muffin) is internal, we have to assume instead of look
    // for the process
    if (de && *de && strstr(*de, "Cinnamon") != NULL)
        return strdup("Muffin");

    // Run through our WM database
    for (int i = 0; i < WINDOW_MANAGERS_LEN; i++)
    {
        if (procExists(WINDOW_MANAGERS[i].cmd, 0))
        {
            // If DE == WM, we may treat this as just a WM
            if (de && *de)
            {
                // Convert both subjects to all caps for a case-insensitive 
                // comparison
                char *deCaps = strdup(*de);
                    for (int j = 0; deCaps[j]; j++)
                        if (deCaps[j] >= 'a' && deCaps[j] <= 'z')
                            deCaps[j] -= 32;
                char *wmCaps = strdup(WINDOW_MANAGERS[i].name);
                for (int j = 0; wmCaps[j]; j++)
                    if (wmCaps[j] >= 'a' && wmCaps[j] <= 'z')
                        wmCaps[j] -= 32;

                if (strstr(deCaps, wmCaps) != NULL)
                {
                    free(deCaps);
                    free(wmCaps);
                    char *wm = strdup(WINDOW_MANAGERS[i].name);
                    *de = wm;
                    return wm;
                }

                free(deCaps);
                free(wmCaps);
            }

            return strdup(WINDOW_MANAGERS[i].name);
        }
    }

    // If we haven't found a WM but we have a DE, there's a good chance DE/
    // WM are one and the same
    if (de && *de)
        return *de;

    return NULL;
}
