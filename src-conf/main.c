/*
    ######################################################
    ##          SHORK UTILITY - SHORKFETCH-CONF         ##
    ######################################################
    ## A menu-driven configuration editor for           ##
    ## SHORKFETCH/shorkfetch.conf                       ##
    ######################################################
    ## Licence: GNU GENERAL PUBLIC LICENSE Version 3    ##
    ######################################################
    ## Kali (links.sharktastica.co.uk)                  ##
    ######################################################
*/



#include "../shorkcommon/general.h"
#include "../shorkcommon/shorkmenu.h"

#include "shorkfetch-conf.h"

#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



void showHelp(void)
{
    TERM_SIZE = getTerminalSize();

    WORD_WRAPPED *desc = wordWrap("SHORKFETCH's menu-driven configuration "
        "editor.\n", TERM_SIZE.ws_col, NULL, NULL, 0, 0);
    printf("%s\n", desc->str);
    free(desc->str);
    free(desc);

    WORD_WRAPPED *usage = wordWrap("Usage: shorkfetch-conf [OPTIONS]\n\n",
        TERM_SIZE.ws_col, NULL, NULL, 0, 0);
    printf("%s", usage->str);
    free(usage->str);
    free(usage);

    WORD_WRAPPED *options = wordWrap("Options:\n", TERM_SIZE.ws_col, NULL,
        NULL, 0, 0);
    printf("%s", options->str);
    free(options->str);
    free(options);

    WORD_WRAPPED *help = wordWrap("-h, --help     Displays help "
        "information and exits\n", TERM_SIZE.ws_col, "               ",
        NULL, 0, 0);
    printf("%s", help->str);
    free(help->str);
    free(help);

    WORD_WRAPPED *reset = wordWrap("-r, --reset    Resets to default, "
        "deletes configuration file and exits\n", TERM_SIZE.ws_col,
        "               ", NULL, 0, 0);
    printf("%s", reset->str);
    free(reset->str);
    free(reset);

    WORD_WRAPPED *version = wordWrap("-v, --version  Displays version "
        "number and exits\n", TERM_SIZE.ws_col, "               ", NULL, 0,
        0);
    printf("%s", version->str);
    free(version->str);
    free(version);
}

int main(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++)
    {
        if ((strcmp(argv[i], "-h") == 0) ||
            (strcmp(argv[i], "--help") == 0))
        {
            showHelp();
            return 0;
        }
        else if ((strcmp(argv[i], "-r") == 0) ||
            (strcmp(argv[i], "--reset") == 0))
        {
            int result = deleteConf();
            if (result)
                printf("SHORKFETCH configuration reset\n");
            else
                printf("WARNING: SHORKFETCH configuration already "
                "default\n");
            return 0;
        }
        else if (strcmp(argv[i], "-v") == 0 ||
            strcmp(argv[i], "--version") == 0)
        {
            printf("SHORKFETCH Configurator %s\n", VERSION);
            return 0;
        }
        else
        {
            printf("ERROR: unrecognised option \"%s\"\n", argv[i]);
            return 1;
        }
    }

    return mainMenu();
}
