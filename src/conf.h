/*
    ######################################################
    ##            SHORK UTILITY - SHORKFETCH            ##
    ######################################################
    ## Functions for reading and writing user settings  ##
    ## to a configuration file                          ##
    ######################################################
    ## Licence: GNU GENERAL PUBLIC LICENSE Version 3    ##
    ######################################################
    ## Kali (links.sharktastica.co.uk)                  ##
    ######################################################
*/



#ifndef CONF
#define CONF



#define CONF_COL_LEN        16
#define CONF_FIELDS_LEN     350
#define MAX_FIELDS          50



typedef enum
{
    NORMAL,
    BULLETS
} VIEW_MODE;



typedef struct
{
    // Bullet point characater (default: '*')
    char charBullet;
    // Accent colour (default: "bright_cyan")
    char colAccent[CONF_COL_LEN];
    // Bullet point colour (default: "cyan")
    char colBullet[CONF_COL_LEN];
    // High % colour (default: "red")
    char colPctHigh[CONF_COL_LEN];
    // Low % colour (default: "green")
    char colPctLow[CONF_COL_LEN];
    // Medium % colour (default: "yellow")
    char colPctMed[CONF_COL_LEN];
    // Compact view (default: 0)
    int compact;
    // Fields
    char fields[CONF_FIELDS_LEN];
    // Maximum data unit (default: 'p')
    char maxUnit;
    // View mode (default: NORMAL)
    VIEW_MODE mode;
    // No SHORK ASCII art (default: 0)
    int noArt;
    // No escape codes (default: 0)
    int noEsc;
    // No Intel Hybrid Architecture-specific CPU core counting (default: 0)
    int noIHA;
    // No IP fields (default: 0)
    int noIP;
} Config;



static const char *POSSIBLE_FIELDS[] =
{
    " ",    // Blank line
    "---",  // Separator
    "head", // user@host header
    "os",   // Operating system
    "krn",  // Kernel
    "upt",  // Uptime
    "pkgs", // Packages
    "loc",  // Locale(s)
    "scn",  // Screen(s)
    "de",   // Desktop environment
    "wm",   // Window manager and/or Wayland compositor
    "trm",  // Terminal emulator/console size
    "sh",   // Shell
    "cpu",  // CPU
    "gpu",  // GPU(s)
    "ram",  // System memory
    "swap", // Swap memory
    "dsk",  // Disk size(s)
    "root", // Root partition size
    "lip",  // Local IP address
    "clrs", // ANSI escape code 16-colour palette
    "clba", // ANSI escape code base 8-colour palette
    "clbr"  // ANSI escape code bright 8-colour palette
};
static const int POSSIBLE_FIELDS_LEN = sizeof(POSSIBLE_FIELDS) /
    sizeof(POSSIBLE_FIELDS[0]);



int deleteConf(void);
int launchConf(void);
void readConf(Config*);
void writeConf(Config);

#endif
