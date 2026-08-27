/*
    ######################################################
    ##            SHORK UTILITY - SHORKFETCH            ##
    ######################################################
    ## Functions and data relating to handling system   ##
    ## main memory and swap memory                      ##
    ######################################################
    ## Licence: GNU GENERAL PUBLIC LICENSE Version 3    ##
    ######################################################
    ## Kali (links.sharktastica.co.uk)                  ##
    ######################################################
*/



#ifndef MEMORY
#define MEMORY

typedef struct {
    long memTotal;
    long memFree;
    long memAvailable;
    long buffers;
    long cached;
    long swapTotal;
    long swapFree;
} MemInfo;



MemInfo getMemInfo(void);
char *getRAM(MemInfo mi);
char *getSwap(MemInfo mi);

#endif
