/*
    ######################################################
    ##            SHORK UTILITY - SHORKFETCH            ##
    ######################################################
    ## Functions and data relating to handling the      ##
    ## terminal emulator                                ##
    ######################################################
    ## Licence: GNU GENERAL PUBLIC LICENSE Version 3    ##
    ######################################################
    ## Kali (links.sharktastica.co.uk)                  ##
    ######################################################
*/



#include "exclusions.h"
#include "general.h"
#include "globals.h"
#include "terminal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>



/**
 * @return String containing the host terminal emulator's name; NULL if not found/applicable
 */
char *getTerminal(void)
{
    // If we don't think we're in a graphical environment, time to leave...
    if (!WAYLAND_PRESENT && !X11_PRESENT)
        return NULL;

    char *terminal = NULL;

    // Try the easy way ($TERM_PROGRAM) first
    char *termProgram = getenv("TERM_PROGRAM");
    if (termProgram && termProgram[0] != '\0')
        terminal = strdup(termProgram);

    // Try looking through our parent processes to get the name
    if (!terminal)
    {
        Process process = getParentProcess(getpid());
        while (process.pid > 1)
        {
            // Flags if we must not use this process as our terminal
            int notTerminal = 0;

            // We must skip wrappers like doas, su or sudo, and possible shells
            for (int i = 0; i < EXCLUDED_TERMINAL_PROCS_LEN; i++)
            {
                if (strcmp(process.name, EXCLUDED_TERMINAL_PROCS[i]) == 0)
                {
                    notTerminal = 1;
                    break;
                }
            }

            // We must also skip shell script parents
            if (!notTerminal)
            {
                size_t len = strlen(process.name);
                if (len > 3 && strcmp(process.name + len - 3, ".sh") == 0)
                    notTerminal = 1;
            }

            if (!notTerminal)
            {
                terminal = strdup(process.name);
                break;
            }

            process = getParentProcess(process.pid);
        }
    }

    // As a fallback, we can also try $TERM to get the terminal's basic 
    // capabilities like "xterm-256color"
    if (!terminal)
    {
        const char *TERM = getenv("TERM");
        if (TERM && TERM[0] != '\0')
        {
            if (COMPACT) terminal = strdup(TERM);
            else
            {
                size_t termLen = strlen(TERM) + 11 + 1;
                terminal = malloc(termLen);
                if (terminal)
                    snprintf(terminal, termLen, "%s compatible", TERM);
            }
        }
    }

    // Do some cleaning if needed
    if (terminal)
    {
        size_t terminalLen = strlen(terminal);

        // Remove trailing hyphen from "gnome-terminal-" (etc.)
        if (terminalLen > 0 && terminal[terminalLen - 1] == '-')
            terminal[terminalLen - 1] = '\0';

        // Remove "agent" from "ptyxis-agent"
        if (terminalLen > 6 && terminal[6] == '-' && strncmp(terminal, "ptyxis", 6) == 0)
            terminal[6] = '\0';
    }

    return terminal;
}
