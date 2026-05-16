/*
    ######################################################
    ##            SHORK UTILITY - SHORKFETCH            ##
    ######################################################
    ## A lightweight Linux tool for displaying basic    ##
    ## system & environment information in a summarised ##
    ## format, similar to fastfetch, neofetch, etc.     ##
    ######################################################
    ## Licence: GNU GENERAL PUBLIC LICENSE Version 3    ##
    ######################################################
    ## Kali (links.sharktastica.co.uk)                  ##
    ######################################################
*/



#include "exclusions.h"
#include "igpus.h"
#include "implementers.h"
#include "replacements.h"
#include "wms.h"

#include <dirent.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <libgen.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/statvfs.h>
#include <sys/utsname.h>
#include <unistd.h>



typedef struct {
    // Connector name (e.g., DP-1)
    char *connector;
    // Flags if this is the primary screen
    int isPrimary;
    // Physical width (mm)
    float physX;
    // Physical height (mm)
    float physY;
    // Resolution width (px)
    int resX;
    // Resolution height (px)
    int resY;
    // Refresh rate (Hz)
    int refresh;
} Screen;

typedef struct {
    char *name;
    int vendor;
    int device;
    int revision;
} GPU;

typedef struct {
    long memTotal;
    long memFree;
    long buffers;
    long cached;
    long swapTotal;
    long swapFree;
} MemInfo;

typedef struct {
    int pid;
    char name[256];
} Process;



#define COL_BLACK           "0;30"
#define COL_BLUE            "0;34"
#define COL_BOLD_BLUE       "1;34"
#define COL_BOLD_CYAN       "1;36"
#define COL_BOLD_GREEN      "1;32"
#define COL_BOLD_MAGENTA    "1;35"
#define COL_BOLD_RED        "1;31"
#define COL_BOLD_WHITE      "1;37"
#define COL_BOLD_YELLOW     "1;33"
#define COL_CYAN            "0;36"
#define COL_GREEN           "0;32"
#define COL_GREY            "1;30"
#define COL_MAGENTA         "0;35"
#define COL_RED             "0;31"
#define COL_RESET           "0"
#define COL_WHITE           "0;37"
#define COL_YELLOW          "0;33"

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define TASK_COMM_LEN       24



static int COMPACT = 0;
static char* HOME;
static const char SHORK[24][20] = {
    "                   ",
    "^`.                ",
    "\\  \\               ",
    "/   `~~-__         ",
    "          `~~-_    ",
    "        \\\\\\  o `.  ",
    "  ,    ,    __,,,) ",
    "~;   ,-~~--`       ",
    "'._.'              ",
    "                   ",
    "                   ",
    "                   ",
    "                   ",
    "                   ",
    "                   ",
    "                   ",
    "                   ",
    "                   ",
    "                   ",
    "                   ",
    "                   ",
    "                   ",
    "                   ",
    "                   "
};
static struct winsize TERM_SIZE;
static int WAYLAND_PRESENT;
static int X11_PRESENT;
static char *XDG_CURRENT_DESKTOP;



/**
 * Converts a data value into a string formatted into a unit that makes sense for
 * its magnitude with its new unit added to the end.
 * @param from Unit the input value is in (e.g., "B", "KiB")
 * @param val Input value to convert
 * @return String containing the converted value and its new unit (e.g., "1.5MiB")
 */
char *bytesToReadable(const char *from, const long long val)
{
    long long bytes = val;
    if (strcmp(from, "KiB") == 0)
        bytes *= 1024;
    else if (strcmp(from, "MiB") == 0)
        bytes *= 1024LL * 1024;
    else if (strcmp(from, "GiB") == 0)
        bytes *= 1024LL * 1024 * 1024;
    else if (strcmp(from, "TiB") == 0)
        bytes *= 1024LL * 1024 * 1024 * 1024;

    const long long TiB = 1024LL * 1024 * 1024 * 1024;
    const long long GiB = 1024LL * 1024 * 1024;
    const long long MiB = 1024LL * 1024;
    const long long KiB = 1024LL;

    const int resultSize = 32;
    char *result = malloc(resultSize);
    if (!result) return strdup("");
    long long whole, remainder;
    int decimal;

    if (bytes >= TiB)
    {
        whole = bytes / TiB;
        remainder = bytes % TiB;

        if (COMPACT)
        {
            if (remainder > 0) whole++;
            snprintf(result, resultSize, "%lldT", whole);
            return result;
        }

        decimal = (int)((remainder * 10 + TiB / 2) / TiB);
        if (decimal == 10) { whole++; decimal = 0; }
        if (decimal == 0) snprintf(result, resultSize, "%lldTiB", whole);
        else snprintf(result, resultSize, "%lld.%dTiB", whole, decimal);
    }
    else if (bytes >= GiB)
    {
        whole = bytes / GiB;
        remainder = bytes % GiB;
        
        if (COMPACT)
        {
            if (remainder > 0) whole++;
            snprintf(result, resultSize, "%lldG", whole);
            return result;
        }

        decimal = (int)((remainder * 10 + GiB / 2) / GiB);
        if (decimal == 10) { whole++; decimal = 0; }
        if (decimal == 0) snprintf(result, resultSize, "%lldGiB", whole);
        else snprintf(result, resultSize, "%lld.%dGiB", whole, decimal);
    }
    else if (bytes >= MiB)
    {
        whole = bytes / MiB;
        remainder = bytes % MiB;
        
        if (COMPACT)
        {
            if (remainder > 0) whole++;
            snprintf(result, resultSize, "%lldM", whole);
            return result;
        }

        decimal = (int)((remainder * 10 + MiB / 2) / MiB);
        if (decimal == 10) { whole++; decimal = 0; }
        if (decimal == 0) snprintf(result, resultSize, "%lldMiB", whole);
        else snprintf(result, resultSize, "%lld.%dMiB", whole, decimal);
    }
    else if (bytes >= KiB)
    {
        whole = bytes / KiB;
        remainder = bytes % KiB;
        
        if (COMPACT)
        {
            if (remainder > 0) whole++;
            snprintf(result, resultSize, "%lldK", whole);
            return result;
        }

        decimal = (int)((remainder * 10 + KiB / 2) / KiB);
        if (decimal == 10) { whole++; decimal = 0; }
        if (decimal == 0) snprintf(result, resultSize, "%lldKiB", whole);
        else snprintf(result, resultSize, "%lld.%dKiB", whole, decimal);
    }
    else
        snprintf(result, resultSize, "%lldB", bytes);

    return result;
}

/**
 * Extracts a substring from an input string after a given separation character
 * and offset. Also removes any surrounding quotes or trailing newline characters
 * present. 
 * @param input Input string
 * @param point Character to find to separate from (e.g., '=' or ':')
 * @param offset How many characters after the point to separate at
 * @param inputSize Size to use when allocating the result string
 * @return String containing what's left after separation and cleaning
 */
char *extractFromPoint(char *input, size_t inputSize, char point, int offset)
{
    if (!input || inputSize < 2) return strdup("");

    // Prepare result string
    char *result = malloc(inputSize);
    if (!result) return strdup("");
    result[0] = '\0';

    // Find our separation point in the input string
    char *sep = strchr(input, point);
    if (!sep) return result;

    // Our start position taking into account possible offset
    char *start = sep + offset;

    // Trim potential leading double quote
    if (*start == '"') start++;

    // Copy everything after the start position into our result
    strncpy(result, start, inputSize - 1);
    result[inputSize - 1] = '\0';
    size_t len = strlen(result);

    // Trim potential trailing newline 
    if (len > 0 && result[len - 1] == '\n')
        result[--len] = '\0';

    // Trim potential trailing double quote
    if (len > 0 && result[len - 1] == '"')
        result[len - 1] = '\0';

    return result;
}

/**
 * Finds and erases a desired substring from an input string.
 * @param input Input string
 * @param inputSize Size to use when allocating the result string
 * @param needle Substring to find and erase
 * @return String containing what's left after erasing
 */
char *findErase(const char *input, const size_t inputSize, const char *needle)
{
    if (!input || !needle || inputSize < 2) return strdup("");

    size_t needleLen = strlen(needle);
    if (needleLen == 0) return strdup("");

    // Prepare result string
    char *result = malloc(inputSize);
    if (!result) return strdup("");

    // Copy input string to result
    strncpy(result, input, inputSize);
    result[inputSize - 1] = '\0';

    // Go through the string looking for our needle(s)... When found, we move the rest
    // of the string over and on top of said needles
    char *pos = result;
    while ((pos = strstr(pos, needle)) != NULL)
    {
        size_t tailLen = strlen(pos + needleLen);
        memmove(pos, pos + needleLen, tailLen + 1);
    }

    return result;
}

/**
 * Finds and replaces a given search term with a desired replacement term from an
 * input string.
 * @param input Input string
 * @param inputSize Size to use when allocating the result string
 * @param needle Substring to find and replace
 * @param replacement New string to insert
 * @return String after term replacement
 */
char *findReplace(const char *input, const size_t inputSize, const char *needle, const char *replacement)
{
    if (!input || !needle || !replacement || inputSize < 2) return strdup("");

    size_t needleLen = strlen(needle);
    size_t replacementLen = strlen(replacement);
    if (needleLen == 0) return strdup("");

    // Prepare result string
    char *result = malloc(inputSize);
    if (!result) return strdup("");

    // Copy input string to result
    strncpy(result, input, inputSize);
    result[inputSize - 1] = '\0';

    char *pos = result;
    while ((pos = strstr(pos, needle)) != NULL)
    {
        size_t tailLen = strlen(pos + needleLen);

        // If replacement is larger than our needle, realloc memory to avoid overflowing
        if (replacementLen > needleLen)
        {
            size_t currentLen = strlen(result);
            size_t newLen = currentLen + (replacementLen - needleLen) + 1;
            char *tmp = realloc(result, newLen);
            if (!tmp) break;
            pos = tmp + (pos - result);
            result = tmp;
        }

        // Move the trailing text to accomodate the new size and paste our replacement into
        // the 'gap'
        memmove(pos + replacementLen, pos + needleLen, tailLen + 1);
        memcpy(pos, replacement, replacementLen);
        pos += replacementLen;
    }

    return result;
}

/**
 * Calculates the square root of a given number (and helps us avoid including
 * math.h) - float variant.
 * @param x Input value
 * @returns Square root of the input value; -1 if imaginary/invalid
 */
float fsqrt(float x)
{
    // Return -1 to flag imaginary result
    if (x < 0.0) return -1;
    // sqrt(0 or 1) = number itself anyway
    if (x == 0.0 || x == 1.0) return x;

    float result = x;
    float last = 0.0;

    // Newton–Raphson...
    for (int i = 0; i < 20; i++)
    {
        last = result;
        result = 0.5 * (result + x / result);

        // Get out once we're stable
        if (result == last) break;
    }

    return result;
}

/**
 * Validates if the WITH_COL value supplied matches a known colour. If not, bright cyan is used as a fallback.
 * @return ANSI escape code for colour; empty string if COL=OFF
 */
char *getAccentColour(void)
{
#ifdef COL
    char *colour = STR(COL);
    if (strcmp(colour, "BLACK") == 0) return "\033[" COL_BLACK "m";
    if (strcmp(colour, "BLUE") == 0) return "\033[" COL_BLUE "m";
    if (strcmp(colour, "BOLD_BLUE") == 0) return "\033[" COL_BOLD_BLUE "m";
    if (strcmp(colour, "BOLD_CYAN") == 0) return "\033[" COL_BOLD_CYAN "m";
    if (strcmp(colour, "BOLD_GREEN") == 0) return "\033[" COL_BOLD_GREEN "m";
    if (strcmp(colour, "BOLD_MAGENTA") == 0) return "\033[" COL_BOLD_MAGENTA "m";
    if (strcmp(colour, "BOLD_RED") == 0) return "\033[" COL_BOLD_RED "m";
    if (strcmp(colour, "BOLD_WHITE") == 0) return "\033[" COL_BOLD_WHITE "m";
    if (strcmp(colour, "BOLD_YELLOW") == 0) return "\033[" COL_BOLD_YELLOW "m";
    if (strcmp(colour, "CYAN") == 0) return "\033[" COL_CYAN "m";
    if (strcmp(colour, "GREEN") == 0) return "\033[" COL_GREEN "m";
    if (strcmp(colour, "GREY") == 0) return "\033[" COL_GREY "m";
    if (strcmp(colour, "MAGENTA") == 0) return "\033[" COL_MAGENTA "m";
    if (strcmp(colour, "OFF") == 0) return "";
    if (strcmp(colour, "RED") == 0) return "\033[" COL_RED "m";
    if (strcmp(colour, "WHITE") == 0) return "\033[" COL_WHITE "m";
    if (strcmp(colour, "YELLOW") == 0) return "\033[" COL_YELLOW "m";
#endif
    return "\033[" COL_BOLD_CYAN "m";
}

/**
 * Gets the parent process ID (PPID) and name of a given process ID (PID).
 * @param pid The input PID
 * @return Process struct with the found PPID and name; pid is -1 if something went wrong
 */
Process getParentProcess(int pid)
{
    Process result = { -1, "" };

    // Open the process's status file
    char pidPath[PATH_MAX];
    snprintf(pidPath, sizeof(pidPath), "/proc/%d/status", pid);
    FILE *pidStatus = fopen(pidPath, "r");
    if (!pidStatus) return result;

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pidStatus))
    {
        // Look for the PPid field in the status file
        if (sscanf(buffer, "PPid: %d", &result.pid) == 1)
        {
            fclose(pidStatus);

            // Get parent process' name
            char commPath[PATH_MAX];
            snprintf(commPath, sizeof(commPath), "/proc/%d/comm", result.pid);
            FILE *comm = fopen(commPath, "r");
            if (!comm) { result.pid = -1; return result; }

            fgets(result.name, sizeof(result.name), comm);
            result.name[strcspn(result.name, "\n")] = '\0';
            fclose(comm);

            return result;
        }
    }

    fclose(pidStatus);
    return result;
}

/**
 * @param prog Program's executable name
 * @param isExec Flags if the function should also check if a found program has
 *               execute permissions
 * @returns 1 if program is installed; 0 if not
 */
int isProgramInstalled(char *prog, int isExec)
{
    int mode = isExec ? X_OK : F_OK;

    char *path = getenv("PATH");
    if (!path)
    {
        char cmd[64];
        snprintf(cmd, 64, "%s --version > /dev/null 2>&1", prog);
        return (system(cmd) == 0);
    }

    char *paths = strdup(path);
    char *dir = strtok(paths, ":");
    while (dir)
    {
        char fullPath[PATH_MAX];
        snprintf(fullPath, sizeof(fullPath), "%s/%s", dir, prog);
        if (access(fullPath, mode) == 0)
        {
            free(paths);
            return 1;
        }
        dir = strtok(NULL, ":");
    }
    free(paths);

    // Also try /usr/libexec
    char libexecPath[PATH_MAX];
    snprintf(libexecPath, PATH_MAX, "/usr/libexec/%s", prog);
    if (access(libexecPath, mode) == 0) return 1;

    return 0;
}

/**
 * Calculates the square root of a given number (and helps us avoid including
 * math.h) - integer variant.
 * @param x Input value
 * @returns Square root of the input value; -1 if imaginary/invalid
 */
int isqrt(int x)
{
    // Return -1 to flag imaginary result
    if (x < 0) return -1;
    // sqrt(0 or 1) = number itself anyway
    if (x < 2) return x;

    long low = 1;
    long high = x / 2;
    int result = 0;

    // Let's do a binary search
    while (low <= high)
    {
        long mid = low + (high - low) / 2;
        long square = mid * mid;

        // If perfect square found, get out now
        if (square == x) return (int)mid;
        else if (square < x)
        {
            result = (int)mid;
            low = mid + 1;
        }
        // If square is too large, go lower
        else high = mid - 1;
    }

    return result;
}

/**
 * Checks if a given process name is presently running and via the /proc 
 * filesystem.
 * @param name The process name to find
 * @param strict Flags if we are looking for an exact match (1) or not (0)
 * @return 1 if found; 0 if not found or error
 */
int procExists(const char *name, const int strict)
{
    DIR *proc = opendir("/proc");
    if (!proc) return 0;

    struct dirent *entry;
    while ((entry = readdir(proc)) != NULL)
    {
        // Skip non-numeric (not PID) entries
        if (entry->d_name[0] < '0' || entry->d_name[0] > '9')
            continue;

        // Build path to process' comm (command) file
        char path[PATH_MAX];
        snprintf(path, sizeof(path), "/proc/%s/comm", entry->d_name);

        FILE *commFile = fopen(path, "r");
        if (!commFile) continue;

        char commVal[TASK_COMM_LEN];
        int found = 0;

        if (fgets(commVal, TASK_COMM_LEN, commFile))
        {
            // Strip trailing newline
            commVal[strcspn(commVal, "\n")] = '\0';

            // If strict, we look for an exact match
            if (strict)
                found = strcmp(commVal, name) == 0;
            // If not, we look for a substring
            else
                found = strstr(commVal, name) != NULL;
        }

        fclose(commFile);

        if (found)
        {
            closedir(proc);
            return 1;
        }
    }

    closedir(proc);
    return 0;
}

/**
 * Reads a single hexadecimal number for a given text file.
 * @param path Path to file to open
 * @return The read number as an integer (0 as fallback)
 */
int readHexFile(const char *path)
{
    FILE *fStream = fopen(path, "r");
    if (!fStream) return 0;
    int val;
    if (fscanf(fStream, "%x", &val) != 1) val = 0;
    fclose(fStream);
    return val;
}



/**
 * Cleans a CPU's name so it is less needlessly verbose and 'to the point'.
 * @param input Input string
 * @param inputSize Size to use when allocating the result string
 * @param coreCount Number of cores the processor has (presently only used for CPU name manipulation)
 * @return String containing the result after cleaning
 */
char *cleanCPUName(const char *input, size_t inputSize, int coreCount)
{
    if (!input || inputSize < 2) return strdup("");

    // Prepare result string
    char *result = malloc(inputSize);
    if (!result) return strdup("");
    
    // Copy input string to result
    strncpy(result, input, inputSize - 1);
    result[inputSize - 1] = '\0';
    size_t strLen  = strlen(result);

    // Remove double-spaces
    char *src = result, *dst = result;
    while (*src)
    {
        *dst++ = *src;
        // If we hit a space, let's begin skipping them...
        if (*src++ == ' ')
            while (*src == ' ')
                src++;
    }
    *dst = '\0';

    // Shorten " / " to "/"
    if (strstr(result, " / "))
    {
        char *tmp = findReplace(result, inputSize, " / ", "/");
        strncpy(result, tmp, inputSize - 1);
        result[inputSize - 1] = '\0';
        free(tmp);
    }

    // Apply generic deletions
    for (size_t i = 0; i < DELETIONS_LEN; i++)
    {
        const char *pattern = DELETIONS[i];
        char *tmp = findErase(result, inputSize, pattern);
        strncpy(result, tmp, inputSize - 1);
        result[inputSize - 1] = '\0';
        free(tmp);
    }

    // Remove clock speed from CPU name
    if (strstr(result, "@") && strstr(result, "Hz"))
    {
        char *at = strstr(result, " @");
        if (at)
        {
            char *leftBrac = strchr(at, '(');
            // If brackets for core/thread count present, selective removal
            if (leftBrac && leftBrac > at)
                memmove(at, leftBrac - 1, strlen(leftBrac - 1) + 1);
            // If no brackets for core/thread count, just nuke @ and after
            else
                *at = '\0';
        }
    }

    // Shorten "Advanced Micro Devices" to "AMD"
    if (strstr(result, "Advanced Micro Devices"))
    {
        char *tmp = findReplace(result, inputSize, "Advanced Micro Devices", "AMD");
        strncpy(result, tmp, inputSize - 1);
        result[inputSize - 1] = '\0';
        free(tmp);
    }

    // Apply AMD-specific replacements
    if (strstr(result, "AMD"))
    {
        int replaces = 0;
        for (int i = 0; i < AMD_REPLACES_LEN; i++)
        {
            if (AMD_REPLACES[i].standalone && replaces > 0) continue;
            else if (strstr(result, AMD_REPLACES[i].match))
            {
                char *tmp = findReplace(result, inputSize, AMD_REPLACES[i].match, AMD_REPLACES[i].replacement);
                strncpy(result, tmp, inputSize - 1);
                result[inputSize - 1] = '\0';
                free(tmp);
                replaces++;
            }
        }

        if (strstr(result, "Ryzen") || strstr(result, "EPYC"))
        {
            // Dynamically generate substrings like "16-Core" or "16 Cores" to find
            // and remove from AMD Ryzen or EPYC CPU names
            int done = 0;
            for (int i = 2; i <= 192; i += 2)
            {
                if (done == 1) break;

                char *withDash = malloc(10);
                char *withSpace = malloc(11);
                if (!withDash || !withSpace) 
                {
                    free(withDash);
                    free(withSpace);
                    continue;
                }
                
                snprintf(withDash, 10, " %d%s", i, "-Core");
                snprintf(withSpace, 11, " %d%s", i, " Cores");

                if (strstr(result, withDash))
                {
                    done = 1;
                    char *tmp = findErase(result, inputSize, withDash);
                    strncpy(result, tmp, inputSize - 1);
                    result[inputSize - 1] = '\0';
                    free(tmp);
                }
                else if (strstr(result, withSpace))
                {
                    done = 1;
                    char *tmp = findErase(result, inputSize, withSpace);
                    strncpy(result, tmp, inputSize - 1);
                    result[inputSize - 1] = '\0';
                    free(tmp);
                }

                free(withDash);
                free(withSpace);
            }
        }
        else if (strstr(result, "AMD [AMD/ATI]"))
        {
            char *tmp = findReplace(result, inputSize, "AMD [AMD/ATI]", "AMD/ATI");
            strncpy(result, tmp, inputSize - 1);
            result[inputSize - 1] = '\0';
            free(tmp);
        }
    }
    // Apply Intel-specific replacements
    else if (strstr(result, "Intel"))
    {
        // Catch Yonah Core CPUs that don't have "Core" in their name
        if (strstr(result, "Intel T") && coreCount > 0)
        {
            char *tmp = NULL;
            if (coreCount == 1)
                tmp = findReplace(result, inputSize, "Intel T", "Intel Core Solo T");
            else if (coreCount == 2)
                tmp = findReplace(result, inputSize, "Intel T", "Intel Core Duo T");

            if (tmp)
            {
                strncpy(result, tmp, inputSize - 1);
                result[inputSize - 1] = '\0';
                free(tmp);
            }
        }

        // Catch redundant "x Gen" in the name of late Intel Core CPUs
        if (strstr(result, " Core ") && strstr(result, " Gen "))
        {
            char *needle = strstr(result, " Gen ");
            if (needle)
            {
                if (needle - result >= 2)
                {
                    char *suffix = needle - 2;
                    // Proceed if we find a "st", "nd", "rd" or "th" suffix
                    if ((suffix[0] == 's' && suffix[1] == 't') || (suffix[0] == 'n' && suffix[1] == 'd') || (suffix[0] == 'r' && suffix[1] == 'd') || (suffix[0] == 't' && suffix[1] == 'h'))
                    {
                        // Walk back to find the ordinal
                        char *digits = suffix - 1;
                        while (digits >= result && *digits >= '0' && *digits <= '9')
                            digits--;
                        char *ordinal = digits + 1;

                        // Make the deletion
                        memmove(ordinal, needle + 5, strlen(needle + 5) + 1);
                    }
                }
            }
        }

        int replaces = 0;
        for (int i = 0; i < INTEL_REPLACES_LEN; i++)
        {
            if (INTEL_REPLACES[i].standalone && replaces > 0) continue;
            else if (strstr(result, INTEL_REPLACES[i].match))
            {
                char *tmp = findReplace(result, inputSize, INTEL_REPLACES[i].match, INTEL_REPLACES[i].replacement);
                strncpy(result, tmp, inputSize - 1);
                result[inputSize - 1] = '\0';
                free(tmp);
            }
        }
    }
    // Apply IDT/Centaur-specific replacements
    else if (strstr(result, "IDT"))
    {
        int replaces = 0;
        for (int i = 0; i < IDT_REPLACES_LEN; i++)
        {
            if (IDT_REPLACES[i].standalone && replaces > 0) continue;
            else if (strstr(result, IDT_REPLACES[i].match))
            {
                char *tmp = findReplace(result, inputSize, IDT_REPLACES[i].match, IDT_REPLACES[i].replacement);
                strncpy(result, tmp, inputSize - 1);
                result[inputSize - 1] = '\0';
                free(tmp);
            }
        }
    }
    // Apply VIA-specific replacements
    else if (strstr(result, "VIA"))
    {
        int replaces = 0;
        for (int i = 0; i < VIA_REPLACES_LEN; i++)
        {
            if (VIA_REPLACES[i].standalone && replaces > 0) continue;
            else if (strstr(result, VIA_REPLACES[i].match))
            {
                char *tmp = findReplace(result, inputSize, VIA_REPLACES[i].match, VIA_REPLACES[i].replacement);
                strncpy(result, tmp, inputSize - 1);
                result[inputSize - 1] = '\0';
                free(tmp);
            }
        }
    }

    // Compact mode specific cleaning
    if (COMPACT)
    {
        // Apply compact-specific CPU name shortenings
        int replaces = 0;
        for (int i = 0; i < COMPACT_CPU_REPLACES_LEN; i++)
        {
            if (COMPACT_CPU_REPLACES[i].standalone && replaces > 0) continue;
            else if (strstr(result, COMPACT_CPU_REPLACES[i].match))
            {
                char *tmp = findReplace(result, inputSize, COMPACT_CPU_REPLACES[i].match, COMPACT_CPU_REPLACES[i].replacement);
                strncpy(result, tmp, inputSize - 1);
                result[inputSize - 1] = '\0';
                free(tmp);
                replaces++;
            }
        }
    }

    return result;
}

/**
 * Cleans a GPU's name so it is less needlessly verbose and 'to the point'.
 * @param input Input string
 * @param inputSize Size to use when allocating the result string
 * @return String containing the result after cleaning
 */
char *cleanGPUName(const char *vendor, const char *device, const size_t inputSize)
{
    if (!vendor || !device || inputSize < 2) return strdup("");

    // Prepare result strings
    const size_t RESULT_SIZE = (inputSize * 2) + 1;
    char *result = malloc(RESULT_SIZE);
    if (!result) return strdup("");
    char *cleanedVendor = NULL;
    char *cleanedDevice = strdup(device);
    char *cleanedDeviceNorm = NULL;
    char *cleanedDeviceBrac = NULL;



    // Shorten " / " to "/"
    if (strstr(cleanedDevice, " / "))
    {
        char *tmp = findReplace(cleanedDevice, inputSize, " / ", "/");
        free(cleanedDevice);
        cleanedDevice = tmp;
    }

    // If we find an opening square bracket, break up device name into "normal"
    // and "bracket" strings
    const char *brac = strchr(cleanedDevice, '[');
    if (brac)
    {
        size_t normLen = (size_t)(brac - cleanedDevice);
        cleanedDeviceNorm = strndup(cleanedDevice, normLen);

        // Remove trailing space
        if (normLen > 0 && cleanedDeviceNorm[normLen - 1] == ' ')
            cleanedDeviceNorm[normLen - 1] = '\0';

        // Find closing bracket
        const char *end = strchr(brac + 1, ']');
        if (end)
        {
            size_t bracLen = (size_t)(end - (brac + 1));
            cleanedDeviceBrac = strndup(brac + 1, bracLen);
        }
        // If no closing bracket, we treat this as invalid...
        else cleanedDeviceBrac = NULL;
    }
    else
    {
        cleanedDeviceNorm = strdup(device);
        cleanedDeviceBrac = NULL;
    }



    // Vendor-specific actions...
    // Advanced Micro Devices, Inc. [AMD/ATI]
    if (vendor[0] == 'A' && strncmp(vendor, "Advanced Micro", 14) == 0)
    {
        // If we used amdgpu.ids, A deterimed "AMD" or "ATI" may be in the
        // device name and we should use that
        if (strncmp(cleanedDevice, "AMD ", 4) == 0)
        {
            cleanedVendor = strdup("AMD");
            memmove(cleanedDevice, cleanedDevice + 4, strlen(cleanedDevice) - 3);
        }
        else if (strncmp(cleanedDevice, "ATI ", 4) == 0)
        {
            cleanedVendor = strdup("ATI");
            memmove(cleanedDevice, cleanedDevice + 4, strlen(cleanedDevice) - 3);
        }
        else
            cleanedVendor = strdup("AMD/ATI");

        // If we have bracketed info, we *may* discard the norm (usually just
        // containing the codename)
        if (cleanedDeviceBrac)
        {
            // If the info contains the "Graphics" (e.g., "Radeon R6 Graphics")
            // or "Vega Series", we will permit the norm to help with
            // distinguishing it
            if (strstr(cleanedDeviceBrac, " Graphics") || strstr(cleanedDeviceBrac, " Vega Series"))
                snprintf(cleanedDevice, inputSize, "%s (%s)", cleanedDeviceBrac, cleanedDeviceNorm);
            // Otherwise, we just use the bracketed info
            else
                snprintf(cleanedDevice, inputSize, "%s", cleanedDeviceBrac);
        }

        // Begin testing for if there are multiple "Radeon"... Look for the
        // first instance of "Radeon"
        char *first = strstr(cleanedDevice, "Radeon");
        if (first)
        {
            // Handle " Radeon"; +6 so we don't trip on the first instance
            char *next = first + 6;
            while ((next = strstr(next, " Radeon")))
                memmove(next, next + 7, strlen(next + 7) + 1);

            // Handle "Radeon "
            next = first + 6;
            while ((next = strstr(next, "Radeon ")))
                memmove(next, next + 7, strlen(next + 7) + 1);
        }

        // Especially for Vega iGPU strings like "Radeon Vega Series /
        // Radeon Vega Mobile Series"
        if (strstr(cleanedDevice, " Series/Vega Mobile Series"))
        {
            char *tmp = findReplace(cleanedDevice, inputSize, " Series/Vega Mobile Series", "/Vega Mobile");
            free(cleanedDevice);
            cleanedDevice = tmp;
        }

        // Prettify (e.g.) "FirePro V (FireGL V)" to "FirePro V/FireGL V"
        char *fireGLBrac = strstr(cleanedDevice, " (Fire");
        if (fireGLBrac)
        {
            char *closeBrac = strchr(fireGLBrac, ')');
            if (closeBrac)
            {
                memmove(fireGLBrac, fireGLBrac + 1, strlen(fireGLBrac));
                cleanedDevice[fireGLBrac - cleanedDevice] = '/';
                char *newClose = strchr(cleanedDevice, ')');
                if (newClose)
                    memmove(newClose, newClose + 1, strlen(newClose));
            }
        }
    }
    // Intel Corporation
    else if (vendor[0] == 'I' && strncmp(vendor, "Intel", 5) == 0)
    {
        cleanedVendor = strdup("Intel");

        // If we have bracketed info, we discard the norm (usually just
        // containing the core name)
        if (cleanedDeviceBrac)
            snprintf(cleanedDevice, inputSize, "%s", cleanedDeviceBrac);

        // Some Arc GPUs have "Intel" in the device name...
        if (strncmp(cleanedDevice, "Intel ", 6) == 0)
            memmove(cleanedDevice, cleanedDevice + 6, strlen(cleanedDevice) - 5);
    }
    // NVIDIA Corporation
    else if (vendor[0] == 'N' && strncmp(vendor, "NVIDIA", 6) == 0)
    {
        cleanedVendor = strdup("NVIDIA");

        // If we have bracketed info, we discard the norm (usually just
        // containing the core name)
        if (cleanedDeviceBrac)
            snprintf(cleanedDevice, inputSize, "%s", cleanedDeviceBrac);
    }
    // 3Dfx Interactive, Inc.
    else if (vendor[0] == '3' && strncmp(vendor, "3Dfx", 4) == 0)
    {
        cleanedVendor = strdup("3Dfx");

        // If this Voodoo is probably a Velocity (entry-level)
        if (cleanedDeviceBrac && cleanedDeviceBrac[0] == 'V')
            snprintf(cleanedDevice, inputSize, "%s", cleanedDeviceBrac);

        // E.g., Voodoo 4/Voodoo 5 -> Voodoo 4/5
        if (strstr(cleanedDevice, "/Voodoo "))
        {
            char *tmp = findReplace(cleanedDevice, inputSize, "/Voodoo ", "/");
            free(cleanedDevice);
            cleanedDevice = tmp;
        }
    }
    else if (vendor[0] == 'C')
    {
        // Chips and Technologies
        if (vendor[1] == '&' || strncmp(vendor, "Chips and", 9) == 0)
        {
            if (COMPACT)
                cleanedVendor = strdup("C&T");
            else
                cleanedVendor = strdup("Chips & Technologies");
        }
        // Cirrus Logic
        else if (strncmp(vendor, "Cirrus Logic", 12) == 0)
        {
            cleanedVendor = strdup("Cirrus Logic");

            // Discard any bracketed info like "Alpine" 
            if (cleanedDeviceBrac)
                snprintf(cleanedDevice, inputSize, "%s", cleanedDeviceNorm);

            // Remove space between "GD" and model number
            if (cleanedDevice[0] == 'G' && cleanedDevice[1] == 'D' && cleanedDevice[2] == ' ')
            {
                char *tmp = findReplace(cleanedDevice, inputSize, "GD ", "GD");
                free(cleanedDevice);
                cleanedDevice = tmp;
            }
        }
    }
    // Matrox Electronics Systems Ltd.
    else if (vendor[0] == 'M' && strncmp(vendor, "Matrox", 6) == 0)
    {
        cleanedVendor = strdup("Matrox");

        // If we have bracketed info, we discard the norm (usually just
        // containing the chip model name like 2064)
        if (cleanedDeviceBrac)
            snprintf(cleanedDevice, inputSize, "%s", cleanedDeviceBrac);
    }
    // S3 Graphics Ltd.
    else if (vendor[0] == 'S' && strncmp(vendor, "S3 ", 3) == 0)
    {
        cleanedVendor = strdup("S3 Graphics");

        // If we have bracketed info, we discard the norm (usually just
        // containing the core name)
        if (cleanedDeviceBrac)
            snprintf(cleanedDevice, inputSize, "%s", cleanedDeviceBrac);
    }
    else if (vendor[0] == 'T')
    {
        // Trident Microsystems
        if (strncmp(vendor, "Trident", 7) == 0)
            cleanedVendor = strdup("Trident");
        // Tseng Labs Inc
        else if (strncmp(vendor, "Tseng", 5) == 0)
            cleanedVendor = strdup("Tseng Labs");
    }
    // VMware
    else if (vendor[0] == 'V' && strncmp(vendor, "VMware", 6) == 0)
        cleanedVendor = strdup("VMware");
    // Anything else...
    else
    {
        // Apply generic deletions
        cleanedVendor = strdup(vendor);
        for (size_t i = 0; i < DELETIONS_LEN; i++)
        {
            const char *pattern = DELETIONS[i];
            char *tmp = findErase(cleanedVendor, inputSize, pattern);
            free(cleanedVendor);
            cleanedVendor = tmp;
        }
    }

    // Universal deletions
    if (strstr(cleanedDevice, " Graphics Adapter"))
    {
        char *tmp = findReplace(cleanedDevice, inputSize, " Graphics Adapter", "");
        free(cleanedDevice);
        cleanedDevice = tmp;
    }



    // Combine and return final result
    snprintf(result, RESULT_SIZE, "%s %s", cleanedVendor, cleanedDevice);

    // Compact mode specific cleaning
    if (COMPACT)
    {
        // Apply compact-specific GPU name shortenings
        int replaces = 0;
        for (int i = 0; i < COMPACT_GPU_REPLACES_LEN; i++)
        {
            if (COMPACT_GPU_REPLACES[i].standalone && replaces > 0) continue;
            else if (strstr(result, COMPACT_GPU_REPLACES[i].match))
            {
                char *tmp = findReplace(result, RESULT_SIZE, COMPACT_GPU_REPLACES[i].match, COMPACT_GPU_REPLACES[i].replacement);
                strncpy(result, tmp, RESULT_SIZE - 1);
                result[RESULT_SIZE - 1] = '\0';
                free(tmp);
                replaces++;
            }
        }
    }

    free(cleanedVendor);
    free(cleanedDevice);
    free(cleanedDeviceNorm);
    free(cleanedDeviceBrac);

    return result;
}

/**
 * Adds new lines to a given string based on the requested line width.
 * @param input Input string
 * @param width Characters per line
 * @param indent Indent to include after newly inserted new line
 * @param trim Flags that any trailing newlines should be removed
 * @return Number of lines in the string
 */
int formatNewLines(char *input, int width, char *indent, int trim)
{
    if (!input || width < 1) return 0;

    // Initialse variables that help us track progress
    size_t inputStrLen = strlen(input);
    size_t indentLen = indent ? strlen(indent) : 0;
    int lines = 1;
    int lastSpace = -1;
    int widthCount = 1;

    // Iterate through the input string to find line breaks or places to add new ones
    for (int i = 0; i < inputStrLen; i++)
    {
        if (input[i] == '\033')
        {
            while (i < inputStrLen && input[i] != 'm') i++;
            if (i >= inputStrLen) break;
            continue; 
        }
        
        // Track where the last space was in case so we can go back for a future word wrap
        if (input[i] == ' ') lastSpace = i;
        // Reset tracking and take into account if we find an existing new line
        else if (input[i] == '\n')
        {
            lines++;
            widthCount = 0;
            continue;
        }

        // Begin word wrapping once the line width is saturated
        if (widthCount == width)
        {
            if (lastSpace != -1)
            {
                input[lastSpace] = '\n';
                lines++;

                if (indent && indentLen > 0)
                {
                    memmove(input + lastSpace + 1 + indentLen, input + lastSpace + 1, inputStrLen - lastSpace);
                    memcpy(input + lastSpace + 1, indent, indentLen);
                    inputStrLen += indentLen;
                    if (lastSpace <= i) i += indentLen;
                }
            }
            widthCount = i - lastSpace;
        }

        widthCount++;
    }

    // If desired, strip possible trailing new line
    if (trim)
    {
        int end = strlen(input) - 1;
        while (end >= 0 && input[end] == '\n')
        {
            input[end] = '\0';
            end--;
            lines--;
        }
    }

    return lines;
}

/**
 * Reads memory and swap data from /proc/meminfo into a MemInfo struct.
 * @return populated MemInfo struct
 */
MemInfo getMemInfo(void)
{
    MemInfo mi = {0};

    FILE *fStream = fopen("/proc/meminfo", "r");
    if (fStream)
    {
        char buffer[128];
        int parsed = 0;
        while (fgets(buffer, sizeof(buffer), fStream) && parsed < 6)
        {
            if (sscanf(buffer, "MemTotal: %ld", &mi.memTotal) == 1) { parsed++; continue; }
            else if (sscanf(buffer, "MemFree: %ld", &mi.memFree) == 1) { parsed++; continue; }
            else if (sscanf(buffer, "Buffers: %ld", &mi.buffers) == 1) { parsed++; continue; }
            else if (sscanf(buffer, "Cached: %ld", &mi.cached) == 1) { parsed++; continue; }
            else if (sscanf(buffer, "SwapTotal: %ld", &mi.swapTotal) == 1) { parsed++; continue; }
            else if (sscanf(buffer, "SwapFree: %ld", &mi.swapFree) == 1) { parsed++; continue; }
        }
        fclose(fStream);
    }

    return mi;
}

/**
 * @return winsize struct containing the current terminal size in columns and rows
 */
struct winsize getTerminalSize(void)
{
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
    {
        ws.ws_col = 80;
        ws.ws_row = 24;
    }
    return ws;
}

/**
 * @param gpu GPU struct containing detected vendor and device IDs and revision
 *            number
 * @return String containing the GPU's assembled and cleaned full name; vendor
 *         and device IDs as hex if interpreting failed
 */
char *interpretGPU(GPU *gpu)
{
    const int GPU_SIZE = 256;
    char *gpuStr = malloc(GPU_SIZE);
    if (!gpuStr) return strdup("unknown");
    gpuStr[0] = '\0';



    // If Intel GPU, query our pre-defined iGPU list
    if (gpu->vendor == 0x8086)
    {
        const char *name = INTEL_IGPUS[gpu->device];
        if (name)
        {
            char *tmp = cleanGPUName("Intel", name, GPU_SIZE);
            strncpy(gpuStr, tmp, GPU_SIZE - 1);
            gpuStr[GPU_SIZE - 1] = '\0';
            free(tmp);
            return gpuStr;
        }
    }
    // If AMD GPU, query the AMD GPU IDs database
    else if (gpu->vendor == 0x1002)
    {
        // Possible paths to amdgpu.ids 
        char userAMDGPUIDs[PATH_MAX];
        snprintf(userAMDGPUIDs, PATH_MAX, "%s/.local/share/libdrm/amdgpu.ids", HOME);
        const char *amdGPUIDs[] = {
            "/usr/share/libdrm/amdgpu.ids",
            userAMDGPUIDs
        };

        for (int i = 0; i < 2; i++)
        {
            FILE *fStream = fopen(amdGPUIDs[i], "r");
            if (!fStream) continue;

            char line[256];
            while (fgets(line, sizeof(line), fStream))
            {
                if (line[0] == '#' || line[0] == '\n') continue;

                int fileDID, fileRev;
                char name[256];
                // A line looks lile: 7480,	C1,	AMD Radeon RX 7700S
                if (sscanf(line, "%x,\t%x,\t%255[^\n]", &fileDID, &fileRev, name) == 3)
                {
                    if (fileDID == gpu->device && fileRev == gpu->revision)
                    {
                        char *tmp = cleanGPUName("Advanced Micro", name, GPU_SIZE);
                        strncpy(gpuStr, tmp, GPU_SIZE - 1);
                        gpuStr[GPU_SIZE - 1] = '\0';
                        free(tmp);
                        fclose(fStream);
                        return gpuStr;
                    }
                }
            }
            fclose(fStream);
        }
    }



    // Check the PCI IDs database
    char *pciids;
    if (access("/usr/share/misc/pci.ids", F_OK) == 0)
        pciids = "/usr/share/misc/pci.ids";
    else if (access("/usr/share/hwdata/pci.ids", F_OK) == 0)
        pciids = "/usr/share/hwdata/pci.ids";
    else
    {
        snprintf(gpuStr, GPU_SIZE, "%04x:%04x", gpu->vendor, gpu->device);
        return gpuStr;
    }

    FILE *fStream = fopen(pciids, "r");
    if (!fStream)
    {
        snprintf(gpuStr, GPU_SIZE, "%04x:%04x", gpu->vendor, gpu->device);
        return gpuStr;
    }

    char *vendor = NULL;
    char vendorHex[5];
    sprintf(vendorHex, "%04x", gpu->vendor);
    char *device = NULL;
    char deviceHex[5];
    sprintf(deviceHex, "%04x", gpu->device);

    char buffer[128];
    while (fgets(buffer, sizeof(buffer), fStream))
    {
        if (buffer[0] == '#' || buffer[0] == 'C' || buffer[0] == '\0') continue;
        
        if (strncmp(buffer, vendorHex, 4) == 0)
        {
            char *start = buffer + 6;
            size_t len = strlen(start);
            if (len > 0 && start[len - 1] == '\n') start[len - 1] = '\0';
            vendor = strdup(start);
        }

        if (vendor)
        {
            int tabs = 0;
            while (buffer[tabs] == '\t') tabs++;

            if (tabs > 0)
            {
                if (strncmp(buffer + tabs, deviceHex, 4) == 0)
                {
                    char *start = buffer + tabs + 6;
                    size_t len = strlen(start);
                    if (len > 0 && start[len - 1] == '\n') start[len - 1] = '\0';
                    device = strdup(start);
                    break;
                }
            }
        }
    }
    fclose(fStream);

    if (!vendor || !device)
        snprintf(gpuStr, GPU_SIZE, "%04x:%04x", gpu->vendor, gpu->device);
    else
        snprintf(gpuStr, GPU_SIZE, "%s", cleanGPUName(vendor, device, GPU_SIZE));

    if (vendor) free(vendor);
    if (device) free(device);

    return gpuStr;
}

/**
 * @param screen Screen struct containing raw specifications for the given
 *               screen
 * @return String containing the screen's assembled specifications
 */
char *interpretScreen(Screen *screen)
{
    // Quick check to make sure we have something to work with...
    if (screen->resX <= 0 || screen->resY <= 0)
        return NULL;

    const int SCREEN_SIZE = 128;
    char *screenStr = malloc(SCREEN_SIZE);

    // Prepare physical screen size
    char physSize[32] = "";
    if (screen->physX > 0.0 && screen->physY > 0.0)
    {
        float diagMm = fsqrt(screen->physX * screen->physX + screen->physY * screen->physY);
        float diagIn = (float)diagMm / 25.4f;
        float diagInRounded = (float)(int)(diagIn * 2.0f + 0.5f) / 2.0f;

        if (diagInRounded == (int)diagInRounded)
            snprintf(physSize, 32, "%d\" ", (int)diagInRounded);
        else
            snprintf(physSize, 32, "%.1f\" ", diagInRounded);
    }

    // Prepare refresh rate
    char refresh[32] = "";
    if (screen->refresh > 0)
    {
        if (COMPACT)
            snprintf(refresh, 32, "@%d", screen->refresh);
        else
            snprintf(refresh, 32, " @ %dHz", screen->refresh);
    }

    // Prepare connector name
    char connector[32] = "";
    if (screen->connector[0] != '\0')
        snprintf(connector, 32, " (%s)", screen->connector);
    free(screen->connector);

    // Assemble the string
    if (!COMPACT)
        snprintf(screenStr, SCREEN_SIZE, "%s%dx%d%s%s", physSize, screen->resX, screen->resY, refresh, connector);
    else
        snprintf(screenStr, SCREEN_SIZE, "%s%dx%d%s", physSize, screen->resX, screen->resY, refresh);

    return screenStr;
}

void showHelp(void)
{
    char desc[100] = "A tool that displays basic system and environment information in a summarised format.\n";
    formatNewLines(desc, TERM_SIZE.ws_col, NULL, 0);
    printf("%s\n", desc);

    char usage[50] = "Usage: shorkfetch [OPTIONS]\n\n";
    formatNewLines(usage, TERM_SIZE.ws_col, NULL, 0);
    printf("%s", usage);

    char options[20] = "Options:\n";
    formatNewLines(options, TERM_SIZE.ws_col, NULL, 0);
    printf("%s", options);

    char bullets[130] = "-b, --bullets      Uses bullet points instead of field headings; can also be used to set a custom character\n";
    formatNewLines(bullets, TERM_SIZE.ws_col, "                   ", 0);
    printf("%s", bullets);

    char categories[90] = "-ca, --categories  Groups and divides similar fields with dashed lines\n";
    formatNewLines(categories, TERM_SIZE.ws_col, "                   ", 0);
    printf("%s", categories);

    char compact[100] = "-c, --compact      Compacts field names (if not using bullets) and field values\n";
    formatNewLines(compact, TERM_SIZE.ws_col, "                   ", 0);
    printf("%s", compact);

    char help[70] = "-h, --help         Displays help information and exits\n";
    formatNewLines(help, TERM_SIZE.ws_col, "                   ", 0);
    printf("%s", help);

    char fields[140] = "-f, --fields       Allows you to specify which fields to show via a comma-separated list (os,krn,...)\n";
    formatNewLines(fields, TERM_SIZE.ws_col, "                   ", 0);
    printf("%s", fields);

    char noArt[100] = "-na, --no-art      Disables the SHORK ASCII art (if compiled with art support)\n";
    formatNewLines(noArt, TERM_SIZE.ws_col, "                   ", 0);
    printf("%s", noArt);

    char noCol[100] = "-nc, --no-col      Disables all coloured output (if compiled with colour support)\n\n";
    formatNewLines(noCol, TERM_SIZE.ws_col, "                   ", 0);
    printf("%s", noCol);

    char fieldNames[100] = "Field names:\nos, krn, upt, pkgs, scn, de, wm, trm, sh, cpu, gpu, ram, swap, root, lip\n";
    formatNewLines(fieldNames, TERM_SIZE.ws_col, NULL, 0);
    printf("%s", fieldNames);
}



/**
 * @return String containing the current username;"unknown" if undetermined/error
 */
char *getUsername(void)
{
    char *username = getenv("USER");
    if (!username || username[0] == '\0')
        username = getenv("LOGNAME");
    if (!username || username[0] == '\0') 
        username = strdup("unknown");
    return username;
}

/**
 * @param u Parsed uname data
 * @param uStatus The status returned from uname attempt
 * @return String containing the hostname; "unknown" if undetermined/error
 */
char *getHostname(struct utsname u, int uStatus)
{
    if (uStatus == 0) return strdup(u.nodename);
    return strdup("unknown");
}

/**
 * @param u Parsed uname data
 * @param uStatus The status returned from uname attempt
 * @return String containing the OS/Linux distro's name or "unknown" if undetermined/error
 */
char *getOS(struct utsname u, int uStatus)
{
    const int osSize = 128;
    char *os = malloc(osSize);
    if (!os) return strdup("unknown");
    os[0] = '\0';

    // Try os-release
    FILE *fStream = fopen("/etc/os-release", "r");
    if (fStream)
    {
        char buffer[128];
        while (fgets(buffer, sizeof(buffer), fStream))
        {
            if (strncmp(buffer, "PRETTY_NAME=", 12) == 0)
            {
                char *extract = extractFromPoint(buffer, osSize, '=', 1);
                strncpy(os, extract, osSize - 1);
                free(extract);
                break;
            }
        }
        fclose(fStream);
    }

    // Try issue
    if (os[0] == '\0')
    {
        fStream = fopen("/etc/issue", "r");
        if (fStream)
        {
            char buffer[osSize];
            if (fgets(buffer, sizeof(buffer), fStream))
            {
                size_t len = strlen(buffer);
                if (len > 0 && buffer[len - 1] == '\n') buffer[len - 1] = '\0';
                char *p = strchr(buffer, '\\');
                if (p) *p = '\0';
                strncpy(os, buffer, osSize - 1);
                os[osSize - 1] = '\0';
            }
            fclose(fStream);
        }
    }

    // Try uname -o
    if (os[0] == '\0')
        if (uStatus == 0)
            strncpy(os, u.sysname, osSize - 1);

    // Fallback
    if (os[0] == '\0')
        strcpy(os, "unknown");

    // If the name is wrapped in apostrophes, remove them
    if (os[0] == '\'')
    {
        size_t osLen = strlen(os);
        if (osLen >= 2 && os[osLen - 1] == '\'')
        {
            memmove(os, os + 1, osLen - 2);
            os[osLen - 2] = '\0';
        }
    }
    
    if (COMPACT)
    {
        // Remove trailing bracketed substring if present
        size_t osLen = strlen(os);
        if (osLen > 0 && os[osLen - 1] == ')')
        {
            for (int i = osLen - 1; i > 0; i--)
            {
                if (os[i] == '(' && i > 0 && os[i - 1] == ' ')
                {
                    os[i - 1] = '\0';
                    break;
                }
            }
        }
    
        int replaces = 0;
        for (int i = 0; i < COMPACT_OS_REPLACES_LEN; i++)
        {
            if (COMPACT_OS_REPLACES[i].standalone && replaces > 0) continue;
            else if (strstr(os, COMPACT_OS_REPLACES[i].match))
            {
                char *tmp = findReplace(os, osSize, COMPACT_OS_REPLACES[i].match, COMPACT_OS_REPLACES[i].replacement);
                strncpy(os, tmp, osSize - 1);
                os[osSize - 1] = '\0';
                free(tmp);
                replaces++;
            }
        }
    }

    return os;
}

/**
 * @param u Parsed uname data
 * @param uStatus The status returned from uname attempt
 * @return String containing the kernel version or "unknown" if undetermined/error
 */
char *getKernel(struct utsname u, int uStatus)
{
    if (uStatus != 0) return strdup("unknown");
    const char *src = u.release;

    if (!COMPACT)
    {
        char *release = malloc(strlen(src) + 1);
        if (!release) return strdup("unknown");
        strcpy(release, u.release);
        return release; 
    }
    else
    {
        // Strip out any suffixes
        int i = 0;
        while (src[i])
        {
            char c = src[i];
            if (!((c >= '0' && c <= '9') || c == '.')) break;
            i++;
        }

        char *release = malloc(i + 1);
        if (!release) return strdup("unknown");
        memcpy(release, src, i);
        release[i] = '\0';
        return release;
    }
}

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
                const char *dayUnit = (days == 1) ? "day" : "days";
                const char *hourUnit = (hours == 1) ? "hour" : "hours";
                const char *minUnit = (minutes == 1) ? "minute" : "minutes";

                if (days > 0)
                    snprintf(uptime, 128, "%d %s, %d %s, %d %s", days, dayUnit, hours, hourUnit, minutes, minUnit);
                else
                    snprintf(uptime, 128, "%d %s, %d %s", hours, hourUnit, minutes, minUnit);
            }
            else
            {
                if (days > 0)
                    snprintf(uptime, 128, "%d:%d:%d", days, hours, minutes);
                else
                    snprintf(uptime, 128, "%d:%d", hours, minutes);
            }
        }
        fclose(fStream);
    }
    else strcpy(uptime, "unknown");

    return uptime;
}

/**
 * @return String containing counts of various packages including dpkg, pacman,
 * rpm, flatpak and snap.
 */
char *getPackages(const char *os)
{
    // We know for sure SHORK doesn't have a package manager...
    if (os && strncmp(os, "SHORK", 5) == 0)
        return NULL;

    const int PKGS_SIZE = 256;
    char *pkgs = malloc(PKGS_SIZE);
    if (!pkgs) return NULL;
    pkgs[0] = '\0';

    int dCount = 0;
    int pCount = 0;
    int rCount = 0;
    int fCount = 0;
    int sCount = 0;

    // Get Debian-style packages by counting inside /var/lib/dpkg/status
    FILE *dpkgStatus = fopen("/var/lib/dpkg/status", "r");
    if (dpkgStatus)
    {
        const char *needle = "Status: install ok installed";
        size_t needleLen = strlen(needle);
        char buffer[512];
        while (fgets(buffer, 512, dpkgStatus))
            if (strncmp(buffer, needle, needleLen) == 0)
                dCount++;
        fclose(dpkgStatus);
    }

    // Get Arch-style packages by counting inside /var/lib/pacman/local
    DIR *pacmanLocal = opendir("/var/lib/pacman/local");
    if (pacmanLocal)
    {
        struct dirent *dirEntry;
        while ((dirEntry = readdir(pacmanLocal)) != NULL)
            if (dirEntry->d_name[0] != '.' && strcmp(dirEntry->d_name, "ALPM_DB_VERSION") != 0)
                pCount++;
        closedir(pacmanLocal);
    }

    // Get Fedora-style packages
    if (isProgramInstalled("rpm", 0))
    {
        // Try rpm for now (it's slow, we should find a better way...)
        FILE *fStream = popen("rpm -qa 2>/dev/null | wc -l", "r");
        if (fStream)
        {
            fscanf(fStream, "%d", &rCount);
            pclose(fStream);
        }
    }

    // Get Flatpak packages
    if (isProgramInstalled("flatpak", 0))
    {
        // Try quickly figuring the number out using the filesystem
        char userApp[PATH_MAX], userRuntime[PATH_MAX];
        snprintf(userApp, PATH_MAX, "%s/.local/share/flatpak/app", HOME);
        snprintf(userRuntime, PATH_MAX, "%s/.local/share/flatpak/runtime", HOME);

        // The directories we need to check - system and user apps and runtimes
        const char *flatpakDirs[] = {
            "/var/lib/flatpak/app",
            "/var/lib/flatpak/runtime",
            userApp,
            userRuntime
        };

        // We are looking for "active" symbolic link files. The tree looks like:
        // flatpakDir[i]/org.kde.Platform/x86_64/6.9   /active
        //              /name            /arch  /branch/BINGO
        for (int i = 0; i < 4; i++)
        {
            DIR *flatpakDir = opendir(flatpakDirs[i]);
            if (!flatpakDir) continue;

            // Enter arch
            struct dirent *nameEntry;
            while ((nameEntry = readdir(flatpakDir)) != NULL)
            {
                if (nameEntry->d_name[0] == '.') continue;

                char archPath[PATH_MAX];
                snprintf(archPath, PATH_MAX, "%s/%s", flatpakDirs[i], nameEntry->d_name);
                DIR *archDir = opendir(archPath);
                if (!archDir) continue;

                // Enter branch
                struct dirent *archEntry;
                while ((archEntry = readdir(archDir)) != NULL)
                {
                    if (archEntry->d_name[0] == '.') continue;

                    char branchPath[PATH_MAX];
                    snprintf(branchPath, PATH_MAX, "%s/%s", archPath, archEntry->d_name);
                    DIR *branchDir = opendir(branchPath);
                    if (!branchDir) continue;

                    // Look for out crucial "active" file
                    struct dirent *branchEntry;
                    while ((branchEntry = readdir(branchDir)) != NULL)
                    {
                        if (branchEntry->d_name[0] == '.') continue;

                        char activePath[PATH_MAX];
                        snprintf(activePath, PATH_MAX, "%s/%s/active", branchPath, branchEntry->d_name);
                        if (access(activePath, F_OK) != 0) continue;

                        // flatpak list seems to skip .Locale, so we do so to
                        // match its output
                        size_t nameLen = strlen(nameEntry->d_name);
                        if (nameLen > 7 && strcmp(nameEntry->d_name + nameLen - 7, ".Locale") == 0)
                            continue;

                        fCount++;
                    }
                    closedir(branchDir);
                }
                closedir(archDir);
            }
            closedir(flatpakDir);
        }
    }

    // Get Snap packages by counting inside /snap
    DIR *snapDir = opendir("/snap");
    if (snapDir)
    {
        struct dirent *dirEntry;
        while ((dirEntry = readdir(snapDir)) != NULL)
            if (dirEntry->d_type == DT_DIR && dirEntry->d_name[0] != '.' && strcmp(dirEntry->d_name, "bin") != 0)
                sCount++;
        closedir(snapDir);
    }

    // Build the result string
    if (dCount > 0)
        snprintf(pkgs, PKGS_SIZE, COMPACT ? "%d(D)" : "%d (dpkg)", dCount);
    if (pCount > 0)
        snprintf(pkgs + strlen(pkgs), PKGS_SIZE - strlen(pkgs), COMPACT ? ":%d(P)" : ", %d (pacman)", pCount);
    if (rCount > 0)
        snprintf(pkgs + strlen(pkgs), PKGS_SIZE - strlen(pkgs), COMPACT ? ":%d(R)" : ", %d (rpm)", rCount);
    if (fCount > 0)
        snprintf(pkgs + strlen(pkgs), PKGS_SIZE - strlen(pkgs), COMPACT ? ":%d(F)" : ", %d (flat)", fCount);
    if (sCount > 0)
        snprintf(pkgs + strlen(pkgs), PKGS_SIZE - strlen(pkgs), COMPACT ? ":%d(S)" : ", %d (snap)", sCount);

    // Make sure we don't start with ", "...
    size_t pkgsLen = strlen(pkgs);
    if (pkgsLen > 2 && pkgs[0] == ',' && pkgs[1] == ' ')
        memmove(pkgs, pkgs + 2, pkgsLen - 1);

    return pkgs;
}

/**
 * @param count Number of screens detected (intended to be used by reference)
 * @return Pointer to Screen structs containing detected GPUs
 */
Screen *getScreens(int *count)
{
    if (!count) return NULL;

    // If we don't think we're in a graphical environment, time to leave...
    if (!WAYLAND_PRESENT && !X11_PRESENT)
        return NULL;

    Screen *screens = NULL;

    // Try getting screens with xrandr (X11)
    if (isProgramInstalled("xrandr", 0))
    {
        FILE *fStream = popen("xrandr 2>/dev/null", "r");
        if (fStream)
        {
            // What we use to read lines of xrandr output in to
            char buffer[512];
            // Flag when we're reading a connected screen
            int in = 0;

            while (fgets(buffer, 512, fStream))
            {
                // Only process lines for things "connected" or inside a "connected"'s block
                char *isConnected = strstr(buffer, " connected");
                if (isConnected) 
                {
                    // Reallocate screens array to take into account new screen
                    screens = realloc(screens, ((*count) + 1) * sizeof(Screen));
                    memset(&screens[(*count)], 0, sizeof(Screen));

                    // Parse connector name
                    char pConnector[64] = {0};
                    sscanf(buffer, "%63s", pConnector);
                    screens[*count].connector = strdup(pConnector);

                    // Replace "Virtual-" with "Virt-" if present
                    char *virtNeedle = strstr(screens[*count].connector, "Virtual-");
                    if (virtNeedle)
                    {
                        memcpy(virtNeedle, "Virt-", 5);
                        memmove(virtNeedle + 5, virtNeedle + 8, strlen(virtNeedle + 8) + 1);
                    }

                    // Flag if connector is for primary screen
                    screens[*count].isPrimary = strstr(buffer, " primary ") != NULL;

                    // Parse physical size
                    screens[*count].physX = 0.0;
                    screens[*count].physY = 0.0;
                    char *needle = strstr(buffer, "mm x");
                    if (needle)
                    {
                        char *start = needle;
                        while (start > buffer && *(start - 1) != ' ')
                            start--;

                        int pPhysX = 0, pPhysY = 0;
                        if (sscanf(start, "%dmm x %dmm", &pPhysX, &pPhysY) == 2)
                        {
                            screens[*count].physX = pPhysX;
                            screens[*count].physY = pPhysY;
                        }
                    }

                    // Parse resolution
                    needle = isConnected;
                    int pResX = 0, pResY = 0;
                    while (*needle && (*needle < '0' || *needle > '9')) needle++;
                    sscanf(needle, "%dx%d", &pResX, &pResY);

                    screens[*count].resX = pResX;
                    screens[*count].resY = pResY;

                    // Flag that we are in a block and thus should search for more info on other lines
                    in = 1;
                }
                else if (strstr(buffer, "disconnected") || !in)
                {
                    in = 0;
                    continue;
                }

                // Look for current refresh rate
                if (strchr(buffer, '*'))
                {
                    char *start = strchr(buffer, '*');
                    while (start > buffer)
                    {
                        char c = *(start - 1);
                        if ((c >= '0' && c <= '9') || c == '.') start--;
                        else break;
                    }
                    screens[*count].refresh = (int)(atof(start) + 0.5);

                    // At this point, we got everything we can for this screen, so let's go
                    (*count)++;
                    in = 0;
                    continue;
                }
            }

            if (in) (*count)++;

            pclose(fStream);
        }
    }

    // If we still have nothing, we can try DRM as an X11/Wayland agnostic fallback
    if (!screens)
    {
        DIR *dirStream = opendir("/sys/class/drm");

        if (dirStream)
        {
            struct dirent *entry;
            while ((entry = readdir(dirStream)))
            {
                // Skip non-connector entries
                if (strstr(entry->d_name, "-") == NULL)
                    continue;

                // Prepare to test connector status
                char path[PATH_MAX];
                snprintf(path, PATH_MAX, "/sys/class/drm/%s/status", entry->d_name);

                FILE *fileStream = fopen(path, "r");
                if (!fileStream) continue;

                // Check status
                char status[64] = {0};
                fgets(status, 64, fileStream);
                fclose(fileStream);

                // Move on if anything but "connected"
                if (strncmp(status, "connected", 9) != 0)
                    continue;

                // Prepare to parse mode for resolution
                snprintf(path, PATH_MAX, "/sys/class/drm/%s/modes", entry->d_name);
                fileStream = fopen(path, "r");
                if (!fileStream) continue;

                char mode[64] = {0};
                fgets(mode, 64, fileStream);
                fclose(fileStream);

                // Parse mode for resolution
                int pResX = 0, pResY = 0;
                sscanf(mode, "%dx%d", &pResX, &pResY);

                // If we got nothing, no point of continuing...
                if (pResX <= 0 || pResY <= 0) continue;

                // Reallocate screens array to take into account new screen
                screens = realloc(screens, ((*count) + 1) * sizeof(Screen));
                memset(&screens[(*count)], 0, sizeof(Screen));

                // Populate screen data
                screens[(*count)].connector = strdup(entry->d_name);
                screens[(*count)].physX = 0.0;
                screens[(*count)].physY = 0.0;
                screens[(*count)].resX = pResX;
                screens[(*count)].resY = pResY;
                screens[(*count)].refresh = 0;

                // Remove "cardX-" prefix if present
                if (strncmp(screens[(*count)].connector, "card", 4) == 0)
                    memmove(screens[(*count)].connector, screens[(*count)].connector + 6, strlen(screens[(*count)].connector + 6) + 1);

                // Replace "Virtual-" with "Virt-" if present
                char *virtNeedle = strstr(screens[*count].connector, "Virtual-");
                if (virtNeedle)
                {
                    memcpy(virtNeedle, "Virt-", 5);
                    memmove(virtNeedle + 5, virtNeedle + 8, strlen(virtNeedle + 8) + 1);
                }

                (*count)++;
            }

            closedir(dirStream);
        }
    }

    return screens;
}

/**
 * @return String containing the active display environment's name; NULL if not found/applicable
 */
char *getDE(void)
{
    // If we don't think we're in a graphical environment, time to leave...
    if (!WAYLAND_PRESENT && !X11_PRESENT)
        return NULL;

    char *de = NULL;

    // Test standardised DE environment var
    if (XDG_CURRENT_DESKTOP && XDG_CURRENT_DESKTOP[0] != '\0')
        de = strdup(XDG_CURRENT_DESKTOP);

    // Do some cleaning if needed
    if (de)
    {
        // Remove "ubuntu" in "ubuntu:GNOME"
        if (strncmp(de, "ubuntu:", 7) == 0)
            memmove(de, de + 7, strlen(de + 7) + 1);

        // Remove "X-" in "X-Cinnamon"
        if (strncmp(de, "X-", 2) == 0)
            memmove(de, de + 2, strlen(de + 2) + 1);

        // "Prettify" XFCE to Xfce
        if (strncmp(de, "XFCE", 4) == 0)
        {
            free(de);
            de = strdup("Xfce");
        }

        // Discard ":Unity7:ubuntu" from "Unity:Unity7:ubuntu" (etc.)
        if (strncmp(de, "Unity", 5) == 0)
        {
            char *needle = strchr(de, ':');
            if (needle) *needle = '\0';
        }
        
        // Discard ":wlroots" from "sway:wlroots"
        if (strncmp(de, "sway", 4) == 0)
        {
            char *needle = strchr(de, ':');
            if (needle) *needle = '\0';
        }
    }

    return de;
}

/**
 * @param de Desktop enivornment's name
 * @return String containing the active window manager's name; NULL if not found/applicable
 */
char *getWM(char **de)
{
    // If we don't think we're in a graphical environment, time to leave...
    if (!WAYLAND_PRESENT && !X11_PRESENT)
        return NULL;

    // Cinnamon's WM (Muffin) is internal, we have to assume instead of look for
    // the process
    if (de && *de && strstr(*de, "Cinnamon") != NULL)
        return strdup("Muffin");

    // Run through our WM database
    for (size_t i = 0; i < WINDOW_MANAGERS_LEN; i++)
    {
        if (procExists(WINDOW_MANAGERS[i].cmd, 0))
        {
            // If DE == WM, we may treat this as just a WM
            if (de && *de)
            {
                // Convert both subjects to all caps for a case-insensitive 
                // comparison
                char *deCaps = strdup(*de);
                    for (size_t j = 0; deCaps[j]; j++)
                        if (deCaps[j] >= 'a' && deCaps[j] <= 'z')
                            deCaps[j] -= 32;
                char *wmCaps = strdup(WINDOW_MANAGERS[i].name);
                for (size_t j = 0; wmCaps[j]; j++)
                    if (wmCaps[j] >= 'a' && wmCaps[j] <= 'z')
                        wmCaps[j] -= 32;

                if (strstr(deCaps, wmCaps) != NULL)
                {
                    free(deCaps);
                    free(wmCaps);
                    char *wm = strdup(WINDOW_MANAGERS[i].name);
                    *de = wm;
                    return wm;
                }

                free(deCaps);
                free(wmCaps);
            }

            return strdup(WINDOW_MANAGERS[i].name);
        }
    }

    // If we haven't found a WM but we have a DE, there's a good chance DE/WM
    // are one and the same
    if (de && *de)
        return *de;

    return NULL;
}

/**
 * @return String containing the host terminal emulator's name; NULL if not found/applicable
 */
char *getTerminal(void)
{
    // If we don't think we're in a graphical environment, time to leave...
    if (!WAYLAND_PRESENT && !X11_PRESENT)
        return NULL;

    char *terminal = NULL;

    // Try the easy way ($TERM_PROGRAM) first
    char *termProgram = getenv("TERM_PROGRAM");
    if (termProgram && termProgram[0] != '\0')
        terminal = strdup(termProgram);

    // Try looking through our parent processes to get the name
    if (!terminal)
    {
        Process process = getParentProcess(getpid());
        while (process.pid > 1)
        {
            // Flags if we must not use this process as our terminal
            int notTerminal = 0;

            // We must skip wrappers like doas, su or sudo, and possible shells
            for (int i = 0; i < EXCLUDED_TERMINAL_PROCS_LEN; i++)
            {
                if (strcmp(process.name, EXCLUDED_TERMINAL_PROCS[i]) == 0)
                {
                    notTerminal = 1;
                    break;
                }
            }

            if (!notTerminal)
            {
                terminal = strdup(process.name);
                break;
            }

            process = getParentProcess(process.pid);
        }
    }

    // As a fallback, we can also try $TERM to get the terminal's basic 
    // capabilities like "xterm-256color"
    if (!terminal)
    {
        const char *TERM = getenv("TERM");
        if (TERM && TERM[0] != '\0')
        {
            if (COMPACT) terminal = strdup(TERM);
            else
            {
                size_t termLen = strlen(TERM) + 11 + 1;
                terminal = malloc(termLen);
                if (terminal)
                    snprintf(terminal, termLen, "%s compatible", TERM);
            }
        }
    }

    // Do some cleaning if needed
    if (terminal)
    {
        size_t terminalLen = strlen(terminal);

        // Remove trailing hyphen from "gnome-terminal-" (etc.)
        if (terminalLen > 0 && terminal[terminalLen - 1] == '-')
            terminal[terminalLen - 1] = '\0';

        // Remove "agent" from "ptyxis-agent"
        if (terminalLen > 6 && terminal[6] == '-' && strncmp(terminal, "ptyxis", 6) == 0)
            terminal[6] = '\0';
    }

    return terminal;
}

/**
 * @return String containing the shell's name or "unknown" if undetermined/error
 */
char *getShell(void)
{
    char *shell = getenv("SHELL");
    if (!shell || shell[0] == '\0') 
        shell = strdup("unknown");
    else
        shell = strdup(basename(shell));
    return shell;
}

/**
 * @return String containing the CPU's name and core/thread specs; empty string if unknown
 */
char *getCPU(void)
{
    char *cpu = malloc(134);
    char *vendor = malloc(16);
    char *implementer = malloc(16);
    char *model = malloc(128);
    char *architecture = malloc(4);
    char *processor = malloc(4);
    char *cores = malloc(4);
    char *threads = malloc(4);
    char *fpu = malloc(4);
    if (!cpu || !vendor || !implementer || !model || !architecture || !processor || !cores || !threads || !fpu) 
    {
        free(cpu);
        free(vendor);
        free(implementer);
        free(model);
        free(architecture);
        free(processor);
        free(cores);
        free(threads);
        free(fpu);
        return strdup("unknown");
    }
    cpu[0] = vendor[0] = implementer[0] = model[0] = architecture[0] = processor[0] = cores[0] = threads[0] = fpu[0] = '\0';



    FILE *fStream = fopen("/proc/cpuinfo", "r");
    if (fStream)
    {
        // Use these to stop parsing once we have everything we need!
        // lookingFor's default value is x86 orientated - the ARM-based path
        // can change this to 2, hence not a const.
        int lookingFor = 5;
        int parsed = 0;

        char buffer[128];
        while (fgets(buffer, sizeof(buffer), fStream) && parsed < lookingFor)
        {
            if (strncmp(buffer, "processor", 9) == 0)
            {
                char *extract = extractFromPoint(buffer, 4, ':', 2);
                strncpy(processor, extract, 3);
                free(extract);
            }
            else if (strncmp(buffer, "vendor_id", 9) == 0)
            {
                char *extract = extractFromPoint(buffer, 16, ':', 2);
                strncpy(vendor, extract, 15);
                free(extract);
                parsed++;
            }
            else if (strncmp(buffer, "CPU implementer", 15) == 0)
            {
                char *extract = extractFromPoint(buffer, 16, ':', 2);
                strncpy(implementer, extract, 15);
                free(extract);
                lookingFor = 2;
            }
            else if (strncmp(buffer, "model name", 10) == 0)
            {
                char *extract = extractFromPoint(buffer, 128, ':', 2);
                strncpy(model, extract, 127);
                free(extract);
                parsed++;
            }
            else if (strncmp(buffer, "CPU architecture", 16) == 0)
            {
                char *extract = extractFromPoint(buffer, 4, ':', 2);
                strncpy(architecture, extract, 3);
                free(extract);
                lookingFor = 2;
            }
            else if (strncmp(buffer, "cpu cores", 9) == 0)
            {
                char *extract = extractFromPoint(buffer, 4, ':', 2);
                strncpy(cores, extract, 3);
                free(extract);
                parsed++;
            }
            else if (strncmp(buffer, "siblings", 8) == 0)
            {
                char *extract = extractFromPoint(buffer, 4, ':', 2);
                strncpy(threads, extract, 3);
                free(extract);
                parsed++;
            }
            else if (strncmp(buffer, "fpu", 3) == 0)
            {
                char *extract = extractFromPoint(buffer, 4, ':', 2);
                if (extract)
                {
                    if (strncmp(extract, "yes", 3) == 0)
                        strncpy(fpu, "1", 2);
                    else if (strncmp(extract, "no", 2) == 0)
                        strncpy(fpu, "0", 2);
                    free(extract);
                }
                parsed++;
            }
        }
        fclose(fStream);



        // Typical CPU path
        if (vendor[0] != '\0' || model[0] != '\0')
        {
            // Check if model name lacks the vendor name and if we need to try adding it in manually
            if ((vendor[0] != '\0' && vendor[0] != 'u') && (model[0] != '\0' && model[0] != 'u'))
            {
                if (!strstr(model, "Intel") && !strstr(model, "AMD") && !strstr(model, "Cyrix") && !strstr(model, "IDT") && !strstr(model, "VIA") && !strstr(model, "Transmeta"))
                {
                    char *tmp = malloc(128);
                    if (tmp)
                    {
                        if (strstr(vendor, "Intel") || strstr(vendor, "Iotel"))
                            snprintf(tmp, 128, "%s %s", "Intel", model);
                        else if (strstr(vendor, "AMD"))
                            snprintf(tmp, 128, "%s %s", "AMD", model);
                        else if (strstr(vendor, "Cyrix"))
                            snprintf(tmp, 128, "%s %s", "Cyrix", model);
                        else if (strstr(vendor, "Centaur"))
                            snprintf(tmp, 128, "%s %s", "IDT/Centaur", model);
                        else if (strstr(vendor, "VIA"))
                            snprintf(tmp, 128, "%s %s", "VIA", model);
                        else if (strstr(vendor, "Transmeta") || strstr(vendor, "TM"))
                            snprintf(tmp, 128, "%s %s", "Transmeta", model);
                        else
                            snprintf(tmp, 128, "%s %s", vendor, model);
                        
                        strncpy(model, tmp, 128);
                        free(tmp);
                        model[127] = '\0';
                    }
                }
            }

            // If we have a Cx486Dxxx with FPU, make sure 387 is included in the model name
            if ((strstr(model, "Cx486DLC") || strstr(model, "Cx486DRx2")) && fpu[0] == '1')
            {
                char tmp[128];
                snprintf(tmp, 128, "%s + 387", model);
                strncpy(model, tmp, 127);
                model[127] = '\0';
            }

            // If we have a Cx486S with FPU, make sure 487 is included in the model name
            if (strstr(model, "Cx486S") && fpu[0] == '1')
            {
                char tmp[128];
                snprintf(tmp, 128, "%s + 487", model);
                strncpy(model, tmp, 127);
                model[127] = '\0';
            }

            // If we have for certain an Intel 486SX with FPU, make sure 487 is included in the model name
            if (strstr(model, "486") && strstr(model, "SX") && fpu[0] == '1')
            {
                char tmp[128];
                snprintf(tmp, 128, "%s + 487", model);
                strncpy(model, tmp, 127);
                model[127] = '\0';
            }

            // If we have a vendorless and revisionless 486, we can at least infer if its purely 486SX, or
            // a 486DX, 487SX (true 486SX + 487SX) or 486SX + 387 (eg, IBM 486BLx/486SLCx  + 387), from the
            // presence of an FPU
            if ((vendor[0] == '\0' || vendor[0] == 'u') && model[0] != '\0' && strcmp(model, "486") == 0)
            {
                if (fpu[0] == '0')
                    snprintf(model, 127, "486SX");
                else if (fpu[0] == '1')
                    snprintf(model, 127, "486DX/487SX/486SX + 387");
            }
        }
        // Possible ARM CPU path
        if (architecture[0] != '\0')
        {
            const char *implementerName = NULL;
            // Try to resolve implementer name 
            if (implementer[0] != '\0')
            {
                char *end = NULL;
                long val = strtol(implementer, &end, 0);
                if (end != implementer && val >= 0 && val <= 193 && ARM_IMPLEMENTERS[val])
                    implementerName = ARM_IMPLEMENTERS[val];
            }

            if (implementerName)
                snprintf(model, 128, "%s Armv%d", implementerName, atoi(architecture));
            else
                snprintf(model, 128, "Armv%d", atoi(architecture));
        }
        // Absolute fallback - we have nothing to show
        else if (cores[0] == '\0' && threads[0] == '\0' && processor[0] == '\0')
        {
            free(vendor);
            free(implementer);
            free(model);
            free(architecture);
            free(processor);
            free(cores);
            free(threads);
            free(fpu);
            return cpu;
        }



        // If we don't have a cores value, set it to the same as threads
        // so we don't try to show them separately later
        if (cores[0] == '\0' && threads[0] != '\0')
            strncpy(cores, threads, 3);

        char coresAndThreads[16];

        // If we don't have cores or threads, we use the processor field in its place
        if (cores[0] == '\0' && threads[0] == '\0')
        {
            int processorInt = atoi(processor);
            processorInt++;
            snprintf(coresAndThreads, 16, "%dC", processorInt);
        }
        // If cores and threads are the same value, just show cores
        else if (strcmp(cores, threads) == 0)
            snprintf(coresAndThreads, 16, "%sC", cores);
        // If cores and threads are different values, show both
        else
            snprintf(coresAndThreads, 16, "%sC/%sT", cores, threads);



        // If we have no model name but we have core/thread count, just show the latter
        if (model[0] == '\0' && coresAndThreads[0] != '\0')
            strncpy(cpu, coresAndThreads, 133);
        // If we're in compact mode, we just show the model name
        else if (COMPACT)
            strncpy(cpu, model, 133);
        // Normal view
        else
        {
            if (coresAndThreads[0] != '\0')
                snprintf(cpu, 134, "%s (%s)", model, coresAndThreads);
            else
                strncpy(cpu, model, 133);
        }
    }
    else strcpy(cpu, "unknown");



    int coreCount = 0;
    if (cores && cores[0] != '\0')
        coreCount = (int)strtol(cores, NULL, 10);

    free(vendor);
    free(implementer);
    free(model);
    free(architecture);
    free(processor);
    free(cores);
    free(threads);
    free(fpu);

    char *cleanedCPU = cleanCPUName(cpu, 134, coreCount);
    strncpy(cpu, cleanedCPU, 133);
    free(cleanedCPU);

    return cpu;
}

/**
 * @param count Number of GPUs actually detected (intended to be used by reference)
 * @return Pointer to up to 4 GPU structs containing detected GPUs
 */
GPU* getGPUs(int *count)
{
    if (!count) return NULL;

    DIR *dir = opendir("/sys/bus/pci/devices");
    if (!dir)
    {
        *count = 0;
        return NULL;
    }

    struct dirent *entry;
    GPU *gpus = malloc(4 * sizeof(GPU));
    if (!gpus) 
    {
        *count = 0;
        return NULL;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_name[0] == '.') continue;

        char classPath[PATH_MAX];
        snprintf(classPath, sizeof(classPath), "%s/%s/class", "/sys/bus/pci/devices", entry->d_name);
        int class = readHexFile(classPath);
        class = (class >> 8) & 0xFFFF;

        // We only want class 0x30x...
        if ((class >> 8) == 0x03 && class != 0x0380)
        {
            char vendorPath[PATH_MAX], devicePath[PATH_MAX], revisionPath[PATH_MAX];
            snprintf(vendorPath, sizeof(vendorPath), "%s/%s/vendor", "/sys/bus/pci/devices", entry->d_name);
            snprintf(devicePath, sizeof(devicePath), "%s/%s/device", "/sys/bus/pci/devices", entry->d_name);
            snprintf(revisionPath, sizeof(revisionPath), "%s/%s/revision", "/sys/bus/pci/devices", entry->d_name);

            int vendor = readHexFile(vendorPath);
            int device = readHexFile(devicePath);
            int revision = readHexFile(revisionPath);

            if (EXCLUDED_PCI_DIDS_LEN > 0)
            {
                int excluded = 0;
                for (int i = 0; i < EXCLUDED_PCI_DIDS_LEN; i++)
                {
                    if (EXCLUDED_PCI_DIDS[i] == device)
                    {
                        excluded = 1;
                        break;
                    }
                }
                if (excluded) continue;
            }

            gpus[*count].vendor = vendor;
            gpus[*count].device = device;
            gpus[*count].revision = revision;
            (*count)++;

            if (*count == 4) break;
        }
    }
    closedir(dir);

    return gpus;
}

/**
 * @param mi /proc/meminfo data
 * @return String containing the system memory used and total amounts both numerically and as a percentage
 */
char *getRAM(MemInfo mi)
{
    const int ramSize = 64;
    char *ram = malloc(ramSize);
    if (!ram) return strdup("");
    ram[0] = '\0';

    long freeMem = mi.memFree + mi.buffers + mi.cached;
    long used = mi.memTotal - freeMem;
    char *usedStr = bytesToReadable("KiB", used);
    char *totalStr = bytesToReadable("KiB", mi.memTotal);

    if (!COMPACT)
    {
        int pct = mi.memTotal ? (int)((used * 100) / mi.memTotal) : 0;
        snprintf(ram, ramSize, "%s / %s (%d%%)", usedStr, totalStr, pct);
    }
    else snprintf(ram, ramSize, "%s / %s", usedStr, totalStr);
    
    free(usedStr);
    free(totalStr);

    return ram;
}

/**
 * @param mi /proc/meminfo data
 * @return String containing the system swap used and total amounts both numerically and as a percentage
 */
char *getSwap(MemInfo mi)
{
    if (mi.swapTotal == 0) return strdup("");

    const int swapSize = 64;
    char *swap = malloc(swapSize);
    if (!swap) return strdup("");
    swap[0] = '\0';

    long used = mi.swapTotal - mi.swapFree;
    char *usedStr = bytesToReadable("KiB", used);
    char *totalStr = bytesToReadable("KiB", mi.swapTotal);

    if (!COMPACT)
    {
        int pct = mi.swapTotal ? (int)((used * 100) / mi.swapTotal) : 0;
        snprintf(swap, swapSize, "%s / %s (%d%%)", usedStr, totalStr, pct);
    }
    else snprintf(swap, swapSize, "%s / %s", usedStr, totalStr);

    free(usedStr);
    free(totalStr);

    return swap;
}

/**
 * @return String containing the root partition's used and total size amounts both numerically and as a percentage
 */
char *getRoot(void)
{
    const int rootSize = 64;
    char *root = malloc(rootSize);
    if (!root) return strdup("");
    root[0] = '\0';

    struct statvfs fs;

    if (statvfs("/", &fs) != 0)
        return root;

    long long total = (long long)fs.f_blocks * fs.f_frsize;
    long long freeRoot  = (long long)fs.f_bfree * fs.f_frsize;
    long long used  = total - freeRoot;
    char *usedStr = bytesToReadable("B", used);
    char *totalStr = bytesToReadable("B", total);

    if (!COMPACT)
    {
        int pct = total ? (int)((used * 100) / total) : 0;
        snprintf(root, rootSize, "%s / %s (%d%%)", usedStr, totalStr, pct);
    }
    else snprintf(root, rootSize, "%s / %s", usedStr, totalStr);

    free(usedStr);
    free(totalStr);

    return root;
}

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
        if (!currIF->ifa_addr || currIF->ifa_addr->sa_family != AF_INET) continue;

        // Skip if down, not running, or loopback device
        if (!(currIF->ifa_flags & IFF_UP) || !(currIF->ifa_flags & IFF_RUNNING) || (currIF->ifa_flags & IFF_LOOPBACK))
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



#ifdef TESTS
/**
 * Tests the interpretGPU function to ensure it finds and cleans GPU names as
 * we expect it to.
 */
void testInterpretGPU(void)
{
    printf("########################\n");
    printf("## INTERPRET GPU TEST ##\n");
    printf("########################\n");
    
    GPU gpus[] = {
        {
            "ATI FirePro V (FireGL V) Graphics Adapter",
            0x1002,
            0x6784,
            0x00
        },
        {
            "Caicos [Radeon HD 6450/7450/8450 / R5 230 OEM]",
            0x1002,
            0x6779,
            -1
        },
        {
            "Tahiti XT GL [FirePro W9000]",
            0x1002,
            0x6780,
            -1
        },
        {
            "AMD FirePro W9000",
            0x1002,
            0x6780,
            0x00
        },
        {
            "Navi 33 [Radeon RX 7600/7600 XT/7600M XT/7600S/7700S / PRO W7600]",
            0x1002,
            0x7480,
            -1
        },
        {
            "AMD Radeon RX 7700S",
            0x1002,
            0x7480, 
            0xC1 
        },
        {
            "NV11 [GeForce2 MX/MX 400]",
            0x10de,
            0x0110,
            -1
        },
        {
            "NV43M [GeForce Go6200 TE / 6600 TE]",
            0x10de,
            0x0146,
            -1
        },
        {
            "G92GLM [Quadro FX 3700M]",
            0x10de,
            0x061e,
            -1
        },
        {
            "G94 [GeForce 9600 GT Green Edition]",
            0x10de,
            0x0624,
            -1
        },
        {
            "G96 [GeForce 9500 GA / 9600 GT / GTS 250]",
            0x10de,
            0x065d,
            -1
        },
        {
            "GF110 [GeForce GTX 560 Ti OEM]",
            0x10de,
            0x1082,
            -1
        },
        {
            "GF110 [GeForce GTX 560 Ti 448 Cores]",
            0x10de,
            0x1087,
            -1
        },
        {
            "GM204M [GeForce GTX 960 OEM / 970M]",
            0x10de,
            0x13d8,
            -1
        },
        {
            "GM204 [GeForce GTX 980]",
            0x10de,
            0x13c0,
            0xa1
        },
        {
            "TU104GLM [Quadro RTX 5000 Mobile / Max-Q]",
            0x10de,
            0x1eb5,
            -1
        },
        {
            "TU106 [GeForce RTX 2070 Rev. A]",
            0x10de,
            0x1f07,
            -1
        },
        {
            "AD103 [GeForce RTX 4080 SUPER]",
            0x10de,
            0x2702,
            -1
        }
    };
    const int noGPUs = sizeof(gpus) / sizeof(gpus[0]);

    for (int i = 0; i < noGPUs; i++)
    {
        char *gpu = interpretGPU(&gpus[i]);
        if (gpu[0] != '\0')
        {
            if (gpus[i].revision == -1)
                printf("%04x:%04x:--: \033[31m%s\033[0m -> \033[32m%s\033[0m\n", gpus[i].vendor, gpus[i].device, gpus[i].name, gpu);
            else
                printf("%04x:%04x:%02X: \033[31m%s\033[0m -> \033[32m%s\033[0m\n", gpus[i].vendor, gpus[i].device, gpus[i].revision, gpus[i].name, gpu);
        }
        free(gpu);
    }
}
#endif



int main(int argc, char *argv[])
{
    TERM_SIZE = getTerminalSize();

    char bullet = '*';
    char *colAccent = getAccentColour();
    char *colReset = (colAccent[0] == '\0') ? "" : "\033[" COL_RESET "m";
    int noIP = 0;
    int shorkLine = 0;
    int showCategories = 0;
#ifdef NO_ART
    int showShork = 0;
#else
    int showShork = 1;
#endif
    int useBullets = 0;

    int showOS = 1;
    int showKrn = 1;
    int showUpt = 1;
    int showPkgs = 1;
    int showScn = 1;
    int showDE = 1;
    int showWM = 1;
    int showTrm = 1;
    int showSh = 1;
    int showCPU = 1;
    int showGPU = 1;
    int showRAM = 1;
    int showSwap = 1;
    int showRoot = 1;
    int showLocIP = 1;

    for (int i = 1; i < argc; i++)
    {
        if ((strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "--help") == 0))
        {
            showHelp();
            return 0;
        }
        else if (strncmp(argv[i], "-b", 2) == 0 || strncmp(argv[i], "--bullets", 9) == 0)
        {
            useBullets = 1;

            char *bulletChar = NULL;
            if (strncmp(argv[i], "-b=", 3) == 0)
                bulletChar = &argv[i][3];
            else if (strncmp(argv[i], "--bullets=", 10) == 0)
                bulletChar = &argv[i][10];

            if (bulletChar)
            {
                if (bulletChar[0] == '\0')
                {
                    printf("ERROR: custom bullet point character not given\n");
                    return 1;
                }
                else if (bulletChar[1] != '\0')
                {
                    printf("ERROR: custom bullet point character can only be a single character\n");
                    return 1;
                }
                bullet = bulletChar[0];
            }
        }
        else if ((strcmp(argv[i], "-ca") == 0) || (strcmp(argv[i], "--categories") == 0))
            showCategories = 1;
        else if ((strcmp(argv[i], "-c") == 0) || (strcmp(argv[i], "--compact") == 0))
            COMPACT = 1;
        else if (strncmp(argv[i], "-f", 2) == 0 || strncmp(argv[i], "--fields", 8) == 0)
        {
            // Find "=" as our needle
            char *equalsNeedle = strchr(argv[i], '=');
            if (!equalsNeedle) 
            {
                printf("ERROR: invalid fields argument syntax\n");
                return 1;
            }
            equalsNeedle++;

            // Set all fields to "off" to begin with
            showOS = 0;
            showKrn = 0;
            showUpt = 0;
            showPkgs = 0;
            showScn = 0;
            showDE = 0;
            showWM = 0;
            showTrm = 0;
            showSh = 0;
            showCPU = 0;
            showGPU = 0;
            showRAM = 0;
            showSwap = 0;
            showRoot = 0;
            showLocIP = 0;
            int noFields = 0;

            // Copy values so we don't mess up argv
            char *csv = strdup(equalsNeedle);
            if (!csv) continue;

            // Parse the CSV for field names
            char *currTok = strtok(csv, ",");
            while (currTok)
            {
                if (strcmp(currTok, "os") == 0)
                {
                    showOS = 1;
                    noFields++;
                }
                else if (strcmp(currTok, "krn") == 0)
                {
                    showKrn = 1;
                    noFields++;
                }
                else if (strcmp(currTok, "upt") == 0)
                {
                    showUpt = 1;
                    noFields++;
                }
                else if (strcmp(currTok, "pkgs") == 0)
                {
                    showPkgs = 1;
                    noFields++;
                }
                else if (strcmp(currTok, "scn") == 0)
                {
                    showScn = 1;
                    noFields++;
                }
                else if (strcmp(currTok, "de") == 0)
                {
                    showDE = 1;
                    noFields++;
                }
                else if (strcmp(currTok, "wm") == 0)
                {
                    showWM = 1;
                    noFields++;
                }
                else if (strcmp(currTok, "trm") == 0)
                {
                    showTrm = 1;
                    noFields++;
                }
                else if (strcmp(currTok, "sh") == 0)
                {
                    showSh = 1;
                    noFields++;
                }
                else if (strcmp(currTok, "cpu") == 0)
                {
                    showCPU = 1;
                    noFields++;
                }
                else if (strcmp(currTok, "gpu") == 0)
                {
                    showGPU = 1;
                    noFields++;
                }
                else if (strcmp(currTok, "ram") == 0)
                {
                    showRAM = 1;
                    noFields++;
                }
                else if (strcmp(currTok, "swap") == 0)
                {
                    showSwap = 1;
                    noFields++;
                }
                else if (strcmp(currTok, "root") == 0)
                {
                    showRoot = 1;
                    noFields++;
                }
                else if (strcmp(currTok, "lip") == 0)
                {
                    showLocIP = 1;
                    noFields++;
                }
                else
                {
                    printf("ERROR: unrecognised field name \"%s\"\n", currTok);
                    return 1;
                }

                currTok = strtok(NULL, ",");
            }

            if (noFields == 0)
            {
                printf("ERROR: no invalid field names were given\n");
                return 1;
            }
            else if (noFields < 6) showShork = 0;
            else if (noFields == 6) shorkLine = 1;
            free(csv);
        }
        else if ((strcmp(argv[i], "-na") == 0) || (strcmp(argv[i], "--no-art") == 0))
            showShork = 0;
        else if ((strcmp(argv[i], "-nc") == 0) || (strcmp(argv[i], "--no-col") == 0))
            colAccent = colReset = "";
        else if ((strcmp(argv[i], "-ni") == 0) || (strcmp(argv[i], "--no-ip") == 0))
            noIP = 1;
    }



    HOME =  getenv("HOME");

    MemInfo mi = {0};
    if (showRAM || showSwap)
    {
        mi = getMemInfo();
        if (mi.memTotal <= 0)
        {
            printf("ERROR: could not load data from /proc/meminfo\n");
            return 1;
        }
    }

    struct utsname u;
    int uStatus = uname(&u);

    char *envWay = getenv("WAYLAND_DISPLAY");
    WAYLAND_PRESENT = (envWay != NULL && envWay[0] != '\0');
    char *envX11 = getenv("DISPLAY");
    X11_PRESENT = (envX11 != NULL && envX11[0] != '\0');
    if (WAYLAND_PRESENT || X11_PRESENT)
        XDG_CURRENT_DESKTOP = getenv("XDG_CURRENT_DESKTOP");



#ifdef TESTS
    testInterpretGPU();
    return 0;
#endif



    char *username = getUsername();
    char *hostname = getHostname(u, uStatus);

    char *os = showOS ? getOS(u, uStatus) : NULL;
    char *kernel = showKrn ? getKernel(u, uStatus) : NULL;
    char *uptime = showUpt ? getUptime() : NULL;
    char *pkgs = showPkgs ? getPackages(os): NULL;

    int noScreens = 0;
    Screen *screens = showScn ? getScreens(&noScreens) : NULL;
    char *de = showDE ? getDE() : NULL;
    char *wm = showWM ? getWM(&de) : NULL;
    char *trm = showTrm ? getTerminal() : NULL;
    char *shell = showSh ? getShell() : NULL;

    char *cpu = showCPU ? getCPU() : NULL;
    int noGPUs = 0;
    GPU *gpus = showGPU ? getGPUs(&noGPUs) : NULL;
    char *ram = showRAM ? getRAM(mi) : NULL;
    char *swap = showSwap ? getSwap(mi) : NULL;
    char *root = showRoot ? getRoot() : NULL;

    char *localIP = (!noIP && showLocIP) ? getLocalIP() : NULL;



    size_t headerWidth = 12;
    if (username[0] != '\0' && hostname[0] != '\0')
    {
        if (showShork) printf("%s%s%s", colAccent, SHORK[shorkLine++], colReset);
        printf("%s%s%s@%s%s%s\n", colAccent, username, colReset, colAccent, hostname, colReset);
        if (showShork) printf("%s%s%s", colAccent, SHORK[shorkLine++], colReset);

        headerWidth = strlen(username) + 1 + strlen(hostname);
        for (size_t i = 0; i < headerWidth; i++) printf("-");
        putchar('\n');
    }

    if (os && os[0] != '\0')
    {
        if (showShork) printf("%s%s%s", colAccent, SHORK[shorkLine++], colReset);
        if (!useBullets)
        {
            if (!COMPACT)
                printf("%sOS:%s       %s\n", colAccent, colReset, os);
            else
                printf("%sOS:%s  %s\n", colAccent, colReset, os);
        }
        else printf(" %s%c%s %s\n", colAccent, bullet, colReset, os);
    }

    if (kernel && kernel[0] != '\0')
    {
        if (showShork) printf("%s%s%s", colAccent, SHORK[shorkLine++], colReset);
        if (!useBullets)
        {
            if (!COMPACT)
                printf("%sKernel:%s   %s\n", colAccent, colReset, kernel);
            else
                printf("%sKrn:%s %s\n", colAccent, colReset, kernel);
        }
        else printf(" %s%c%s %s\n", colAccent, bullet, colReset, kernel);
    }

    if (uptime && uptime[0] != '\0')
    {
        if (showShork) printf("%s%s%s", colAccent, SHORK[shorkLine++], colReset);
        if (!useBullets)
        {
            if (!COMPACT)
                printf("%sUptime:%s   %s\n", colAccent, colReset, uptime);
            else
                printf("%sUp:%s  %s\n", colAccent, colReset, uptime);
        }
        else printf(" %s%c%s %s\n", colAccent, bullet, colReset, uptime);
    }

    if (pkgs && pkgs[0] != '\0')
    {
        if (showShork) printf("%s%s%s", colAccent, SHORK[shorkLine++], colReset);
        if (!useBullets)
        {
            if (!COMPACT)
                printf("%sPackages:%s %s\n", colAccent, colReset, pkgs);
            else
                printf("%sPkg:%s %s\n", colAccent, colReset, pkgs);
        }
        else printf(" %s%c%s %s\n", colAccent, bullet, colReset, pkgs);
    }

    if (showCategories)
    {
        if (showShork) printf("%s%s%s", colAccent, SHORK[shorkLine++], colReset);
        for (size_t i = 0; i < headerWidth; i++) printf("-");
        putchar('\n');
    }

    if (screens)
    {
        int pastFirstScreen = 0;
        for (int i = 0; i < noScreens; i++)
        {
            char *screen = interpretScreen(&screens[i]);

            if (screen && screen[0] != '\0')
            {
                if (showShork) printf("%s%s%s", colAccent, SHORK[shorkLine++], colReset);

                if (!useBullets)
                {
                    if (!COMPACT)
                    {
                        // No compact - no bullet - single screen
                        if (noScreens == 1)
                            printf("%sScreen:%s   %s\n", colAccent, colReset, screen);
                        // No compact - no bullet - multiple screens - first screen
                        else if (!pastFirstScreen)
                            printf("%sScreens:%s  %s\n", colAccent, colReset, screen);
                        // No compact - no bullet - multiple screens - subsequent screens
                        else 
                            printf("          %s\n", screen);
                    }
                    else
                    {
                        // Compact - no bullet - single screen
                        if (noScreens == 1)
                            printf("%sScn:%s %s\n", colAccent, colReset, screen);
                        // Compact - no bullet - multiple screens - first screen
                        else if (!pastFirstScreen)
                            printf("%sScn:%s %s\n", colAccent, colReset, screen);
                        // Compact - no bullet - multiple screens - subsequent screens
                        else 
                            printf("     %s\n", screen);
                    }
                }
                else 
                {
                    // Compact - bullet - single or multiple screens
                    if (COMPACT)
                        printf(" %s%c%s %s\n", colAccent, bullet, colReset, screen);
                    else
                    {
                        // No compact - bullet - single screen
                        if (noScreens == 1)
                            printf(" %s%c%s %s\n", colAccent, bullet, colReset, screen);
                        // No compact - bullet - multiple screens
                        else if (!COMPACT)
                            printf(" %s%c%s %s\n", colAccent, bullet, colReset, screen);
                    }
                }
            }

            free(screen);
            pastFirstScreen = 1;
        }
    }

    if (de && de != wm && de[0] != '\0')
    {
        if (showShork) printf("%s%s%s", colAccent, SHORK[shorkLine++], colReset);
        if (!useBullets)
        {
            if (!COMPACT)
                printf("%sDE:%s       %s\n", colAccent, colReset, de);
            else
                printf("%sDE:%s  %s\n", colAccent, colReset, de);
        }
        else printf(" %s%c%s %s\n", colAccent, bullet, colReset, de);
    }

    if (wm && wm[0] != '\0')
    {
        char server[32] = "";
        if (!COMPACT)
        {
            if (WAYLAND_PRESENT)
                snprintf(server, 32, " (Wayland)");
            else if (X11_PRESENT)
                snprintf(server, 32, " (X11)");
        }

        if (showShork) printf("%s%s%s", colAccent, SHORK[shorkLine++], colReset);
        if (!useBullets)
        {
            if (!COMPACT)
                printf("%sWM:%s       %s%s\n", colAccent, colReset, wm, server);
            else
                printf("%sWM:%s  %s\n", colAccent, colReset, wm);
        }
        else 
        {
            if (!COMPACT)
                printf(" %s%c%s %s%s\n", colAccent, bullet, colReset, wm, server);
            else
                printf(" %s%c%s %s\n", colAccent, bullet, colReset, wm);
        }
    }

    if (trm && trm[0] != '\0')
    {
        if (showShork) printf("%s%s%s", colAccent, SHORK[shorkLine++], colReset);
        if (!useBullets)
        {
            if (!COMPACT)
                printf("%sTerminal:%s %s (%dx%d)\n", colAccent, colReset, trm, TERM_SIZE.ws_col, TERM_SIZE.ws_row);
            else
                printf("%sTrm:%s %s\n", colAccent, colReset, trm);
        }
        else
        {
            if (!COMPACT)
                printf(" %s%c%s %s (%dx%d)\n", colAccent, bullet, colReset, trm, TERM_SIZE.ws_col, TERM_SIZE.ws_row);
            else
                printf(" %s%c%s %s\n", colAccent, bullet, colReset, trm);
        }
    }
    // If we don't have a terminal name, we can at least still show the console 
    // size
    else if (showTrm)
    {
        if (showShork) printf("%s%s%s", colAccent, SHORK[shorkLine++], colReset);
        if (!useBullets)
        {
            if (!COMPACT)
                printf("%sConsole:%s  %dx%d\n", colAccent, colReset, TERM_SIZE.ws_col, TERM_SIZE.ws_row);
            else
                printf("%sCon:%s %dx%d\n", colAccent, colReset, TERM_SIZE.ws_col, TERM_SIZE.ws_row);
        }
        else
        {
            if (!COMPACT)
                printf(" %s%c%s %dx%d console\n", colAccent, bullet, colReset, TERM_SIZE.ws_col, TERM_SIZE.ws_row);
            else
                printf(" %s%c%s %dx%dch\n", colAccent, bullet, colReset, TERM_SIZE.ws_col, TERM_SIZE.ws_row);
        }
    }

    if (shell && shell[0] != '\0')
    {
        if (showShork) printf("%s%s%s", colAccent, SHORK[shorkLine++], colReset);
        if (!useBullets)
        {
            if (!COMPACT)
                printf("%sShell:%s    %s\n", colAccent, colReset, shell);
            else
                printf("%sSh:%s  %s\n", colAccent, colReset, shell);
        }
        else printf(" %s%c%s %s\n", colAccent, bullet, colReset, shell);
    }

    if (showCategories)
    {
        if (showShork) printf("%s%s%s", colAccent, SHORK[shorkLine++], colReset);
        for (size_t i = 0; i < headerWidth; i++) printf("-");
        putchar('\n');
    }

    if (cpu && cpu[0] != '\0')
    {
        if (showShork) printf("%s%s%s", colAccent, SHORK[shorkLine++], colReset);
        if (!useBullets)
        {
            if (!COMPACT)
                printf("%sCPU:%s      %s\n", colAccent, colReset, cpu);
            else
                printf("%sCPU:%s %s\n", colAccent, colReset, cpu);
        }
        else printf(" %s%c%s %s\n", colAccent, bullet, colReset, cpu);
    }

    if (gpus)
    {
        int pastFirstGPU = 0;
        for (int i = 0; i < noGPUs; i++)
        {
            char *gpu = interpretGPU(&gpus[i]);

            if (gpu && gpu[0] != '\0')
            {
                if (showShork) printf("%s%s%s", colAccent, SHORK[shorkLine++], colReset);
                if (!useBullets)
                {
                    if (!COMPACT)
                    {
                        if (noGPUs == 1)
                            printf("%sGPU:%s      %s\n", colAccent, colReset, gpu);
                        else if (!pastFirstGPU)
                            printf("%sGPUs:%s     %s\n", colAccent, colReset, gpu);
                        else
                            printf("          %s\n", gpu);
                    }
                    else
                    {
                        if (noGPUs == 1 || !pastFirstGPU)
                            printf("%sGPU:%s %s\n", colAccent, colReset, gpu);
                        else
                            printf("     %s\n", gpu);
                    }
                }
                else printf(" %s%c%s %s\n", colAccent, bullet, colReset, gpu);
            }

            free(gpu);
            pastFirstGPU = 1;
        }
    }

    if (ram && ram[0] != '\0')
    {
        if (showShork) printf("%s%s%s", colAccent, SHORK[shorkLine++], colReset);
        if (!useBullets)
        {
            if (!COMPACT)
                printf("%sRAM:%s      %s\n", colAccent, colReset, ram);
            else
                printf("%sRAM:%s %s\n", colAccent, colReset, ram);
        }
        else 
        {
            if (!COMPACT)
                printf(" %s%c%s %s RAM\n", colAccent, bullet, colReset, ram);
            else
                printf(" %s%c%s %s (R)\n", colAccent, bullet, colReset, ram);
        }
    }

    if (swap && swap[0] != '\0')
    {
        if (showShork) printf("%s%s%s", colAccent, SHORK[shorkLine++], colReset);
        if (!useBullets)
        {
            if (!COMPACT)
                printf("%sSwap:%s     %s\n", colAccent, colReset, swap);
            else
                printf("%sSwp:%s %s\n", colAccent, colReset, swap);
        }
        else 
        {
            if (!COMPACT)
                printf(" %s%c%s %s swap\n", colAccent, bullet, colReset, swap);
            else
                printf(" %s%c%s %s (S)\n", colAccent, bullet, colReset, swap);
        }
    }

    if (root && root[0] != '\0')
    {
        if (showShork) printf("%s%s%s", colAccent, SHORK[shorkLine++], colReset);
        if (!useBullets)
        {
            if (!COMPACT)
                printf("%sRoot:%s     %s\n", colAccent, colReset, root);
            else
                printf("%s/:%s   %s\n", colAccent, colReset, root);
        }
        else 
        {
            if (!COMPACT)
                printf(" %s%c%s %s root\n", colAccent, bullet, colReset, root);
            else
                printf(" %s%c%s %s (/)\n", colAccent, bullet, colReset, root);
        }
    }

    if (localIP)
    {
        // Since local IP is the only current field for the final category, we
        // nest this part in here
        if (showCategories)
        {
            if (showShork) printf("%s%s%s", colAccent, SHORK[shorkLine++], colReset);
            for (size_t i = 0; i < headerWidth; i++) printf("-");
            putchar('\n');
        }

        if (showShork) printf("%s%s%s", colAccent, SHORK[shorkLine++], colReset);
        if (!useBullets)
        {
            if (!COMPACT)
                printf("%sLocal IP:%s %s\n", colAccent, colReset, localIP);
            else
                printf("%sLoc:%s %s\n", colAccent, colReset, localIP);
        }
        else 
        {
            if (!COMPACT)
                printf(" %s%c%s %s local\n", colAccent, bullet, colReset, localIP);
            else
                printf(" %s%c%s %s (L)\n", colAccent, bullet, colReset, localIP);
        }
    }
    
    putchar('\n');



    free(ram);
    free(swap);
    free(os);
    free(kernel);
    free(uptime);
    free(pkgs);

    if (screens) free(screens);
    if (de != wm) free(de);
    free(wm);
    free(trm);
    free(shell);

    free(cpu);
    if (gpus) free(gpus);
    free(root);

    if (!noIP) free(localIP);

    return 0;
}
