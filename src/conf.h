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
#define CONF_VER_LEN        10
#define MAX_FIELDS          50



typedef enum
{
    BULLETS,
    NORMAL
} VIEW_MODE;



typedef struct
{
    // Version (default: current SHORKFETCH version)
    char ver[CONF_VER_LEN];
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
    // Separator colour (default: "off")
    char colSep[CONF_COL_LEN];
    // Compact view (default: 0)
    int compact;
    // Fields
    char fields[CONF_FIELDS_LEN];
    // Maximum data unit (default: 'p')
    char maxUnit;
    // View mode (default: NORMAL)
    VIEW_MODE mode;
    // Show SHORK ASCII art (default: 1)
    int showArt;
    // Show escape codes (default: 1)
    int showEsc;
    // Show Intel Hybrid Architecture-specific core/thread counting
    // (default: 1)
    int showIHA;
    // Show IP fields (default: 1)
    int showIP;
} Config;

typedef struct
{
    const char *id;
    const char *name;
} FieldInfo;



static const FieldInfo POSSIBLE_FIELDS[] =
{
    { " ",      "(blank line)" },
    { "---",    "(separator)" },
    { "log",    "Login (user@hostname)" },
    { "os",     "Operating system" },
    { "krn",    "Kernel" },
    { "upt",    "Uptime" },
    { "pkgs",   "Packages" },
    { "loc",    "Locale(s)" },
    { "scn",    "Screen(s)" },
    { "de",     "Desktop environment" },
    { "wm",     "Window manager and/or Wayland compositor" },
    { "trm",    "Terminal emulator/console size" },
    { "sh",     "Shell" },
    { "cpu",    "CPU" },
    { "gpu",    "GPU(s)" },
    { "ram",    "Physical memory" },
    { "swap",   "Swap memory" },
    { "dsk",    "Disk size(s)" },
    { "root",   "Root partition size" },
    { "lip",    "Local IP address" },
    { "xip",    "External IP address" },
    { "clrs",   "ANSI escape code base & bright 16-colour palette" },
    { "clba",   "ANSI escape code base 8-colour palette" },
    { "clbr",   "ANSI escape code bright 8-colour palette" }
};
static const int POSSIBLE_FIELDS_LEN = sizeof(POSSIBLE_FIELDS) /
    sizeof(POSSIBLE_FIELDS[0]);



int deleteConf(void);
int launchConf(void);
void readConf(Config*);
int validateFields(const char*, char fields[][5], int);
void writeConf(Config);

#endif
