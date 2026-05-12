/*
    ######################################################
    ##            SHORK UTILITY - SHORKFETCH            ##
    ######################################################
    ## Database of window managers and compositors.     ##
    ######################################################
    ## Licence: GNU GENERAL PUBLIC LICENSE Version 3    ##
    ######################################################
    ## Kali (links.sharktastica.co.uk)                  ##
    ######################################################
*/



#ifndef WMS
#define WMS

struct WM {
    // Command name substring
    const char *cmd;
    // Display name
    const char *name;
    // Flags if this WM is not normally for a DE
    const int noDE;
};



static const struct WM WINDOW_MANAGERS[] = {
    { "mutter-",        "Mutter",           0 },
    { "kwin_",          "KWin",             0 },
    { "xfwm",           "Xfwm",             0 },
    { "openbox",        "Openbox",          0 },
    { "compiz",         "Compiz",           0 },
    { "Hyprland",       "Hyprland",         1 },
    { "i3",             "i3",               1 },
    { "niri",           "niri",             1 },
    { "sway",           "sway",             1 },
    { "2bwm",           "2bwm",             1 },
    { "awesome",        "Awesome",          1 },
    { "blackbox",       "Blackbox",         1 },
    { "bspwm",          "bspwm",            1 },
    { "cwm",            "cwm",              1 },
    { "dwl",            "dwl",              1 },
    { "dwm",            "dwm",              1 },
    { "enlightenment",  "Enlightenment",    1 },
    { "evilwm",         "evilwm",           1 },
    { "fluxbox",        "Fluxbox",          1 },
    { "fvwm",           "FVWM",             1 },
    { "herbstluftwm",   "herbstluftwm",     1 },
    { "icewm",          "IceWM",            1 },
    { "jwm",            "JWM",              1 },
    { "kwm",            "KWM",              0 },
    { "labwc",          "labwc",            1 },
    { "marco",          "Marco",            0 },
    { "metacity",       "Metacity",         0 },
    { "river",          "River",            1 },
    { "sawfish",        "Sawfish",          1 },
    { "twm",            "TWM",              1 },
    { "wayfire",        "Wayfire",          1 },
    { "weston",         "Weston",           1 },
};
static const int WINDOW_MANAGERS_LEN = sizeof(WINDOW_MANAGERS) / sizeof(WINDOW_MANAGERS[0]);

#endif
