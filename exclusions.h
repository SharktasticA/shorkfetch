/*
    ######################################################
    ##            SHORK UTILITY - SHORKFETCH            ##
    ######################################################
    ## List of items to be excluded during some         ##
    ## processings.                                     ##
    ######################################################
    ## Licence: GNU GENERAL PUBLIC LICENSE Version 3    ##
    ######################################################
    ## Kali (links.sharktastica.co.uk)                  ##
    ######################################################
*/



#ifndef EXCLUSIONS
#define EXCLUSIONS

// PCI DEVICE IDs that should be excluded when processing potential GPUs
static const int EXCLUDED_PCI_DIDS[] = {
    // Not presently used, but here if we need it...
};
static const int EXCLUDED_PCI_DIDS_LEN = sizeof(EXCLUDED_PCI_DIDS) / sizeof(EXCLUDED_PCI_DIDS[0]);



// Process names that should be excluded when looking for the host terminal's
// name
const char *EXCLUDED_TERMINAL_PROCS[] = {
    // Shells
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
    "rc",

    // Wrappers
    "sudo",
    "doas",
    "su",
    "pkexec",
    "runuser",
    "firejail",
    "bubblewrap",
    "flatpak",

    // Misc
    "conmon",
    "systemd"
};
static const int EXCLUDED_TERMINAL_PROCS_LEN = sizeof(EXCLUDED_TERMINAL_PROCS) / sizeof(EXCLUDED_TERMINAL_PROCS[0]);

#endif
