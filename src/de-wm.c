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

#include "trie.h"


#ifndef SHORK_DISKETTE

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
    
    // assemble WM proc names from database into radix-trie and search for match
    struct trie* procNames = trie_new();
    for (int i = 0; i < WINDOW_MANAGERS_LEN; i++)
        trie_add_word( procNames, WINDOW_MANAGERS[i].cmd, strlen(WINDOW_MANAGERS[i].cmd), i );
    int wmID = findProcs(procNames);
    trie_free(procNames);
    
    // if no match
    if (wmID == -1) {
        // If we have a DE but no WM, they're probably one and the same
        if (de && *de)
            return *de;
        
        return NULL;
    }
    
    WM wm = WINDOW_MANAGERS[wmID];
    
    // if de not known, no extra work needed
    if (!de || !*de)
        return strdup(wm.name);
    
    // Check if DE == WM, in which case we treat this as just a WM
    // Convert both strings to all caps for a case-insensitive check
    char *deCaps = strdup(*de);
        for (int j = 0; deCaps[j]; j++)
            if (deCaps[j] >= 'a' && deCaps[j] <= 'z')
                deCaps[j] -= 32;
    char *wmCaps = strdup(wm.name);
    for (int j = 0; wmCaps[j]; j++)
        if (wmCaps[j] >= 'a' && wmCaps[j] <= 'z')
            wmCaps[j] -= 32;
    
    int DEeqWM = strstr(deCaps, wmCaps) != NULL;
    free(deCaps);
    free(wmCaps);
    
    if (DEeqWM)
        return *de = strdup(wm.name);
    
    return strdup(wm.name);
}

#else

char *getDE(void) { return NULL; }
char *getWM(char **de)  { return NULL; }

#endif
