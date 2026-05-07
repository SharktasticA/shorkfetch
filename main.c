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
    char *connector;
    int isPrimary;
    float physSize;
    int resX;
    int resY;
    int resTotal;
    int refresh;
} Display;

typedef struct {
    int vendor;
    int device;
} GPU;

typedef struct {
    long memTotal;
    long memFree;
    long buffers;
    long cached;
    long swapTotal;
    long swapFree;
} MemInfo;



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



static int COMPACT = 0;
const char SHORK[20][20] = {
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
    "                   "
};
static struct winsize TERM_SIZE;



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
 * @return ASCII escape code for colour
 */
char *getAccentColour(void)
{
#ifdef COL
    char *colour = STR(COL);
    if (strcmp(colour, "BLACK") == 0) return COL_BLACK;
    if (strcmp(colour, "BLUE") == 0) return COL_BLUE;
    if (strcmp(colour, "BOLD_BLUE") == 0) return COL_BOLD_BLUE;
    if (strcmp(colour, "BOLD_CYAN") == 0) return COL_BOLD_CYAN;
    if (strcmp(colour, "BOLD_GREEN") == 0) return COL_BOLD_GREEN;
    if (strcmp(colour, "BOLD_MAGENTA") == 0) return COL_BOLD_MAGENTA;
    if (strcmp(colour, "BOLD_RED") == 0) return COL_BOLD_RED;
    if (strcmp(colour, "BOLD_WHITE") == 0) return COL_BOLD_WHITE;
    if (strcmp(colour, "BOLD_YELLOW") == 0) return COL_BOLD_YELLOW;
    if (strcmp(colour, "CYAN") == 0) return COL_CYAN;
    if (strcmp(colour, "GREEN") == 0) return COL_GREEN;
    if (strcmp(colour, "GREY") == 0) return COL_GREY;
    if (strcmp(colour, "MAGENTA") == 0) return COL_MAGENTA;
    if (strcmp(colour, "OFF") == 0) return COL_RESET;
    if (strcmp(colour, "RED") == 0) return COL_RED;
    if (strcmp(colour, "WHITE") == 0) return COL_WHITE;
    if (strcmp(colour, "YELLOW") == 0) return COL_YELLOW;
#endif
    return COL_BOLD_CYAN;
}

/**
 * @param prog Program's executable name
 * @returns 1 if program is installed; 0 if not
 */
int isProgramInstalled(char *prog)
{
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
        char fullpath[512];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", dir, prog);
        if (access(fullpath, X_OK) == 0)
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
    if (access(libexecPath, X_OK) == 0) return 1;

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
 * Reads a single hexadecimal number for a given text file.
 * @param path Path to file to open
 * @return The read number as an integer (0 as fallback)
 */
int readHexFile(const char *path)
{
    FILE *stream = fopen(path, "r");
    if (!stream) return 0;
    int val;
    if (fscanf(stream, "%x", &val) != 1) val = 0;
    fclose(stream);
    return val;
}



/**
 * Removes predefined substrings from an input string. It's intended to
 * simplify CPU and GPU names by removing things like "(R)", "(TM)",
 * "Corporation", etc.
 * @param input Input string
 * @param inputSize Size to use when allocating the result string
 * @param coreCount Number of cores the processor has (presently only used for CPU name manipulation)
 * @return String containing what's left after cleaning
 */
char *cleanProcessorName(const char *input, size_t inputSize, int coreCount)
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
    while (strstr(result, "  "))
    {
        char *tmp = findReplace(result, inputSize, "  ", " ");
        strncpy(result, tmp, inputSize - 1);
        result[inputSize - 1] = '\0';
        free(tmp);
    }

    // Shorten " / " to "/"
    while (strstr(result, " / "))
    {
        char *tmp = findReplace(result, inputSize, " / ", "/");
        strncpy(result, tmp, inputSize - 1);
        result[inputSize - 1] = '\0';
        free(tmp);
    }

    // If applicable, handle GPU names in/around square brackets
    if (strstr(result, "[") && result[strLen - 1] == ']')
    {
        // If Cirrus Logic GPU, we actually want to discard the square brackets and their
        // contents
        if (result[0] == 'G' && result[1] == 'D')
        {
            char *openBrac = strchr(result, '[');
            if (openBrac) *openBrac = '\0';
        }
        // Extract a GPU name from square brackets - e.g. "GM204 [GeForce GTX 980]" ->
        // "GeForce GTX 980" 
        else
        {
            result[strLen - 1] = '\0';
            char *extract = extractFromPoint(result, inputSize, '[', 1);
            strncpy(result, extract, inputSize - 1);
            free(extract);
        }
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

    // Remove space between "GD" and model number for Cirrus Logic GPUs
    if (result[0] == 'G' && result[1] == 'D' && result[2] == ' ')
    {
        char *tmp = findReplace(result, inputSize, "GD ", "GD");
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
                        size_t removeLen = (needle + 5) - ordinal;
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
        // Apply compact-specific CPU and GPU name shortenings
        int replaces = 0;
        for (int i = 0; i < COMPACT_CPU_GPU_REPLACES_LEN; i++)
        {
            if (COMPACT_CPU_GPU_REPLACES[i].standalone && replaces > 0) continue;
            else if (strstr(result, COMPACT_CPU_GPU_REPLACES[i].match))
            {
                char *tmp = findReplace(result, inputSize, COMPACT_CPU_GPU_REPLACES[i].match, COMPACT_CPU_GPU_REPLACES[i].replacement);
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

    FILE *stream = fopen("/proc/meminfo", "r");
    if (stream)
    {
        char buffer[128];
        while (fgets(buffer, sizeof(buffer), stream))
        {
            sscanf(buffer, "MemTotal: %ld", &mi.memTotal);
            sscanf(buffer, "MemFree: %ld", &mi.memFree);
            sscanf(buffer, "Buffers: %ld", &mi.buffers);
            sscanf(buffer, "Cached: %ld", &mi.cached);
            sscanf(buffer, "SwapTotal: %ld", &mi.swapTotal);
            sscanf(buffer, "SwapFree: %ld", &mi.swapFree);
        }
        fclose(stream);
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
 * @param gpuIDs GPU struct containing detected vendor and device IDs
 * @return String containing the GPU's vendor and device name or "unknown" along with vendor and device IDs as hex if interpreting failed
 */
char *interpretGPU(GPU *gpuIDs)
{
    const int gpuSize = 256;
    char *gpu = malloc(gpuSize);
    if (!gpu) return strdup("unknown");
    gpu[0] = '\0';

    // If Intel GPU, query pre-defined iGPU list
    if (gpuIDs->vendor == 0x8086)
    {
        if (INTEL_IGPUS[gpuIDs->device])
        {
            snprintf(gpu, gpuSize, "Intel %s", INTEL_IGPUS[gpuIDs->device]);
            char *clean = cleanProcessorName(gpu, gpuSize, -1);
            strncpy(gpu, clean, gpuSize);
            free(clean);
            return gpu;
        }
    }
    
    char *pciids;
    if (access("/usr/share/misc/pci.ids", F_OK) == 0)
        pciids = "/usr/share/misc/pci.ids";
    else if (access("/usr/share/hwdata/pci.ids", F_OK) == 0)
        pciids = "/usr/share/hwdata/pci.ids";
    else
    {
        snprintf(gpu, gpuSize, "unknown (%04x:%04x)", gpuIDs->vendor, gpuIDs->device);
        return gpu;
    }

    FILE *stream = fopen(pciids, "r");
    if (!stream)
    {
        snprintf(gpu, gpuSize, "unknown (%04x:%04x)", gpuIDs->vendor, gpuIDs->device);
        return gpu;
    }

    char *vendor = NULL;
    char vendorHex[5];
    sprintf(vendorHex, "%04x", gpuIDs->vendor);
    char *device = NULL;
    char deviceHex[5];
    sprintf(deviceHex, "%04x", gpuIDs->device);

    char buffer[128];
    while (fgets(buffer, sizeof(buffer), stream))
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
    fclose(stream);

    if (!vendor || !device)
        snprintf(gpu, gpuSize, "unknown (%04x:%04x)", gpuIDs->vendor, gpuIDs->device);
    else
    {
        char *cleanVendor = cleanProcessorName(vendor, gpuSize / 2, -1);
        char *cleanDevice = cleanProcessorName(device, gpuSize / 2, -1);
        snprintf(gpu, gpuSize, "%s %s", cleanVendor, cleanDevice);
        free(cleanVendor);
        free(cleanDevice);
    }

    if (vendor) free(vendor);
    if (device) free(device);

    return gpu;
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

    char bullets[130] = "-b, --bullets    Uses bullet points instead of field headings; can also be used to set a custom character\n";
    formatNewLines(bullets, TERM_SIZE.ws_col, "                 ", 0);
    printf("%s", bullets);

    char compact[130] = "-c, --compact    Compacts field names (if not using bullets) and field values\n";
    formatNewLines(compact, TERM_SIZE.ws_col, "                 ", 0);
    printf("%s", compact);

    char help[70] = "-h, --help       Displays help information and exits\n";
    formatNewLines(help, TERM_SIZE.ws_col, "                 ", 0);
    printf("%s", help);

    char fields[140] = "-f, --fields     Allows you to specify which fields to show via a comma-separated list (os,krn,...)\n";
    formatNewLines(fields, TERM_SIZE.ws_col, "                 ", 0);
    printf("%s", fields);

    char noArt[100] = "-na, --no-art    Disables the SHORK ASCII art (if compiled with art support)\n";
    formatNewLines(noArt, TERM_SIZE.ws_col, "                 ", 0);
    printf("%s", noArt);

    char noCol[100] = "-nc, --no-col    Disables all coloured output (if compiled with colour support)\n\n";
    formatNewLines(noCol, TERM_SIZE.ws_col, "                 ", 0);
    printf("%s", noCol);

    char fieldNames[90] = "Field names:\nos, krn, upt, sh, scn, con, cpu, gpu, ram, swap, root and lip\n";
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
 * @return String containing the hostname; "unknown" if undetermined/error
 */
char *getHostname(void)
{
    char *hostname = NULL;
    
    FILE *stream = popen("hostname 2>/dev/null", "r");
    if (stream)
    {
        char buffer[255];
        if (fgets(buffer, sizeof(buffer), stream) != NULL)
        {
            buffer[strcspn(buffer, "\n")] = '\0';
            hostname = strdup(buffer);
        }
        pclose(stream);
    }

    if (!hostname)
    {
        stream = popen("uname -n 2>/dev/null", "r");
        if (stream)
        {
            char buffer[255];
            if (fgets(buffer, sizeof(buffer), stream) != NULL)
            {
                buffer[strcspn(buffer, "\n")] = '\0';
                hostname = strdup(buffer);
            }
            pclose(stream);
        }
    }

    if (!hostname)
    {
        FILE *stream = fopen("/etc/hostname", "r");
        if (stream)
        {
            char buffer[255];
            if (fgets(buffer, sizeof(buffer), stream))
            {
                buffer[strcspn(buffer, "\n")] = '\0';
                hostname = strdup(buffer);
            }
            fclose(stream);
        }
    }

    if (!hostname) return strdup("unknown");
    return hostname;
}

/**
 * @return String containing the OS/Linux distro's name or "unknown" if undetermined/error
 */
char *getOS(void)
{
    const int osSize = 128;
    char *os = malloc(osSize);
    if (!os) return strdup("unknown");
    os[0] = '\0';

    // Try os-release
    FILE *stream = fopen("/etc/os-release", "r");
    if (stream)
    {
        char buffer[128];
        while (fgets(buffer, sizeof(buffer), stream))
        {
            if (strncmp(buffer, "PRETTY_NAME=", 12) == 0)
            {
                char *extract = extractFromPoint(buffer, osSize, '=', 1);
                strncpy(os, extract, osSize - 1);
                free(extract);
                break;
            }
        }
        fclose(stream);
    }

    // Try issue
    if (os[0] == '\0')
    {
        FILE *stream = fopen("/etc/issue", "r");
        if (stream)
        {
            char buffer[osSize];
            if (fgets(buffer, sizeof(buffer), stream))
            {
                size_t len = strlen(buffer);
                if (len > 0 && buffer[len - 1] == '\n') buffer[len - 1] = '\0';
                char *p = strchr(buffer, '\\');
                if (p) *p = '\0';
                strncpy(os, buffer, osSize - 1);
                os[osSize - 1] = '\0';
            }
            fclose(stream);
        }
    }

    // Try uname -o
    if (os[0] == '\0')
    {
        FILE *stream = popen("uname -o 2>/dev/null", "r");
        if (stream)
        {
            char buffer[128];
            if (fgets(buffer, sizeof(buffer), stream) != NULL)
            {
                // Remove trailing newline
                buffer[strcspn(buffer, "\n")] = '\0';
                strcpy(os, buffer);
            }
            pclose(stream);
        }
    }

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
 * @return String containing the kernel version or "unknown" if undetermined/error
 */
char *getKernel(void)
{
    struct utsname u;
    if (uname(&u) == -1) return strdup("unknown");
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

    FILE *stream = fopen("/proc/uptime", "r");
    if (stream)
    {
        double seconds;
        if (fscanf(stream, "%lf", &seconds) == 1)
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
        fclose(stream);
    }
    else strcpy(uptime, "unknown");

    return uptime;
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
 * @param count Number of displays detected (intended to be used by reference)
 * @return Pointer to Display structs containing detected GPUs
 */
Display* getDisplays(int *count)
{
    if (!count) return NULL;

    const char *x11Test = getenv("DISPLAY");
    const char *wayTest = getenv("WAYLAND_DISPLAY");

    // If we don't think we're in a graphical environment, time to leave...
    if (!((x11Test && *x11Test) || (wayTest && *wayTest)))
        return NULL;

    Display *displays = NULL;

    // Try getting displays with xrandr (X11)
    if (isProgramInstalled("xrandr"))
    {
        FILE *stream = popen("xrandr 2>/dev/null", "r");
        if (stream)
        {
            // What we use to read lines of xrandr output in to
            char buffer[512];
            // Flag when we're reading a connected display
            int in = 0;

            while (fgets(buffer, 512, stream))
            {
                // Only process lines for things "connected" or inside a "connected"'s block
                char *isConnected = strstr(buffer, " connected");
                if (isConnected) 
                {
                    // Reallocate displays array to take into account new display
                    displays = realloc(displays, ((*count) + 1) * sizeof(Display));
                    memset(&displays[(*count)], 0, sizeof(Display));

                    // Parse connector name
                    char pConnector[64] = {0};
                    sscanf(buffer, "%63s", pConnector);
                    displays[*count].connector = strdup(pConnector);

                    // Replace "Virtual-" with "Virt-" if present
                    char *virtNeedle = strstr(displays[*count].connector, "Virtual-");
                    if (virtNeedle)
                    {
                        memcpy(virtNeedle, "Virt-", 5);
                        memmove(virtNeedle + 5, virtNeedle + 8, strlen(virtNeedle + 8) + 1);
                    }

                    // Flag if connector is for primary display
                    displays[*count].isPrimary = strstr(buffer, " primary ") != NULL;

                    // Parse and calculate physical size (DISABLED FOR NOW - DEPENDING ROUNDING TWEAKS)
                    displays[*count].physSize = 0.0;
                    /*char *needle = strstr(buffer, "mm x");
                    if (needle)
                    {
                        char *start = needle;
                        while (start > buffer && *(start - 1) != ' ')
                            start--;

                        int pPhysX = 0, pPhysY = 0;
                        if (sscanf(start, "%dmm x %dmm", &pPhysX, &pPhysY) == 2)
                        {
                            float diagMm = fsqrt(pPhysX * pPhysX + pPhysY * pPhysY);
                            float diagIn = (float)diagMm / 25.4f;

                            // "Prettify" the size
                            int whole = (int)diagIn;
                            float dec = diagIn - (float)whole;
                            int first = (int)(dec * 10.0f);

                            // If the first decimal is a 0, we can likely assume the marketing size was a whole
                            // number. Eg. 34.058189" -> 34"
                            if (first == 0)
                                displays[*count].physSize = (float)whole;
                            // If first decimal is not 0, then we preserve it and around to just 1dp
                            else
                                displays[*count].physSize = (float)((int)(diagIn * 10.0f + 0.5f)) / 10.0f;
                        }
                    }*/

                    // Parse resolution
                    char *needle = isConnected;
                    int pResX = 0, pResY = 0;
                    while (*needle && (*needle < '0' || *needle > '9')) needle++;
                    sscanf(needle, "%dx%d", &pResX, &pResY);

                    displays[*count].resX = pResX;
                    displays[*count].resY = pResY;
                    displays[*count].resTotal = pResX * pResY;

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
                    displays[*count].refresh = (int)(atof(start) + 0.5);

                    // At this point, we got everything we can for this display, so let's go
                    (*count)++;
                    in = 0;
                    continue;
                }
            }

            if (in) (*count)++;

            pclose(stream);
        }
    }

    // If we still have nothing, we can try DRM as an X11/Wayland agnostic fallback
    if (!displays)
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

                // Reallocate displays array to take into account new display
                displays = realloc(displays, ((*count) + 1) * sizeof(Display));
                memset(&displays[(*count)], 0, sizeof(Display));

                // Populate display data
                displays[(*count)].connector = strdup(entry->d_name);
                displays[(*count)].physSize = 0.0;
                displays[(*count)].resX = pResX;
                displays[(*count)].resY = pResY;
                displays[(*count)].refresh = 0;

                // Remove "cardX-" prefix if present
                if (strncmp(displays[(*count)].connector, "card", 4) == 0)
                    memmove(displays[(*count)].connector, displays[(*count)].connector + 6, strlen(displays[(*count)].connector + 6) + 1);

                // Replace "Virtual-" with "Virt-" if present
                char *virtNeedle = strstr(displays[*count].connector, "Virtual-");
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

    return displays;
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



    FILE *stream = fopen("/proc/cpuinfo", "r");
    if (stream)
    {
        char buffer[128];
        while (fgets(buffer, sizeof(buffer), stream))
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
            }
            else if (strncmp(buffer, "CPU implementer", 15) == 0)
            {
                char *extract = extractFromPoint(buffer, 16, ':', 2);
                strncpy(implementer, extract, 15);
                free(extract);
            }
            else if (strncmp(buffer, "model name", 10) == 0)
            {
                char *extract = extractFromPoint(buffer, 128, ':', 2);
                strncpy(model, extract, 127);
                free(extract);
            }
            else if (strncmp(buffer, "CPU architecture", 16) == 0)
            {
                char *extract = extractFromPoint(buffer, 4, ':', 2);
                strncpy(architecture, extract, 3);
                free(extract);
            }
            else if (strncmp(buffer, "cpu cores", 9) == 0)
            {
                char *extract = extractFromPoint(buffer, 4, ':', 2);
                strncpy(cores, extract, 3);
                free(extract);
            }
            else if (strncmp(buffer, "siblings", 8) == 0)
            {
                char *extract = extractFromPoint(buffer, 4, ':', 2);
                strncpy(threads, extract, 3);
                free(extract);
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
            }
        }
        fclose(stream);



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

    char *cleanedCPU = cleanProcessorName(cpu, 134, coreCount);
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

        char classPath[PATH_MAX / 3], vendorPath[PATH_MAX / 3], devicePath[PATH_MAX / 3];
        snprintf(classPath, sizeof(classPath), "%s/%s/class", "/sys/bus/pci/devices", entry->d_name);
        snprintf(vendorPath, sizeof(vendorPath), "%s/%s/vendor", "/sys/bus/pci/devices", entry->d_name);
        snprintf(devicePath, sizeof(devicePath), "%s/%s/device", "/sys/bus/pci/devices", entry->d_name);

        int class = readHexFile(classPath);
        int vendor = readHexFile(vendorPath);
        int device = readHexFile(devicePath);

        if ((class >> 16) == 0x03)
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

            gpus[*count].vendor = vendor;
            gpus[*count].device = device;
            (*count)++;
        }

        if (*count == 4) break;
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



int main(int argc, char *argv[])
{
    TERM_SIZE = getTerminalSize();

    char bullet = '*';
    char *colAccent = getAccentColour();
    int useBullets = 0;
    int noIP = 0;
    int showOS = 1;
    int showKrn = 1;
    int showUpt = 1;
    int showSh = 1;
    int showScn = 1;
    int showCon = 1;
    int showCPU = 1;
    int showGPU = 1;
    int showRAM = 1;
    int showSwap = 1;
    int showRoot = 1;
    int showLocIP = 1;
#ifdef NO_ART
    int showShork = 0;
#else
    int showShork = 1;
#endif
    int shorkLine = 0;

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
            showSh = 0;
            showScn = 0;
            showCon = 0;
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
                else if (strcmp(currTok, "sh") == 0)
                {
                    showSh = 1;
                    noFields++;
                }
                else if (strcmp(currTok, "scn") == 0)
                {
                    showScn = 1;
                    noFields++;
                }
                else if (strcmp(currTok, "con") == 0)
                {
                    showCon = 1;
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
        }
        else if ((strcmp(argv[i], "-na") == 0) || (strcmp(argv[i], "--no-art") == 0))
            showShork = 0;
        else if ((strcmp(argv[i], "-nc") == 0) || (strcmp(argv[i], "--no-col") == 0))
            colAccent = COL_RESET;
        else if ((strcmp(argv[i], "-ni") == 0) || (strcmp(argv[i], "--no-ip") == 0))
            noIP = 1;
    }



    MemInfo mi;
    if (showRAM || showSwap)
    {
        mi = getMemInfo();
        if (mi.memTotal <= 0)
        {
            printf("ERROR: could not load data from /proc/meminfo\n");
            return 1;
        }
    }



    char *username = getUsername();
    char *hostname = getHostname();
    char *os = showOS ? getOS() : NULL;
    char *kernel = showKrn ? getKernel() : NULL;
    char *uptime = showUpt ? getUptime() : NULL;
    char *shell = showSh ? getShell() : NULL;
    int noDisplays = 0;
    Display *displays = showScn ? getDisplays(&noDisplays) : NULL;
    char *cpu = showCPU ? getCPU() : NULL;
    int noGPUs = 0;
    GPU *gpus = showGPU ? getGPUs(&noGPUs) : NULL;
    char *ram = showRAM ? getRAM(mi) : NULL;
    char *swap = showSwap ? getSwap(mi) : NULL;
    char *root = showRoot ? getRoot() : NULL;
    char *localIP = (!noIP && showLocIP) ? getLocalIP() : NULL;



    if (username[0] != '\0' && hostname[0] != '\0') 
    {
        if (showShork) printf("\033[%sm%s\033[%sm", colAccent, SHORK[shorkLine++], COL_RESET);
        printf("\033[%sm%s\033[%sm@\033[%sm%s\033[%sm\n", colAccent, username, COL_RESET, colAccent, hostname, COL_RESET);
        if (showShork) printf("\033[%sm%s\033[%sm", colAccent, SHORK[shorkLine++], COL_RESET);

        size_t len = strlen(username) + 1 + strlen(hostname);
        for (size_t i = 0; i < len; i++) printf("-");
        putchar('\n');
    }

    if (os && os[0] != '\0')          
    {
        if (showShork) printf("\033[%sm%s\033[%sm", colAccent, SHORK[shorkLine++], COL_RESET);
        if (!useBullets)
        {
            if (!COMPACT)
                printf("\033[%smOS:\033[%sm      %s\n", colAccent, COL_RESET, os);
            else
                printf("\033[%smOS:\033[%sm  %s\n", colAccent, COL_RESET, os);
        }
        else printf(" \033[%sm%c\033[%sm %s\n", colAccent, bullet, COL_RESET, os);
    }

    if (kernel && kernel[0] != '\0')      
    {
        if (showShork) printf("\033[%sm%s\033[%sm", colAccent, SHORK[shorkLine++], COL_RESET);
        if (!useBullets)
        {
            if (!COMPACT)
                printf("\033[%smKernel:\033[%sm  %s\n", colAccent, COL_RESET, kernel);
            else
                printf("\033[%smKrn:\033[%sm %s\n", colAccent, COL_RESET, kernel);
        }
        else printf(" \033[%sm%c\033[%sm %s\n", colAccent, bullet, COL_RESET, kernel);
    }

    if (uptime && uptime[0] != '\0')      
    {
        if (showShork) printf("\033[%sm%s\033[%sm", colAccent, SHORK[shorkLine++], COL_RESET);
        if (!useBullets)
        {
            if (!COMPACT)
                printf("\033[%smUptime:\033[%sm  %s\n", colAccent, COL_RESET, uptime);
            else
                printf("\033[%smUp:\033[%sm  %s\n", colAccent, COL_RESET, uptime);
        }
        else printf(" \033[%sm%c\033[%sm %s\n", colAccent, bullet, COL_RESET, uptime);
    }

    if (shell && shell[0] != '\0')       
    {
        if (showShork) printf("\033[%sm%s\033[%sm", colAccent, SHORK[shorkLine++], COL_RESET);
        if (!useBullets)
        {
            if (!COMPACT)
                printf("\033[%smShell:\033[%sm   %s\n", colAccent, COL_RESET, shell);
            else
                printf("\033[%smSh:\033[%sm  %s\n", colAccent, COL_RESET, shell);
        }
        else printf(" \033[%sm%c\033[%sm %s\n", colAccent, bullet, COL_RESET, shell);
    }

    if (displays)
    {
        int pastFirstDisplay = 0;
        for (int i = 0; i < noDisplays; i++)
        {
            Display *dis = &displays[i];

            char size[32] = "";
            if (dis->physSize > 0.0)
                snprintf(size, 32, "%g\" ", dis->physSize);

            char refresh[32] = "";
            if (dis->refresh > 0)
            {
                if (COMPACT)
                    snprintf(refresh, 32, "@%d", dis->refresh);
                else
                    snprintf(refresh, 32, " @ %dHz", dis->refresh);
            }

            char connector[32] = "";
            if (dis->connector[0] != '\0')
                snprintf(connector, 32, " (%s)", dis->connector);
            free(dis->connector);

            if (showShork) printf("\033[%sm%s\033[%sm", colAccent, SHORK[shorkLine++], COL_RESET);

            if (!useBullets)
            {
                if (!COMPACT)
                {
                    // No compact - no bullet - single display
                    if (noDisplays == 1)
                        printf("\033[%smScreen:\033[%sm  %s%dx%d%s%s\n", colAccent, COL_RESET, size, dis->resX, dis->resY, refresh, connector);
                    // No compact - no bullet - multiple displays - first display
                    else if (!pastFirstDisplay)
                        printf("\033[%smScreens:\033[%sm %s%dx%d%s%s\n", colAccent, COL_RESET, size, dis->resX, dis->resY, refresh, connector);
                    // No compact - no bullet - multiple displays - subsequent displays
                    else 
                        printf("         %s%dx%d%s%s\n", size, dis->resX, dis->resY, refresh, connector);
                }
                else
                {
                    // Compact - no bullet - single display
                    if (noDisplays == 1)
                        printf("\033[%smScn:\033[%sm %s%dx%d%s\n", colAccent, COL_RESET, size, dis->resX, dis->resY, refresh);
                    // Compact - no bullet - multiple displays - first display
                    else if (!pastFirstDisplay)
                        printf("\033[%smScn:\033[%sm %s%dx%d%s\n", colAccent, COL_RESET, size, dis->resX, dis->resY, refresh);
                    // Compact - no bullet - multiple displays - subsequent displays
                    else 
                        printf("     %s%dx%d%s\n", size, dis->resX, dis->resY, refresh);
                }
            }
            else 
            {
                // Compact - bullet - single or multiple displays
                if (COMPACT)
                    printf(" \033[%sm%c\033[%sm %s%dx%d%s\n", colAccent, bullet, COL_RESET, size, dis->resX, dis->resY, refresh);
                else
                {
                    // No compact - bullet - single display
                    if (noDisplays == 1)
                        printf(" \033[%sm%c\033[%sm %s%dx%d%s%s\n", colAccent, bullet, COL_RESET, size, dis->resX, dis->resY, refresh, connector);
                    // No compact - bullet - multiple displays
                    else if (!COMPACT)
                        printf(" \033[%sm%c\033[%sm %s%dx%d%s%s\n", colAccent, bullet, COL_RESET, size, dis->resX, dis->resY, refresh, connector);
                }
            }

            pastFirstDisplay = 1;
        }
    }

    // Console
    if (showCon)
    {
        if (showShork) printf("\033[%sm%s\033[%sm", colAccent, SHORK[shorkLine++], COL_RESET);
        if (!useBullets)
        {
            if (!COMPACT)
                printf("\033[%smConsole:\033[%sm %dx%d\n", colAccent, COL_RESET, TERM_SIZE.ws_col, TERM_SIZE.ws_row);
            else
                printf("\033[%smCon:\033[%sm %dx%d\n", colAccent, COL_RESET, TERM_SIZE.ws_col, TERM_SIZE.ws_row);
        }
        else
        {
            if (!COMPACT)
                printf(" \033[%sm%c\033[%sm %dx%d console\n", colAccent, bullet, COL_RESET, TERM_SIZE.ws_col, TERM_SIZE.ws_row);
            else
                printf(" \033[%sm%c\033[%sm %dx%dch\n", colAccent, bullet, COL_RESET, TERM_SIZE.ws_col, TERM_SIZE.ws_row);
        }
    }

    if (cpu && cpu[0] != '\0')         
    {
        if (showShork) printf("\033[%sm%s\033[%sm", colAccent, SHORK[shorkLine++], COL_RESET);
        if (!useBullets)
        {
            if (!COMPACT)
                printf("\033[%smCPU:\033[%sm     %s\n", colAccent, COL_RESET, cpu);
            else
                printf("\033[%smCPU:\033[%sm %s\n", colAccent, COL_RESET, cpu);
        }
        else printf(" \033[%sm%c\033[%sm %s\n", colAccent, bullet, COL_RESET, cpu);
    }

    if (gpus)
    {
        int pastFirstGPU = 0;
        for (int i = 0; i < noGPUs; i++)
        {
            char *gpu = interpretGPU(&gpus[i]);

            if (gpu[0] != '\0')     
            {
                if (showShork) printf("\033[%sm%s\033[%sm", colAccent, SHORK[shorkLine++], COL_RESET);
                if (!useBullets)
                {
                    if (!COMPACT)
                    {
                        if (noGPUs == 1)
                            printf("\033[%smGPU:\033[%sm     %s\n", colAccent, COL_RESET, gpu);
                        else if (!pastFirstGPU)
                            printf("\033[%smGPUs:\033[%sm    %s\n", colAccent, COL_RESET, gpu);
                        else
                            printf("         %s\n", gpu);
                    }
                    else
                    {
                        if (noGPUs == 1 || !pastFirstGPU)
                            printf("\033[%smGPU:\033[%sm %s\n", colAccent, COL_RESET, gpu);
                        else
                            printf("     %s\n", gpu);
                    }
                }
                else printf(" \033[%sm%c\033[%sm %s\n", colAccent, bullet, COL_RESET, gpu);
            }

            free(gpu);
            pastFirstGPU = 1;
        }
    }

    if (ram && ram[0] != '\0')         
    {
        if (showShork) printf("\033[%sm%s\033[%sm", colAccent, SHORK[shorkLine++], COL_RESET);
        if (!useBullets)
        {
            if (!COMPACT)
                printf("\033[%smRAM:\033[%sm     %s\n", colAccent, COL_RESET, ram);
            else
                printf("\033[%smRAM:\033[%sm %s\n", colAccent, COL_RESET, ram);
        }
        else 
        {
            if (!COMPACT)
                printf(" \033[%sm%c\033[%sm %s RAM\n", colAccent, bullet, COL_RESET, ram);
            else
                printf(" \033[%sm%c\033[%sm %s (R)\n", colAccent, bullet, COL_RESET, ram);
        }
    }

    if (swap && swap[0] != '\0')        
    {
        if (showShork) printf("\033[%sm%s\033[%sm", colAccent, SHORK[shorkLine++], COL_RESET);
        if (!useBullets)
        {
            if (!COMPACT)
                printf("\033[%smSwap:\033[%sm    %s\n", colAccent, COL_RESET, swap);
            else
                printf("\033[%smSwp:\033[%sm %s\n", colAccent, COL_RESET, swap);
        }
        else 
        {
            if (!COMPACT)
                printf(" \033[%sm%c\033[%sm %s swap\n", colAccent, bullet, COL_RESET, swap);
            else
                printf(" \033[%sm%c\033[%sm %s (S)\n", colAccent, bullet, COL_RESET, swap);
        }
    }

    if (root && root[0] != '\0')        
    {
        if (showShork) printf("\033[%sm%s\033[%sm", colAccent, SHORK[shorkLine++], COL_RESET);
        if (!useBullets)
        {
            if (!COMPACT)
                printf("\033[%smRoot:\033[%sm    %s\n", colAccent, COL_RESET, root);
            else
                printf("\033[%sm/:\033[%sm   %s\n", colAccent, COL_RESET, root);
        }
        else 
        {
            if (!COMPACT)
                printf(" \033[%sm%c\033[%sm %s root\n", colAccent, bullet, COL_RESET, root);
            else
                printf(" \033[%sm%c\033[%sm %s (/)\n", colAccent, bullet, COL_RESET, root);
        }
    }

    if (localIP)
    {
        if (showShork) printf("\033[%sm%s\033[%sm", colAccent, SHORK[shorkLine++], COL_RESET);
        if (!useBullets)
        {
            if (!COMPACT)
                printf("\033[%smLocal:\033[%sm   %s\n", colAccent, COL_RESET, localIP);
            else
                printf("\033[%smLoc:\033[%sm %s\n", colAccent, COL_RESET, localIP);
        }
        else 
        {
            if (!COMPACT)
                printf(" \033[%sm%c\033[%sm %s local\n", colAccent, bullet, COL_RESET, localIP);
            else
                printf(" \033[%sm%c\033[%sm %s (L)\n", colAccent, bullet, COL_RESET, localIP);
        }
    }
    
    putchar('\n');



    free(ram);
    free(swap);
    free(os);
    free(kernel);
    free(uptime);
    free(shell);
    if (displays) free(displays);
    free(cpu);
    if (gpus) free(gpus);
    free(root);
    if (!noIP) free(localIP);

    return 0;
}
