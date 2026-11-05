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
#include <unistd.h>



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

            if (strcmp(key, "ver") == 0)
                snprintf(conf->ver, CONF_VER_LEN, "%s", value);
            else if (strcmp(key, "charBullet") == 0)
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

#endif

/**
 * Validates the given CONFIG.fields string, breaking it up and putting it
 * into the fieldsOut array.
 * @param fields CONFIG.fields value
 * @param fieldsOut Array to put the validated fields in (must be
 *                  char[MAX_FIELDS][5])
 * @param noEsc CONFIG.noEsc value
 * @return Number of fields in fieldsOut; -1 if error
 */
int validateFields(const char *fields, char fieldsOut[][5], int noEsc)
{
    char *fieldsCopy = strdup(fields);
    if (!fieldsCopy)
    {
        printf("ERROR: could not allocate memory to process fields\n");
        return -1;
    }
    int noFields = 0;

    if (fieldsCopy && fieldsCopy[0] != '\0')
    {
        char *currTok = strtok(fieldsCopy, ",");
        while (currTok)
        {
            // Make sure current field is a known one
            int valid = 0;
            for (int i = 0; i < POSSIBLE_FIELDS_LEN; i++)
            {
                if (strcmp(currTok, POSSIBLE_FIELDS[i].id) == 0)
                {
                    valid = 1;
                    break;
                }
            }

            if (valid)
            {
                if (noFields >= MAX_FIELDS)
                {
                    free(fieldsCopy);
                    printf("ERROR: too many fields given (max %d)\n",
                        MAX_FIELDS);
                    return -1;
                }

                // Queue this field up
                strncpy(fieldsOut[noFields], currTok, 4);
                fieldsOut[noFields][4] = '\0';
                noFields++;
            }
            else
            {
                free(fieldsCopy);
                printf("ERROR: unrecognised field name \"%s\"\n", currTok);
                return -1;
            }

            currTok = strtok(NULL, ",");
        }

        // If no-escape-codes mode, we need to strip out colour palette
        // fields since they are not supported. For good measure, we also
        // remove any blank space padding above them.
        if (noEsc)
        {
            int write = 0;
            for (int read = 0; read < noFields; read++)
            {
                // Skip clrs, clba or clbr
                if (strcmp(fieldsOut[read], "clrs") == 0 ||
                    strcmp(fieldsOut[read], "clba") == 0 ||
                    strcmp(fieldsOut[read], "clbr") == 0)
                    continue;
                // Skip space if previous ele was clrs
                if (strcmp(fieldsOut[read], " ") == 0 && read > 0 &&
                    (strcmp(fieldsOut[read - 1], "clrs") == 0 ||
                    strcmp(fieldsOut[read - 1], "clba") == 0 ||
                    strcmp(fieldsOut[read - 1], "clbr") == 0))
                    continue;
                // Keep ele
                if (write != read)
                    strcpy(fieldsOut[write], fieldsOut[read]);
                write++;
            }
            noFields = write;
        }
    }

    free(fieldsCopy);
    return noFields;
}

#ifndef SHORK_DISKETTE

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
        fprintf(stream, "version=%s\n", VERSION);
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
    sync();
}

#else

int deleteConf(void) { return 1; }
void launchConf(Config *conf) { return; }
void readConf(Config *conf) { return; }
void writeConf(Config *conf) { return; }

#endif
