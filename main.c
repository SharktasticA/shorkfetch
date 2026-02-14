/*
    ######################################################
    ## A SHORK utility for displaying basic system and  ##
    ## environment information in a summarised format,  ##
    ## similar to fastfetch, neofetch, etc.             ##
    ######################################################
    ## Licence: GNU GENERAL PUBLIC LICENSE Version 3    ##
    ######################################################
    ## Kali (links.sharktastica.co.uk)                  ##
    ######################################################
*/



#include <dirent.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/statvfs.h>
#include <sys/utsname.h>
#include <unistd.h>



typedef struct {
    int vendor;
    int device;
} GPU;



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

    char *result = malloc(32);
    if (!result) return strdup("");
    long long whole, remainder;
    int decimal;

    if (bytes >= TiB)
    {
        whole = bytes / TiB;
        remainder = bytes % TiB;
        decimal = (int)((remainder * 10 + TiB / 2) / TiB);
        if (decimal == 10) { whole++; decimal = 0; }
        if (decimal == 0) snprintf(result, 32, "%lldTiB", whole);
        else snprintf(result, 32, "%lld.%dTiB", whole, decimal);
    }
    else if (bytes >= GiB)
    {
        whole = bytes / GiB;
        remainder = bytes % GiB;
        decimal = (int)((remainder * 10 + GiB / 2) / GiB);
        if (decimal == 10) { whole++; decimal = 0; }
        if (decimal == 0) snprintf(result, 32, "%lldGiB", whole);
        else snprintf(result, 32, "%lld.%dGiB", whole, decimal);
    }
    else if (bytes >= MiB)
    {
        whole = bytes / MiB;
        remainder = bytes % MiB;
        decimal = (int)((remainder * 10 + MiB / 2) / MiB);
        if (decimal == 10) { whole++; decimal = 0; }
        if (decimal == 0) snprintf(result, 32, "%lldMiB", whole);
        else snprintf(result, 32, "%lld.%dMiB", whole, decimal);
    }
    else if (bytes >= KiB)
    {
        whole = bytes / KiB;
        remainder = bytes % KiB;
        decimal = (int)((remainder * 10 + KiB / 2) / KiB);
        if (decimal == 10) { whole++; decimal = 0; }
        if (decimal == 0) snprintf(result, 32, "%lldKiB", whole);
        else snprintf(result, 32, "%lld.%dKiB", whole, decimal);
    }
    else
        snprintf(result, 32, "%lldB", bytes);

    return result;
}

/**
 * Extracts a substring from an input string after a given separation character
 * and offset. Also removes any surrounding quotes or trailing newline
 * characters present. 
 * @param buffer Input string
 * @param point Character to find to separate from (e.g., '=' or ':')
 * @param offset How many characters after the point to separate at
 * @param outputSize Size to use when allocating the result string
 * @return String containing what's left after separation and cleaning
 */
char *extractFromPoint(char *buffer, char point, int offset, int outputSize)
{
    if (!buffer || outputSize < 2) return strdup("");

    char *result = malloc(outputSize);
    if (!result) return strdup("");
    result[0] = '\0';

    char *sep = strchr(buffer, point);
    if (!sep) return result;

    char *start = sep + offset;

    // Trim potential leading quote
    if (*start == '"') start++;

    strncpy(result, start, outputSize - 1);
    result[outputSize - 1] = '\0';
    size_t len = strlen(result);

    // Trim potential trailing newline 
    if (len > 0 && result[len - 1] == '\n')
        result[--len] = '\0';

    // Trim potential trailing quote
    if (len > 0 && result[len - 1] == '"')
        result[len - 1] = '\0';

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
 * @return String containing a combined user name and hostname or "unknown" if undetermined/error
 */
char *getHeader(void)
{
    char *username = getenv("USER");
    if (!username || username[0] == '\0')
        username = getenv("LOGNAME");
    if (!username || username[0] == '\0') 
        username = "unknown";

    char *hostname;
    FILE *stream = popen("hostname 2>/dev/null", "r");
    if (stream)
    {
        char buffer[64];
        if (fgets(buffer, sizeof(buffer), stream) != NULL)
        {
            buffer[strcspn(buffer, "\n")] = '\0';
            hostname = strdup(buffer);
        }
        else hostname = strdup("unknown");
        pclose(stream);
    }
    else hostname = strdup("unknown");
    
    size_t len = strlen(username) + 1 + strlen(hostname) + 1;
    char *header = malloc(len);
    snprintf(header, len, "%s@%s", username, hostname);
    free(hostname);
    return header;
}

/**
 * @return String containing the OS/Linux distro's name or "unknown" if undetermined/error
 */
char *getOS(void)
{
    char *os = malloc(128);
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
                char *extract = extractFromPoint(buffer, '=', 1, 128);
                strncpy(os, extract, 127);
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
            char buffer[128];
            if (fgets(buffer, sizeof(buffer), stream))
            {
                size_t len = strlen(buffer);
                if (len > 0 && buffer[len - 1] == '\n') buffer[len - 1] = '\0';
                char *p = strchr(buffer, '\\');
                if (p) *p = '\0';
                strncpy(os, buffer, 127);
                os[127] = '\0';
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

    return os;
}

/**
 * @return String containing the kernel version or "unknown" if undetermined/error
 */
char *getKernel(void)
{
    struct utsname u;
    if (uname(&u) == -1) return strdup("unknown");
    char *release = malloc(strlen(u.release) + 1);
    if (!release) return strdup("unknown");
    strcpy(release, u.release);
    return release; 
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

            const char *dayUnit = (days == 1) ? "day" : "days";
            const char *hourUnit = (hours == 1) ? "hour" : "hours";
            const char *minUnit = (minutes == 1) ? "minute" : "minutes";

            if (days > 0)
                snprintf(uptime, 128, "%d %s, %d %s, %d %s", days, dayUnit, hours, hourUnit, minutes, minUnit);
            else
                snprintf(uptime, 128, "%d %s, %d %s", hours, hourUnit, minutes, minUnit);
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
 * @return String containing the CPU's name and core/thread specs or "unknown" if undetermined/error
 */
char *getCPU(void)
{
    char *cpu = malloc(134);
    char *name = malloc(128);
    char *processor = malloc(4);
    char *cores = malloc(4);
    char *threads = malloc(4);
    if (!cpu || !name || !processor || !cores || !threads) 
    {
        free(cpu);
        free(name);
        free(processor);
        free(cores);
        free(threads);
        return strdup("unknown");
    }
    cpu[0] = name[0] = processor[0] = cores[0] = threads[0] = '\0';

    FILE *stream = fopen("/proc/cpuinfo", "r");
    if (stream)
    {
        char buffer[128];
        while (fgets(buffer, sizeof(buffer), stream))
        {
            if (strncmp(buffer, "processor", 9) == 0)
            {
                char *extract = extractFromPoint(buffer, ':', 2, 4);
                strncpy(processor, extract, 3);
                free(extract);
            }
            else if (strncmp(buffer, "model name", 10) == 0)
            {
                char *extract = extractFromPoint(buffer, ':', 2, 128);
                strncpy(name, extract, 127);
                free(extract);
            }
            else if (strncmp(buffer, "cpu cores", 9) == 0)
            {
                char *extract = extractFromPoint(buffer, ':', 2, 4);
                strncpy(cores, extract, 3);
                free(extract);
            }
            else if (strncmp(buffer, "siblings", 8) == 0)
            {
                char *extract = extractFromPoint(buffer, ':', 2, 4);
                strncpy(threads, extract, 3);
                free(extract);
            }
            
            if (name[0] != '\0' && cores[0] != '\0' && threads[0] != '\0')
                break;
        }
        fclose(stream);

        if (cores[0] == '\0' && threads[0] != '\0')
            strncpy(cores, threads, 3);
            
        if (cores[0] == '\0' && threads[0] == '\0')
        {
            int processorInt = atoi(processor);
            processorInt++;
            snprintf(cpu, 134, "%s (%dC)", name, processorInt);
        }
        else if (strcmp(cores, threads) == 0)
            snprintf(cpu, 134, "%s (%sC)", name, cores);
        else
            snprintf(cpu, 134, "%s (%sC/%sT)", name, cores, threads);
    }
    else strcpy(cpu, "unknown");

    free(name);
    free(processor);
    free(cores);
    free(threads);

    return cpu;
}

/**
 * @param count Number of GPUs actually detected (intended to be used by reference)
 * @return Pointer to up to 4 GPU structs containing detected GPUs
 */
GPU* getGPUs(int *count)
{
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

        char classPath[512], vendorPath[512], devicePath[512];
        snprintf(classPath, sizeof(classPath), "%s/%s/class", "/sys/bus/pci/devices", entry->d_name);
        snprintf(vendorPath, sizeof(vendorPath), "%s/%s/vendor", "/sys/bus/pci/devices", entry->d_name);
        snprintf(devicePath, sizeof(devicePath), "%s/%s/device", "/sys/bus/pci/devices", entry->d_name);

        int class = readHexFile(classPath);
        int vendor = readHexFile(vendorPath);
        int device = readHexFile(devicePath);

        if ((class >> 16) == 0x03)
        {
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
 * @param gpuIDs Pointer to up to 4 GPU structs containing detected GPUs
 * @return String containing the GPU's vendor and device name or "unknown" along with vendor and device IDs as hex if interpreting failed
 */
char *interpretGPU(GPU *gpuIDs)
{
    char *gpu = malloc(257);
    if (!gpu) return strdup("unknown");
    gpu[0] = '\0';
    
    char *pciids;
    if (access("/usr/share/misc/pci.ids", F_OK) == 0)
        pciids = "/usr/share/misc/pci.ids";
    else if (access("/usr/share/hwdata/pci.ids", F_OK) == 0)
        pciids = "/usr/share/hwdata/pci.ids";
    else
    {
        snprintf(gpu, 257, "unknown (%04x:%04x)", gpuIDs->vendor, gpuIDs->device);
        return gpu;
    }

    FILE *stream = fopen(pciids, "r");
    if (!stream)
    {
        snprintf(gpu, 257, "unknown (%04x:%04x)", gpuIDs->vendor, gpuIDs->device);
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
        snprintf(gpu, 257, "unknown (%04x:%04x)", gpuIDs->vendor, gpuIDs->device);
    else
        snprintf(gpu, 257, "%s %s", vendor, device);

    if (vendor) free(vendor);
    if (device) free(device);

    return gpu;
}

/**
 * @return String containing the system memory used and total amounts both numerically and as a percentage
 */
char *getRAM(void)
{
    char *ram = malloc(64);
    if (!ram) return strdup("");
    ram[0] = '\0';

    FILE *stream = fopen("/proc/meminfo", "r");
    if (stream)
    {
        char buffer[128];
        long total = 0, freeMem = 0, buffers = 0, cached = 0;
        while (fgets(buffer, sizeof(buffer), stream))
        {
            if (sscanf(buffer, "MemTotal: %ld kB", &total) == 1) continue;
            if (sscanf(buffer, "MemFree: %ld kB", &freeMem) == 1) continue;
            if (sscanf(buffer, "Buffers: %ld kB", &buffers) == 1) continue;
            if (sscanf(buffer, "Cached: %ld kB", &cached) == 1) continue;
            if (total != 0 && freeMem != 0 && buffers != 0 && cached != 0) break;
        }
        fclose(stream);

        freeMem = freeMem + buffers + cached;
        long used = total - freeMem;
        int pct = total ? (int)((used * 100) / total) : 0;

        char *usedStr = bytesToReadable("KiB", used);
        char *totalStr = bytesToReadable("KiB", total);
        snprintf(ram, 64, "%s / %s (%d%%)", usedStr, totalStr, pct);
        free(usedStr);
        free(totalStr);
    }

    return ram;
}

/**
 * @return String containing the system swap used and total amounts both numerically and as a percentage
 */
char *getSwap(void)
{
    char *swap = malloc(64);
    if (!swap) return strdup("");
    swap[0] = '\0';

    FILE *stream = fopen("/proc/meminfo", "r");
    if (stream)
    {
        char buffer[128];
        long total = 0, freeSwap = 0;
        while (fgets(buffer, sizeof(buffer), stream))
        {
            if (sscanf(buffer, "SwapTotal: %ld kB", &total) == 1) continue;
            if (sscanf(buffer, "SwapFree: %ld kB", &freeSwap) == 1) continue;
            if (total != 0 && freeSwap != 0) break;
        }
        fclose(stream);

        long used = total - freeSwap;
        int pct = total ? (int)((used * 100) / total) : 0;

        char *usedStr = bytesToReadable("KiB", used);
        char *totalStr = bytesToReadable("KiB", total);
        snprintf(swap, 64, "%s / %s (%d%%)", usedStr, totalStr, pct);
        free(usedStr);
        free(totalStr);
    }

    return swap;
}

/**
 * @return String containing the root partition's used and total size amounts both numerically and as a percentage
 */
char *getRoot(void)
{
    char *root = malloc(64);
    if (!root) return strdup("");
    root[0] = '\0';

    struct statvfs fs;

    if (statvfs("/", &fs) != 0)
        return root;

    long long total = (long long)fs.f_blocks * fs.f_frsize;
    long long freeRoot  = (long long)fs.f_bfree * fs.f_frsize;
    long long used  = total - freeRoot;
    int pct = total ? (int)((used * 100) / total) : 0;

    char *usedStr = bytesToReadable("B", used);
    char *totalStr = bytesToReadable("B", total);
    snprintf(root, 64, "%s / %s (%d%%)", usedStr, totalStr, pct);
    free(usedStr);
    free(totalStr);

    return root;
}



int main(void)
{
    char *ram = getRAM();
    char *swap = getSwap();
    char *header = getHeader();
    char *os = getOS();
    char *kernel = getKernel();
    char *uptime = getUptime();
    char *shell = getShell();
    int noGPUs = 0;
    GPU *gpus = getGPUs(&noGPUs);
    char *cpu = getCPU();
    char *root = getRoot();

    if (header[0] != '\0') 
    {
                                printf("%s\n", header);
        size_t len = strlen(header);
        for (size_t i = 0; i < len; i++) printf("-");
        printf("\n");
    }

    if (os[0] != '\0')          printf("OS:      %s\n", os);
    if (kernel[0] != '\0')      printf("Kernel:  %s\n", kernel);
    if (uptime[0] != '\0')      printf("Uptime:  %s\n", uptime);
    if (shell[0] != '\0')       printf("Shell:   %s\n", shell);
    if (cpu[0] != '\0')         printf("CPU:     %s\n", cpu);

    if (gpus)
    {
        for (int i = 0; i < noGPUs; i++)
        {
            char *gpu = interpretGPU(&gpus[i]);
            if (gpu[0] != '\0')     printf("GPU:     %s\n", gpu);
            free(gpu);
        }
    }

    if (ram[0] != '\0')         printf("Memory:  %s\n", ram);
    if (swap[0] != '\0')        printf("Swap:    %s\n", swap);
    if (root[0] != '\0')        printf("Root:    %s\n", root);
    
    printf("\n");

    free(ram);
    free(swap);
    free(header);
    free(os);
    free(kernel);
    free(uptime);
    free(cpu);
    free(gpus);
    free(root);

    return 0;
}
