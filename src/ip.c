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

#ifndef SHORK_DISKETTE

#include <fcntl.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>



/**
 * Gets the external IP address via STUN.
 * @return String containing this computer's external IP address
 */
char *getExternalIP(void)
{
    // Binding request: type 1, length 0, magic cookie, transaction ID
    unsigned char req[20] = {
        0x00, 0x01, 0x00, 0x00, 0x21, 0x12, 0xA4, 0x42
    };

    // Fill the transaction ID with random bytes
    int randFD = open("/dev/urandom", O_RDONLY);
    if (randFD < 0 || read(randFD, req + 8, 12) != 12)
        for (int i = 8; i < 20; i++)
            req[i] = (unsigned char)rand();
    if (randFD >= 0)
        close(randFD);

    for (int i = 0; i < STUN_SRVS_LEN; i++)
    {
        // Resolve server's hostname to an IPv4 UDP
        struct addrinfo hints = {
            .ai_family = AF_INET, .ai_socktype = SOCK_DGRAM
        };
        struct addrinfo *res = NULL;
        if (getaddrinfo(STUN_SRVS[i][0], STUN_SRVS[i][1], &hints, &res) !=
            0)
            continue;

        // Create a UDP socket
        int sockFD = socket(res->ai_family, res->ai_socktype,
            res->ai_protocol);
        struct pollfd pollFD = { sockFD, POLLIN, 0 };
        unsigned char buffer[512];
        ssize_t len = -1;

        // Send the request and wait up to REQ_TIMEOUT for a reply
        if (sockFD >= 0 &&
            sendto(sockFD, req, sizeof(req), 0, res->ai_addr,
                   res->ai_addrlen) == (ssize_t)sizeof(req) &&
            poll(&pollFD, 1, REQ_TIMEOUT) > 0)
            len = recv(sockFD, buffer, sizeof(buffer), 0);

        if (sockFD >= 0)
            close(sockFD);
        freeaddrinfo(res);

        // Valid reply: STUN header present, binding response (0x0101),
        // and magic cookie + transaction ID match our req
        if (len < 20 || buffer[0] != 0x01 || buffer[1] != 0x01 ||
            memcmp(buffer + 4, req + 4, 16) != 0)
            continue;

        // Bytes 2-3 give the attribute length after the 20-byte header
        size_t end = 20 + ((buffer[2] << 8) | buffer[3]);
        if (end > (size_t)len)
            end = len;

        // Attributes: 2-byte type, 2-byte length, value padded to 4 bytes
        for (size_t off = 20; off + 4 <= end; )
        {
            unsigned type = (buffer[off] << 8) | buffer[off + 1];
            unsigned alen = (buffer[off + 2] << 8) | buffer[off + 3];
            const unsigned char *val = buffer + off + 4;

            // Runs past the packet: malformed
            if (off + 4 + alen > end)
                break;

            // 0x0020 = XOR-MAPPED-ADDRESS
            // 0x0001 = MAPPED-ADDRESS
            // val[1] == 0x01 means IPv4
            if ((type == 0x0020 || type == 0x0001) &&
                alen >= 8 && val[1] == 0x01)
            {
                // XOR-mapped IPs are XORed with the magic cookie
                const unsigned char *cookie = buffer + 4;
                char ip[16];
                snprintf(ip, sizeof(ip), "%u.%u.%u.%u",
                    val[4] ^ (type == 0x0020 ? cookie[0] : 0),
                    val[5] ^ (type == 0x0020 ? cookie[1] : 0),
                    val[6] ^ (type == 0x0020 ? cookie[2] : 0),
                    val[7] ^ (type == 0x0020 ? cookie[3] : 0));
                return strdup(ip);
            }

            // If we didn't find what we want, skip to the next attrib
            off += 4 + ((alen + 3) & ~3u);
        }
    }

    return NULL;
}

/**
 * Gets the local IP address.
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

#else

#include <stddef.h>



char *getExternalIP(void) { return NULL; }
char *getLocalIP(void) { return NULL; }

#endif
