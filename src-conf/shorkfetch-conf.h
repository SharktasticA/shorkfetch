/*
    ######################################################
    ##          SHORK UTILITY - SHORKFETCH-CONF         ##
    ######################################################
    ## Main program logic                               ##
    ######################################################
    ## Licence: GNU GENERAL PUBLIC LICENSE Version 3    ##
    ######################################################
    ## Kali (links.sharktastica.co.uk)                  ##
    ######################################################
*/



#ifndef SHORKFETCH_CONF
#define SHORKFETCH_CONF

#include "../shorkcommon/shorkmenu.h"

#include "../src/conf.h"



extern Config CONFIG;
extern char FIELDS[MAX_FIELDS][5];
extern int NO_FIELDS;



int addField(void);
void asciiArtMenu(void);
void bulletCharMenu(void);
void colAccMenu(void);
void colBulMenu(void);
void colPclMenu(void);
void colPcmMenu(void);
void colPchMenu(void);
void colMenu(char*, char*, char*);
void compViewMenu(void);
void escCodesMenu(void);
int fieldActionsMenu(char*, const int);
void fieldsMenu(void);
void ihaMenu(void);
void ipMenu(void);
int mainMenu(void);
void maxUnitMenu(void);
int modifyField(MenuItem, const int);
int populateFieldsMenu(MenuItem*, const int);
void saveFields(void);
void toggleOptionMenu(char*, char*, int*, int);
void viewModeMenu(void);

#endif
