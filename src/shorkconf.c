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



#include "globals.h"
#include "shorkconf.h"

#include <linux/limits.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



/**
 * Deletes shorkfetch.conf.
 * @return 1 if deleted successfully; 0 if not
 */
int deleteConf(void)
{
    char path[PATH_MAX];
    snprintf(path, PATH_MAX, "%s/.config/shorkutils/shorkfetch.conf", HOME);

    if (remove(path) != 0)
        return 0;
    else
        return 1;
}

/**
 * Reads shorkfetch.conf.
 * @param bullet
 * @param colour
 * @param compact
 * @param fields
 * @param noIP
 * @param showShork
 * @param useBullets
 */
void readConf(char *bullet, char **colour, int *compact, char **fields, int *noIP, int *showShork, int *useBullets)
{
    char path[PATH_MAX];
    snprintf(path, PATH_MAX, "%s/.config/shorkutils/shorkfetch.conf", HOME);

    FILE *shorkconf = fopen(path, "r");
    if (shorkconf)
    {
        char line[512];
        while (fgets(line, sizeof(line), shorkconf))
        {
            if (line[0] == '\n') continue;
            line[strcspn(line, "\n")] = '\0';

            char *eq = strchr(line, '=');
            if (!eq) continue;
            *eq = '\0';

            char *key   = line;
            char *value = eq + 1;

            if (strcmp(key, "bullet") == 0)
                *bullet = value[0];
            else if (strcmp(key, "colour") == 0)
            {
                free(*colour);
                *colour = strdup(value);
            }
            else if (strcmp(key, "compact") == 0)
                *compact = atoi(value);
            else if (strcmp(key, "fields") == 0)
            {
                free(*fields);
                *fields = strdup(value);
            }
            else if (strcmp(key, "noIP") == 0)
                *noIP = atoi(value);
            else if (strcmp(key, "showShork") == 0)
                *showShork = atoi(value);
            else if (strcmp(key, "useBullets") == 0)
                *useBullets = atoi(value);
        }
        fclose(shorkconf);
    }
}

/**
 * Writes shorkfetch.conf.
 * @param bullet
 * @param colour
 * @param compact
 * @param fields
 * @param noIP
 * @param showShork
 * @param useBullets
 */
void writeConf(char bullet, char *colour, int compact, char *fields, int noIP, int showShork, int useBullets)
{
    char path[PATH_MAX];

    // Create directory to store the conf file - this is broken into parts in
    // case the system does not have .config/
    snprintf(path, PATH_MAX, "%s/.config/", HOME);
    mkdir(path, 0755);
    strncat(path, "shorkutils/", PATH_MAX - strlen(path) - 1);
    mkdir(path, 0755);

    strncat(path, "shorkfetch.conf", PATH_MAX - strlen(path) - 1);
    FILE *shorkconf = fopen(path, "w");
    if (shorkconf)
    {
        fprintf(shorkconf, "bullet=%c\n", bullet);
        fprintf(shorkconf, "colour=%s\n", colour);
        fprintf(shorkconf, "compact=%d\n", compact);
        fprintf(shorkconf, "fields=%s\n", fields);
        fprintf(shorkconf, "noIP=%d\n", noIP);
        fprintf(shorkconf, "showShork=%d\n", showShork);
        fprintf(shorkconf, "useBullets=%d\n", useBullets);
        fclose(shorkconf);
    }
}
