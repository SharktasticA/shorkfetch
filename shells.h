/*
    ######################################################
    ##            SHORK UTILITY - SHORKFETCH            ##
    ######################################################
    ## Database of possible Linux shells.               ##
    ######################################################
    ## Licence: GNU GENERAL PUBLIC LICENSE Version 3    ##
    ######################################################
    ## Kali (links.sharktastica.co.uk)                  ##
    ######################################################
*/



#ifndef SHELLS
#define SHELLS

const char *SHELL_NAMES[] = {
    "bash",
    "sh",
    "zsh",
    "fish",
    "dash",
    "ksh",
    "tcsh",
    "csh",
    "mksh",
    "pdksh",
    "elvish",
    "nu",
    "ion",
    "xonsh",
    "rc"
};
static const int SHELL_NAMES_LEN = sizeof(SHELL_NAMES) / sizeof(SHELL_NAMES[0]);

#endif
