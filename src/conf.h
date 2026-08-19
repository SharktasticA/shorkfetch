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

#include "globals.h"



int deleteConf(void);
void readConf(char*, char**, int*, char**, VIEW_MODE*, int*, int*, int*);
void writeConf(char, char*, int, char*, VIEW_MODE, int, int, int);

#endif
