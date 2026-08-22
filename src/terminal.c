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
 * Gets the host terminal emulator name and console size.
 * @return String containing the the result name and size; NULL if not
 *         found/applicable
 */
char *getTerminal(void)
{
    // If we don't think we're in a graphical environment, time to leave...
    if (!WAYLAND_PRESENT && !X11_PRESENT)
        return NULL;

    char *result = malloc(TERMINAL_LEN);
    if (!result)
        return NULL;
    result[0] = '\0';

    const char *termProgramEnv = getenv("TERM_PROGRAM");
    // Try the easy way ($TERM_PROGRAM) first
    char *termProgram = strdup(termProgramEnv ? termProgramEnv : "");
    if (!termProgram)
        return NULL;

    // Flags if GNU Screen was detected. This is needed because a prior
    // Screen session can leave $TERM stale and thus not a reliable
    // indicator of whether we're actually in Screen or not.
    int inScreen = 0;

    // If nothing from $TERM_PROGRAM, try looking through our parent
    // processes to get the name
    if (termProgram[0] == '\0')
    {
        PROCESS process = getParentProcess(getpid());
        while (process.pid > 1)
        {
            // Flags if we must not use this process as our terminal
            int notTerminal = 0;

            // We must skip wrappers like doas, su or sudo, and possible
            // shells
            for (int i = 0; i < EXCLUDED_TERMINAL_PROCS_LEN; i++)
            {
                if (strstr(process.name, EXCLUDED_TERMINAL_PROCS[i]))
                {
                    // If we found that we're in GNU Screen, mark that down
                    // for later
                    if (strcmp(process.name, "screen") == 0)
                        inScreen = 1;
                    notTerminal = 1;
                    break;
                }
            }

            // We must also skip shell script parents
            if (!notTerminal)
            {
                int len = strlen(process.name);
                if (len > 3 && strcmp(process.name + len - 3, ".sh") == 0)
                    notTerminal = 1;
            }

            // Found our terminal name?
            if (!notTerminal)
            {
                char *dup = strdup(process.name);
                if (!dup)
                    break;
                free(termProgram);
                termProgram = dup;
                break;
            }

            process = getParentProcess(process.pid);
        }
    }

    // Get $TERM as a possible fallback or supplement because it can inform
    // basic, generic capabilities like "xterm-256color" 
    const char *term = getenv("TERM");

#ifndef NO_STR_CLEANING
    // Do some cleaning to $TERM_PROGRAM or process name
    if (termProgram && termProgram[0] != '\0')
    {
        int termProgramLen = strlen(termProgram);

        // Remove trailing hyphen from "gnome-terminal-" (etc.)
        if (termProgramLen > 0 && termProgram[termProgramLen - 1] == '-')
            termProgram[termProgramLen - 1] = '\0';

        // Remove "agent" from "ptyxis-agent"
        if (termProgramLen > 6 && termProgram[6] == '-' &&
            strncmp(termProgram, "ptyxis", 6) == 0)
            termProgram[6] = '\0';
    }
#endif

    // Prefer to use $TERM_PROGRAM or process name
    if (termProgram[0] != '\0')
    {
        // tmux is not a terminal emulator, so we create a distinction with
        // also including $TERM if available
        if(strcmp(termProgram, "tmux") == 0 && term && term[0] != '\0')
        {
            if (COMPACT)
                snprintf(result, TERMINAL_LEN, "%s (%dx%d)", term,
                    TERM_SIZE.ws_col, TERM_SIZE.ws_row);
            else
                snprintf(result, TERMINAL_LEN, "%s (%s, %dx%d)",
                    termProgram, term, TERM_SIZE.ws_col, TERM_SIZE.ws_row);
        }
        // Ditto for GNU Screen
        else if(inScreen && term && strncmp(term, "screen", 5) == 0)
        {
            if (COMPACT)
                snprintf(result, TERMINAL_LEN, "%s (%dx%d)",
                    termProgram, TERM_SIZE.ws_col, TERM_SIZE.ws_row);
            else
                snprintf(result, TERMINAL_LEN, "screen (%s, %dx%d)",
                    termProgram, TERM_SIZE.ws_col, TERM_SIZE.ws_row);
        }
        else
            snprintf(result, TERMINAL_LEN, "%s (%dx%d)", termProgram,
                TERM_SIZE.ws_col, TERM_SIZE.ws_row);
    }
    // As a fallback, just use $TERM 
    else if (term && term[0] != '\0')
        snprintf(result, TERMINAL_LEN, "%s (%dx%d)", term, TERM_SIZE.ws_col,
            TERM_SIZE.ws_row);

    free(termProgram);
    if (result[0] == '\0')
    {
        free(result);
        return NULL;
    }
    else
        return result;
}
