/*
    ######################################################
    ##                  SHORK UTILITY                   ##
    ######################################################
    ## General, utility functions for SHORK Utilities & ##
    ## SHORK ENTERTAINMENT                              ##
    ######################################################
    ## Licence: GNU GENERAL PUBLIC LICENSE Version 3    ##
    ######################################################
    ## Kali (links.sharktastica.co.uk)                  ##
    ######################################################
*/



#include "general.h"
#include "globals.h"

#include <ctype.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <linux/limits.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>



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
 * Captures the output of the given program command capped at the given buffer
 * size.
 * @param command Program command to run
 * @param bufferSize The maximum string size
 * @return char* Captured program output as string
 */
char *captureProgramOutput(const char *command, const size_t bufferSize)
{
    FILE* stream = popen(command, "r");
    if (!stream)
        return NULL;

    char *buffer = malloc(bufferSize);
    if (!buffer)
    {
        pclose(stream);
        return NULL;
    }

    size_t len = 0;
    size_t chunkSize = 256;
    char tmp[256];

    while (len < bufferSize - 1)
    {
        size_t toRead = (bufferSize - 1 - len < chunkSize) ? bufferSize - 1 - len : chunkSize;

        size_t bytesRead = fread(tmp, 1, toRead, stream);
        if (bytesRead == 0)
            break;

        memcpy(buffer + len, tmp, bytesRead);
        len += bytesRead;
    }
    pclose(stream);

    buffer[len] = '\0';
    return buffer;
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
char *extractFromPoint(char *input, size_t inputSize, char point)
{
    if (!input || inputSize < 2) return strdup("");

    // Prepare result string
    char *result = malloc(inputSize);
    if (!result) return strdup("");
    result[0] = '\0';

    // Find our separation point in the input string
    char *sep = strchr(input, point);
    if (!sep) return result;

    // Make our start position take into account any possible offset
    char *start = sep + 1;
    while (*start == ' ' || *start == '\t')
        start++;

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
 * Checks if a file exists or not.
 * @param file Full path to file
 * @return 1 if file found; 0 if not or error
 */
int fileExists(const char *file)
{
    if (!file || file[0] == '\0')
        return 0;

    struct stat st;
    if (stat(file, &st) == 0 && S_ISREG(st.st_mode))
        return 1;
    return 0;
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
float fSqrt(float x)
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
 * Returns the full path to the directory this program is stored in.
 * @return Full path the binary is stored in
 */
char *getBinDir(void)
{
    // Get binary's full path
    static char binDir[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", binDir, sizeof(binDir) - 1);
    if (len <= 0) return NULL;
    binDir[len] = '\0';

    // Remove filename from path
    char *slash = strrchr(binDir, '/');
    if (!slash) return NULL;
    *(slash + 1) = '\0';

    return binDir;
}

/**
 * Gets the parent process ID (PPID) and name of a given process ID (PID).
 * @param pid The input PID
 * @return PROCESS struct with the found PPID and name; pid is -1 if something went wrong
 */
PROCESS getParentProcess(int pid)
{
    PROCESS result = { -1, "" };

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
 * @param currPath Current working directory path
 * @param entry Directory entry to check
 */
int isFileExecutable(char *currPath, struct dirent *entry)
{
    char filePath[PATH_MAX + 256];
    snprintf(filePath, PATH_MAX + 256, "%s/%s", currPath, entry->d_name);
    if (access(filePath, X_OK) == 0) return 1;
    else return 0;
}

/**
 * Checks if the given string is entirely numeric.
 * @param str Input string to test
 * @return 1 if numeric; 0 if not or empty string
 */
int isNumeric(const char *str)
{
    int numeric = 0;
    while (*str)
    {
        // Skip over any whitespace
        if (isspace((unsigned char)*str))
        {
            str++;
            continue;
        }

        if (!isdigit((unsigned char)*str))
            return 0;

        numeric = 1;
        str++;
    }
    return numeric;
}

/**
 * @param prog Program's executable name or full path
 * @param isExec Flags if the function should also check if a found program has
 *               execute permissions
 * @returns 1 if program is installed; 0 if not
 */
int isProgramInstalled(char *prog, int isExec)
{
    int mode = isExec ? X_OK : F_OK;

    // If prog contains '/' treat it as a full path
    if (strchr(prog, '/') != NULL)
        return (access(prog, mode) == 0);

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
int iSqrt(int x)
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
 * Reduces a given string to the given maximum lines by counting '\n' escape
 * codes.
 * @param str Input string
 * @param maxLines Number of lines before cutting the string
 */
void limitLines(char* str, const int maxLines)
{
    if (!str || maxLines <= 0)
    {
        if (str)
            str[0] = '\0';
        return;
    }

    int count = 0;
    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] == '\n')
        {
            count++;
            if (count == maxLines)
            {
                str[i + 1] = '\0';
                return;
            }
        }
    }
}

/**
 * Parses a line taken from a CSV list into separate fields.
 * @param line Raw line from CSV file to be processed
 * @param out Array of separated fields
 * @param maxFields Max number of fields to look for
 * @return Number of fields detected
 */
int loadCSVLine(char *line, char *out[], int maxFields)
{
    int i = 0;

    while (*line && i < maxFields)
    {
        out[i++] = line;
        int inQuotes = 0;

        while (*line)
        {
            if (*line == '"')
            {
                inQuotes = !inQuotes;
                if (line[1] == '"') line++;
            }
            else if (*line == ',' && !inQuotes)
            {
                *line = '\0';
                line++;
                break;
            }

            line++;
        }
    }

    return i;
}

int natCmp(const void *a, const void *b)
{
    const char *s1 = (const char *)a;
    const char *s2 = (const char *)b;

    while (*s1 && *s2)
    {
        if (isdigit((unsigned char)*s1) && isdigit((unsigned char)*s2))
        {
            char *end1, *end2;
            long n1 = strtol(s1, &end1, 10);
            long n2 = strtol(s2, &end2, 10);
            if (n1 != n2) return (n1 > n2) - (n1 < n2);
            s1 = end1;
            s2 = end2;
        }
        else
        {
            int c1 = tolower((unsigned char)*s1);
            int c2 = tolower((unsigned char)*s2);
            if (c1 != c2) return c1 - c2;
            s1++;
            s2++;
        }
    }

    return *s1 - *s2;
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
 * Removes any bracketed/parenthesis contents from a given input string.
 * @param input Input string
 * @param inputSize Size to use when allocating the result string
 * @return Result string after operation
 */
char *removeBrackets(const char *input, const size_t inputSize)
{
    if (!input)
        return NULL;

    // In case the input size happens to be too small...
    size_t inputLen = strlen(input);
    if (inputLen == 0)
        return NULL;
    size_t allocSize = inputSize;
    if (allocSize < inputLen + 1)
        allocSize = inputLen + 1;

    // Prepare result string
    char *result = calloc(allocSize, 1);
    if (!result)
        return NULL;

    const char *src = input;
    char *dst = result;
    int depth = 0;
    while (*src)
    {
        if (*src == '(')
        {
            depth++;
            src++;
        }
        else if (*src == ')')
        {
            if (depth > 0)
                depth--;
            src++;
        }
        else if (depth == 0)
            *dst++ = *src++;
        else
            src++;
    }
    *dst = '\0';

    return result;
}

/**
 * Splits a given string via any newline escape sequences into an array of strings.
 * @param text Text to split
 * @param textLines Text once split
 * @param totalLines Number of newlines detected
 */
void splitText(char *text, char *textLines[], int totalLines)
{
    int count = 0;
    char *curr = text;
    char *start = text;

    while (*curr && count < totalLines)
    {
        if (*curr == '\n')
        {
            *curr = '\0';
            textLines[count++] = start;
            start = curr + 1;
        }
        curr++;
    }

    if (count < totalLines)
        textLines[count++] = start;
}

/**
 * Word-wraps a given string based on the requested width, optionalling adding
 * indents to the start of each newly-made line.
 * @param input Input string
 * @param width Number of characters per line
 * @param indent Indent to include after a wrap
 * @param hardBreak Flags if the function should hard-break words if needed
 * @param trim Flags that any trailing newlines should be removed
 * @return Malloc'd WORD_WRAPPED struct containing the result string, how long
 *         it is & how many lines it has
 * TODO: soft-wrapping should also include breaking after '.', ',', '-' or '_'
 */
WORD_WRAPPED *wordWrap(char *input, int width, char *indent, int hardBreak, int trim)
{
    if (!input || width < 1)
        return NULL;
    
    // Initialse variables that help us track progress
    size_t inputStrLen = strlen(input);
    size_t indentLen = indent ? strlen(indent) : 0;
    // Count of lines found in the ouptu
    int lines = 1;
    // Index of the most recent breakable character
    int lastBreakPos = -1;
    // The character lastBreakPos points to
    char lastBreakChar = '\0';
    // The current size of the line being processed 
    int widthCount = 1;

    // Allocate a buffer for the result string that we can grow if needed
    size_t capacity = inputStrLen + 1;
    char *result = malloc(capacity);
    if (!result)
        return NULL;
    memcpy(result, input, inputStrLen + 1);

    // Iterate through the input string to find line breaks or places to add new ones
    for (int i = 0; i < inputStrLen; i++)
    {
        // Skip counting ANSI escape sequences
        if (result[i] == '\033')
        {
            while (i < inputStrLen && result[i] != 'm')
                i++;
            if (i >= inputStrLen)
                break;
            continue;
        }

        // If a newline is already in the string, handle it and advance to next
        // iteration
        if (result[i] == '\n')
        {
            lines++;
            widthCount = 0;
            lastBreakPos = -1;
            lastBreakChar = '\0';
            continue;
        }

        // If we have an indent, add some grace in case it's being used to
        // skip over a field heading (etc.)
        if (indentLen > 0 && widthCount < indentLen)
        {
            widthCount++;
            continue;
        }
        
        // Begin word wrapping once the line width is saturated
        if (widthCount >= width)
        {
            int breaking = 0;

            // Preferred case: make a soft wrap at lastBreak
            if (lastBreakPos != -1)
                breaking = 1;
            // Fallback case: if hardBreak=1, wrap immediately
            else if (hardBreak && result[i] != '\n' && result[i] != ' ')
                breaking = 1;

            if (breaking)
            {
                int breakPos;

                // If lastBreak is a space, we can just overwrite it with '\n'
                if (lastBreakPos != -1 && lastBreakChar == ' ')
                {
                    breakPos = lastBreakPos;
                    result[breakPos] = '\n';
                }
                // If lastBreak is not a space, we need to make space instead
                // of overwrite lastBreak's current character
                else
                {
                    // Where to insert
                    int insertAt = (lastBreakPos != -1) ? lastBreakPos + 1 : i;

                    // Make sure there is room for new newline char
                    size_t needed = inputStrLen + 1 + 1;
                    if (needed > capacity)
                    {
                        size_t newCapacity = capacity ? capacity * 2 : 16;
                        while (newCapacity < needed)
                            newCapacity *= 2;

                        char *newResult = realloc(result, newCapacity);
                        if (!newResult) { free(result); return NULL; }
                        result = newResult;
                        capacity = newCapacity;
                    }

                    memmove(result + insertAt + 1, result + insertAt, inputStrLen - insertAt + 1);
                    inputStrLen++;
                    result[insertAt] = '\n';
                    i = insertAt + 1;
                    breakPos = insertAt;
                }

                lines++;

                // If indent is desired, time to add that to the start of the line
                if (indent && indentLen > 0)
                {
                    size_t needed2 = inputStrLen + indentLen + 1;
                    if (needed2 > capacity)
                    {
                        // Make sure there is room for the extra length needed for
                        // inserting an indent
                        size_t newCapacity = capacity ? capacity * 2 : 16;
                        while (newCapacity < needed2)
                            newCapacity *= 2;

                        char *newResult = realloc(result, newCapacity);
                        if (!newResult)
                        {
                            free(result);
                            return NULL;
                        }
                        result = newResult;
                        capacity = newCapacity;
                    }

                    memmove(result + breakPos + 1 + indentLen, result + breakPos + 1, inputStrLen - breakPos);
                    memcpy(result + breakPos + 1, indent, indentLen);
                    inputStrLen += indentLen;
                    if (breakPos <= i)
                        i += indentLen;
                }

                widthCount = i - breakPos;
                lastBreakPos = -1;
                lastBreakChar = '\0';
            }
        }

        // Track where the last break char was so we can go back to wrap from it
        for (int j = 0; j < BREAK_CHARS_LEN - 1; j++)
        {
            if (result[i] == BREAK_CHARS[j])
            {
                lastBreakPos = i;
                lastBreakChar = BREAK_CHARS[j];
                break;
            }
        }

        widthCount++;
    }

    // If desired, strip possible trailing new line
    if (trim)
    {
        int end = strlen(result) - 1;
        while (end >= 0 && result[end] == '\n')
        {
            result[end] = '\0';
            end--;
            lines--;
        }
        inputStrLen = strlen(result);
    }

    WORD_WRAPPED *wrapped = malloc(sizeof(WORD_WRAPPED));
    if (!wrapped)
    {
        free(result);
        return NULL;
    }

    wrapped->str = result;
    wrapped->len = inputStrLen;
    wrapped->lines = lines;

    return wrapped;
}
