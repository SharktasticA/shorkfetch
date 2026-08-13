/*
    ######################################################
    ##            SHORK UTILITY - SHORKFETCH            ##
    ######################################################
    ## Functions and data relating to handling disk and ##
    ## partition sizes                                  ##
    ######################################################
    ## Licence: GNU GENERAL PUBLIC LICENSE Version 3    ##
    ######################################################
    ## Kali (links.sharktastica.co.uk)                  ##
    ######################################################
*/



#ifndef DISK
#define DISK

#define DISK_LEN    64
#define DISKS_LEN   10
#define ROOT_LEN    64



typedef struct {
    char disks[DISKS_LEN][DISK_LEN];
    int count;
} DISKS;



DISKS *getDisks(void);
char *getRoot(void);

#endif
