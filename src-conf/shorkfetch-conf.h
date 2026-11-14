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



#if defined(__x86_64__) || defined(__i386__)
void abdMenu(void);
#endif

int addField(void);

#if defined(__aarch64__) || defined(__arm__)
void armRevMenu(void);
#endif

void asciiArtMenu(void);
void bulletCharMenu(void);
void colAccMenu(void);
void colBulMenu(void);
void colMenu(char*, char*, char*);
void colPclMenu(void);
void colPcmMenu(void);
void colPchMenu(void);
void colSepMenu(void);
void compViewMenu(void);
void escCodesMenu(void);
int fieldActionsMenu(char*, const int);
void fieldsMenu(void);

#if defined(__x86_64__) || defined(__i386__)
void ihaMenu(void);
#endif

void ipMenu(void);
int mainMenu(void);
void maxUnitMenu(void);
int modifyField(MenuItem, const int);
int populateFieldsMenu(MenuItem*, const int);
void saveFields(void);
void toggleOptionMenu(char*, char*, int*, int);
void viewModeMenu(void);

#endif
