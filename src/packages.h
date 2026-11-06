/*
    ######################################################
    ##            SHORK UTILITY - SHORKFETCH            ##
    ######################################################
    ## Functions and data relating to handle system     ##
    ## packages                                         ##
    ######################################################
    ## Licence: GNU GENERAL PUBLIC LICENSE Version 3    ##
    ######################################################
    ## Kali (links.sharktastica.co.uk)                  ##
    ######################################################
*/



#ifndef PACKAGES
#define PACKAGES

#define PACKAGES_LEN        256
#define SQL_OK              0
#define SQL_OPEN_READONLY   0x00000001
#define SQL_OPEN_URI        0x00000040
#define SQL_ROW             100

static const char *PKGTOOL_DIRS[] = {
    "/var/lib/pkgtools/packages",
    "/var/log/packages"
};
static const int PKGTOOL_DIRS_LEN = sizeof(PKGTOOL_DIRS) /
    sizeof(PKGTOOL_DIRS[0]);

static const char *RPM_DBS[] = {
    "/var/lib/rpm/rpmdb.sqlite",
    "/usr/lib/sysimage/rpm/rpmdb.sqlite"
};
static const int RPM_DBS_LEN = sizeof(RPM_DBS) / sizeof(RPM_DBS[0]);

static const char *SNAP_DIRS[] = {
    "/snap",
    "/var/lib/snapd/snap"
};
static const int SNAP_DIRS_LEN = sizeof(SNAP_DIRS) / sizeof(SNAP_DIRS[0]);



char *getPackages(const char*);

#endif
