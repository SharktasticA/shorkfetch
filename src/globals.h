/*
    ######################################################
    ##            SHORK UTILITY - SHORKFETCH            ##
    ######################################################
    ## Global variables                                 ##
    ######################################################
    ## Licence: GNU GENERAL PUBLIC LICENSE Version 3    ##
    ######################################################
    ## Kali (links.sharktastica.co.uk)                  ##
    ######################################################
*/



#ifndef GLOBALS
#define GLOBALS

#include <sys/ioctl.h>
#include <stdlib.h>



typedef enum
{
    NORMAL = 0,
    BULLETS,
} VIEW_MODE;



#define MAX_FIELDS  50

static const char *POSSIBLE_FIELDS[] =
{
    " ",    // Blank line
    "---",  // Category separator
    "os",   // Operating system
    "krn",  // Kernel
    "upt",  // Uptime
    "pkgs", // Packages
    "scn",  // Screen(s)
    "de",   // Desktop environment
    "wm",   // Window manager and/or Wayland compositor
    "trm",  // Terminal emulator/console size
    "sh",   // Shell
    "cpu",  // CPU
    "gpu",  // GPU(s)
    "ram",  // System memory
    "swap", // Swap memory
    "root", // Root partition size
    "lip",  // Local IP address
    "clrs"  // ANSI escape code colour palette
};
static const int POSSIBLE_FIELDS_LEN = sizeof(POSSIBLE_FIELDS) / sizeof(POSSIBLE_FIELDS[0]);

extern char *COLOUR;
extern int COMPACT;
extern char *HOME;
extern struct winsize TERM_SIZE;
extern int WAYLAND_PRESENT;
extern int X11_PRESENT;
extern char *XDG_CURRENT_DESKTOP;

#endif
