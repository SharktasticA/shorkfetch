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



#include "../shorkcommon/general.h"

#include "conf.h"

#include <linux/limits.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



#ifndef SHORK_DISKETTE

/**
 * Deletes shorkfetch.conf.
 * @return 1 if deleted successfully; 0 if not
 */
int deleteConf(void)
{
    char path[PATH_MAX];
    snprintf(path, PATH_MAX, "%s/.config/shorkutils/shorkfetch.conf",
        getenv("HOME"));

    if (remove(path) != 0)
        return 0;
    else
        return 1;
}

/**
 * Launches SHORKFETCH Configurator (shorkfetch-conf).
 * @return 0 if successful; 1 if not
 */
int launchConf(void)
{
    // Look for global shorkfetch-conf
    if (isProgramInstalled("shorkfetch-conf", 1))
    {
        int result = runCmd("shorkfetch-conf", NULL);
        return (result >= 0) ? result : 1;
    }

    // Look for shorkfetch-conf in shorkfetch's bin dir
    char *binDir = getBinDir();
    if (binDir)
    {
        char local[PATH_MAX];
        snprintf(local, sizeof(local), "%sshorkfetch-conf", binDir);

        if (isProgramInstalled(local, 1))
        {
            int result = runCmd(local, NULL);
            return (result >= 0) ? result : 1;
        }
    }
    
    printf("ERROR: could not find shorkfetch-conf\n");
    return 1;
}

/**
 * Reads shorkfetch.conf.
 */
void readConf(Config *conf)
{
    char path[PATH_MAX];
    snprintf(path, PATH_MAX, "%s/.config/shorkutils/shorkfetch.conf",
        getenv("HOME"));

    FILE *stream = fopen(path, "r");
    if (stream)
    {
        char line[512];
        while (fgets(line, sizeof(line), stream))
        {
            if (line[0] == '\n') continue;
            line[strcspn(line, "\n")] = '\0';

            char *eq = strchr(line, '=');
            if (!eq) continue;
            *eq = '\0';

            char *key = line;
            char *value = eq + 1;

            if (strcmp(key, "charBullet") == 0)
                conf->charBullet = value[0];
            else if (strcmp(key, "colAccent") == 0)
                snprintf(conf->colAccent, CONF_COL_LEN, "%s", value);
            else if (strcmp(key, "colBullet") == 0)
                snprintf(conf->colBullet, CONF_COL_LEN, "%s", value);
            else if (strcmp(key, "colPctHigh") == 0)
                snprintf(conf->colPctHigh, CONF_COL_LEN, "%s", value);
            else if (strcmp(key, "colPctLow") == 0)
                snprintf(conf->colPctLow, CONF_COL_LEN, "%s", value);
            else if (strcmp(key, "colPctMed") == 0)
                snprintf(conf->colPctMed, CONF_COL_LEN, "%s", value);
            else if (strcmp(key, "compact") == 0)
                conf->compact = atoi(value);
            else if (strcmp(key, "fields") == 0)
                snprintf(conf->fields, CONF_FIELDS_LEN, "%s", value);
            else if (strcmp(key, "maxUnit") == 0)
                conf->maxUnit = value[0];
            else if (strcmp(key, "mode") == 0)
                conf->mode = atoi(value);
            else if (strcmp(key, "noArt") == 0)
                conf->noArt = atoi(value);
            else if (strcmp(key, "noEsc") == 0)
                conf->noEsc = atoi(value);
            else if (strcmp(key, "noIHA") == 0)
                conf->noIHA = atoi(value);
            else if (strcmp(key, "noIP") == 0)
                conf->noIP = atoi(value);
        }
        fclose(stream);
    }
}

/**
 * Writes shorkfetch.conf.
 */
void writeConf(Config conf)
{
    char path[PATH_MAX];

    // Create directory to store the conf file - this is broken into parts
    // in case the system does not have .config/
    snprintf(path, PATH_MAX, "%s/.config/", getenv("HOME"));
    mkdir(path, 0755);
    strncat(path, "shorkutils/", PATH_MAX - strlen(path) - 1);
    mkdir(path, 0755);

    strncat(path, "shorkfetch.conf", PATH_MAX - strlen(path) - 1);
    FILE *stream = fopen(path, "w");
    if (stream)
    {
        fprintf(stream, "charBullet=%c\n", conf.charBullet);
        fprintf(stream, "colAccent=%s\n", conf.colAccent);
        fprintf(stream, "colBullet=%s\n", conf.colBullet);
        fprintf(stream, "colPctHigh=%s\n", conf.colPctHigh);
        fprintf(stream, "colPctLow=%s\n", conf.colPctLow);
        fprintf(stream, "colPctMed=%s\n", conf.colPctMed);
        fprintf(stream, "compact=%d\n", conf.compact);
        fprintf(stream, "fields=%s\n", conf.fields);
        fprintf(stream, "maxUnit=%c\n", conf.maxUnit);
        fprintf(stream, "mode=%d\n", conf.mode);
        fprintf(stream, "noArt=%d\n", conf.noArt);
        fprintf(stream, "noEsc=%d\n", conf.noEsc);
        fprintf(stream, "noIHA=%d\n", conf.noIHA);
        fprintf(stream, "noIP=%d\n", conf.noIP);
        fclose(stream);
    }
}

#else

int deleteConf(void) { return 1; }
void readConf(Config *conf) { return; }
void writeConf(Config *conf) { return; }

#endif
