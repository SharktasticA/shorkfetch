/*
    ######################################################
    ##            SHORK UTILITY - SHORKFETCH            ##
    ######################################################
    ## Functions and data relating to handling IP       ##
    ## addresses                                        ##
    ######################################################
    ## Licence: GNU GENERAL PUBLIC LICENSE Version 3    ##
    ######################################################
    ## Kali (links.sharktastica.co.uk)                  ##
    ######################################################
*/



#include "ip.h"

#include <net/if.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <string.h>



/**
 * @return String containing this computer's local IP address
 */
char *getLocalIP(void)
{
    struct ifaddrs *ifs;
    struct ifaddrs *currIF;
    char *result = NULL;

    // Attempt retrieivng network interfaces
    if (getifaddrs(&ifs) == -1) return NULL;

    // Iterate through found interfaces to find any IPv4s to use
    for (currIF = ifs; currIF != NULL; currIF = currIF->ifa_next) 
    {
        // Skip if null or not IPv4
        if (!currIF->ifa_addr || currIF->ifa_addr->sa_family != AF_INET)
            continue;

        // Skip if down, not running, or loopback device
        if (!(currIF->ifa_flags & IFF_UP) ||
            !(currIF->ifa_flags & IFF_RUNNING) ||
            (currIF->ifa_flags & IFF_LOOPBACK))
            continue;

        // Make IP human readible and in our result string
        char host[INET_ADDRSTRLEN];
        void *ifPtr = &((struct sockaddr_in *)currIF->ifa_addr)->sin_addr;
        if (inet_ntop(AF_INET, ifPtr, host, INET_ADDRSTRLEN)) 
        {
            result = strdup(host);
            break;
        }
    }

    freeifaddrs(ifs);
    return result;
}
