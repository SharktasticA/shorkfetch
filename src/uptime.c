/*
    ######################################################
    ##            SHORK UTILITY - SHORKFETCH            ##
    ######################################################
    ## Functions and data relating to handle the        ##
    ## operating system's name                          ##
    ######################################################
    ## Licence: GNU GENERAL PUBLIC LICENSE Version 3    ##
    ######################################################
    ## Kali (links.sharktastica.co.uk)                  ##
    ######################################################
*/



#include "globals.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/utsname.h>



/**
 * @return String containing uptime or "unknown" if undetermined/error
 */
char *getUptime(void)
{
    char *uptime = malloc(128);
    if (!uptime) return strdup("unknown");
    uptime[0] = '\0'; 

    FILE *fStream = fopen("/proc/uptime", "r");
    if (fStream)
    {
        double seconds;
        if (fscanf(fStream, "%lf", &seconds) == 1)
        {
            int sec = (int)seconds;
            int days = sec / 86400;
            int hours = (sec % 86400) / 3600;
            int minutes = (sec % 3600) / 60;

            if (!COMPACT)
            {
                if (days > 0)
                    snprintf(uptime, 128, "%dd, %dh, %dm", days, hours, minutes);
                else if (hours > 0)
                    snprintf(uptime, 128, "%dh, %dm", hours, minutes);
                else
                    snprintf(uptime, 128, "%dm", minutes);
            }
            else
            {
                if (days > 0)
                    snprintf(uptime, 128, "%dd:%dh:%dm", days, hours, minutes);
                else if (hours > 0)
                    snprintf(uptime, 128, "%dh:%dm", hours, minutes);
                else
                    snprintf(uptime, 128, "%dm", minutes);
            }
        }
        fclose(fStream);
    }
    else strcpy(uptime, "unknown");

    return uptime;
}
