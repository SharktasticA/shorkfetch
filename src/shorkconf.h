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



#ifndef SHORKCONF
#define SHORKCONF

int deleteConf(void);
void readConf(char*, char**, int*, char**, VIEW_MODE*, int*, int*);
void writeConf(char, char*, int, char*, VIEW_MODE, int, int);

#endif
