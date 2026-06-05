/*
    ######################################################
    ##            SHORK UTILITY - SHORKFETCH            ##
    ######################################################
    ## Functions and data relating to handling CPUs     ##
    ######################################################
    ## Licence: GNU GENERAL PUBLIC LICENSE Version 3    ##
    ######################################################
    ## Kali (links.sharktastica.co.uk)                  ##
    ######################################################
*/



/*
    OPTIMISATION TODO
        * Intel
            * Arrandale embedded (Core iX-XXXE/LE/UE)
*/



#include "cpu.h"
#include "general.h"
#include "globals.h"
#include "gpu.h"
#include "replacements.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>



/**
 * Cleans a CPU's name so it is less needlessly verbose and 'to the point'.
 * @param input Input string
 * @param inputSize Size to use when allocating the result string
 * @return String containing the result after cleaning
 */
char *cleanCPUName(const char *input, size_t inputSize)
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
    char *hz = strstr(result, "Hz");
    if (hz)
    {
        // Walk back from "Hz" to find the beginning of the frequency substring
        char *at = hz;
        if (at > result && isalpha(*(at-1)))
            at--;
        while (at > result && (isdigit(*(at-1)) || *(at-1) == '.'))
            at--;

        // Take into account possible " @ " or " " in front
        if (at > result && *(at-1) == ' ')
        {
            at--;
            if (at > result && *(at-1) == '@')
                at--;
            if (at > result && *(at-1) == ' ')
                at--;
        }

        char *leftBrac = strchr(hz + 2, '(');
        // If brackets for core/thread count present, selective removal
        if (leftBrac)
            memmove(at, leftBrac - 1, strlen(leftBrac - 1) + 1);
        // If no brackets for core/thread count, just nuke
        else
            *at = '\0';
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

        strLen = strlen(result);

        // Remove potential bracketed data at the end. Start by finding first
        // non-null character...
        while (strLen > 0 && result[strLen - 1] == ' ') strLen--;
        if (strLen > 0 && result[strLen - 1] == ')')
        {
            for (int i = strLen - 1; i >= 0; i--)
            {
                if (result[i] == '(')
                {
                    result[i] = '\0';
                    break;
                }
            }
        }
    }

    return result;
}

/**
 * Extracts CPU data from the given cpuinfo file and packs it into a CPU_DATA
 * struct for future processing and interpretation.
 * @param cpuInfo A file path to a cpuinfo file to read
 * @param gpuFromCPU A pointer to a string for returning an extracted GPU name
 * @return A CPU_DATA struct containing CPU data; NULL if no info found/error
 */
CPU_DATA *getCPU(char *cpuInfo, char **gpuFromCPU)
{
    if (!cpuInfo) return NULL;

    CPU_DATA *result = malloc(sizeof(CPU_DATA));
    if (!result) return NULL;

    *result = (CPU_DATA) {
        .arch = UNKNOWN,
        .family = -1,
        .model = -1,
        .stepping = -1,
        .freq = -1,
        .index = 0,
        .maxPhysID = -1,
        .cores = -1,
        .threads = -1,
        .cacheSize = -1,
        .flags[0] = '\0'
    };



    FILE *fStream = fopen(cpuInfo, "r");
    if (fStream)
    {
        char buffer[CPUINFO_BUFFER_LEN];
        while (fgets(buffer, sizeof(buffer), fStream))
        {
            // RISC-V: get micro architecture (uarch)
            if (!result->uarch && strncasecmp(buffer, "uarch", 5) == 0)
            {
                char *extract = extractFromPoint(buffer, UARCH_LEN, ':', 2);
                if (extract)
                {
                    if (result->arch == UNKNOWN)
                        result->arch = RISCV;

                    result->uarch = malloc(UARCH_LEN);
                    strncpy(result->uarch, extract, UARCH_LEN - 1);
                    result->uarch[UARCH_LEN - 1] = '\0';
                    free(extract);
                }
            }
            // x86: get vendor ID
            else if (!result->vendor && strncasecmp(buffer, "vendor_id", 9) == 0)
            {
                char *extract = extractFromPoint(buffer, VENDOR_LEN, ':', 2);
                if (extract)
                {
                    if (result->arch == UNKNOWN)
                        result->arch = X86;

                    result->vendor = malloc(VENDOR_LEN);
                    strncpy(result->vendor, extract, VENDOR_LEN - 1);
                    result->vendor[VENDOR_LEN - 1] = '\0';
                    free(extract);
                }
            }
            // ARM: get CPU implementer name
            else if (!result->vendor && (strncasecmp(buffer, "cpu implementer", 15) == 0 || strncasecmp(buffer, "cpu implementor", 15) == 0))
            {
                char *extract = extractFromPoint(buffer, 16, ':', 2);
                if (extract)
                {
                    if (result->arch == UNKNOWN)
                        result->arch = ARM;

                    // Try to resolve the implementer name from the received
                    // hex value
                    char *end = NULL;
                    long val = strtol(extract, &end, 0);
                    if (end != extract && val >= 0 && val <= 193 && ARM_IMPLEMENTERS[val])
                    {
                        result->vendor = malloc(VENDOR_LEN);
                        strncpy(result->vendor, ARM_IMPLEMENTERS[val], VENDOR_LEN - 1);
                        result->vendor[VENDOR_LEN - 1] = '\0';
                        free(extract);
                    }
                }
            }
            // x86: get model name
            else if (!result->name && strncasecmp(buffer, "model name", 10) == 0)
            {
                char *extract = extractFromPoint(buffer, NAME_LEN, ':', 2);
                if (extract)
                {
                    if (result->arch == UNKNOWN)
                        result->arch = X86;

                    result->name = malloc(NAME_LEN);
                    strncpy(result->name, extract, NAME_LEN - 1);
                    result->name[NAME_LEN - 1] = '\0';
                    free(extract);
                }
            }
            // ARM: get CPU architecture
            else if (!result->name && strncasecmp(buffer, "CPU architecture", 16) == 0)
            {
                char *extract = extractFromPoint(buffer, NAME_LEN, ':', 2);
                if (extract)
                {
                    if (result->arch == UNKNOWN)
                        result->arch = ARM;

                    result->name = malloc(NAME_LEN);
                    snprintf(result->name, NAME_LEN, "Armv%s", extract);
                    free(extract);
                }
            }
            // x86: get family number
            else if (result->family == -1 && strncasecmp(buffer, "cpu family", 10) == 0)
            {
                char *extract = extractFromPoint(buffer, 4, ':', 2);
                if (extract)
                {
                    if (result->arch == UNKNOWN)
                        result->arch = X86;

                    result->family = atoi(extract);
                    free(extract);
                }
            }
            // POWER: get CPU type
            else if (!result->name && strncasecmp(buffer, "cpu", 3) == 0)
            {
                char *extract = extractFromPoint(buffer, NAME_LEN, ':', 2);
                if (extract && extract[0] == 'P' && extract[4] == 'R')
                {
                    if (result->arch == UNKNOWN)
                        result->arch = POWER;

                    result->name = malloc(NAME_LEN);
                    strncpy(result->name, extract, NAME_LEN - 1);
                    free(extract);

                    // In cases like "POWER9, altivec supported", we want to
                    // remove the comma and everything after
                    char *comma = strchr(result->name, ',');
                    if (comma) *comma = '\0';
                }
            }
            // RISC-V: get instruction set architecture (ISA)
            else if (!result->name && strncasecmp(buffer, "isa", 3) == 0)
            {
                char *extract = extractFromPoint(buffer, CPUINFO_BUFFER_LEN, ':', 2);
                if (extract && extract[0] == 'r' && extract[1] == 'v')
                {
                    if (result->arch == UNKNOWN)
                        result->arch = RISCV;

                    result->name = malloc(NAME_LEN);
                    strncpy(result->name, "RISC-V", NAME_LEN - 1);
                    result->name[NAME_LEN - 1] = '\0';
                }
            }
            // x86: get model number
            else if (result->model == -1 && strncasecmp(buffer, "model", 5) == 0)
            {
                char *extract = extractFromPoint(buffer, 4, ':', 2);
                if (extract)
                {
                    if (result->arch == UNKNOWN)
                        result->arch = X86;

                    result->model = atoi(extract);
                    free(extract);
                }
            }
            // x86: get stepping number
            else if (result->stepping == -1 && strncasecmp(buffer, "stepping", 8) == 0)
            {
                char *extract = extractFromPoint(buffer, 4, ':', 2);
                if (extract && extract[0] != 'u')
                {
                    if (result->arch == UNKNOWN)
                        result->arch = X86;

                    result->stepping = atoi(extract);
                    free(extract);
                }
            }
            // x86: get clock frequency in MHz
            else if (result->freq < 0 && strncasecmp(buffer, "cpu mhz", 7) == 0)
            {
                char *extract = extractFromPoint(buffer, 16, ':', 2);
                if (extract)
                {
                    if (result->arch == UNKNOWN)
                        result->arch = X86;

                    result->freq = atof(extract);
                    free(extract);
                }
            }
            // POWER: get clock speed in MHz
            else if (result->freq < 0 && strncasecmp(buffer, "clock", 5) == 0)
            {
                char *extract = extractFromPoint(buffer, 16, ':', 2);
                if (extract)
                {
                    if (result->arch == UNKNOWN)
                        result->arch = POWER;

                    result->freq = atof(extract);
                    free(extract);
                }
            }
            // RISC-V: get clock speed in MHz
            else if (result->freq < 0 && strncasecmp(buffer, "cpu-freq", 8) == 0)
            {
                char *extract = extractFromPoint(buffer, 16, ':', 2);
                if (extract)
                {
                    if (result->arch == UNKNOWN)
                        result->arch = RISCV;

                    // RISC-V CPU frequencies are in GHz but the rest of the
                    // code will expect MHz, so let's convert
                    result->freq = atof(extract) * 1000;
                    free(extract);
                }
            }
            // All: get processor index count (must repeat to get the final
            // value)
            else if (strncasecmp(buffer, "processor", 9) == 0)
            {
                char *extract = extractFromPoint(buffer, 5, ':', 2);
                if (extract)
                {
                    result->index = (atoi(extract) + 1);
                    free(extract);
                }
            }
            // x86: get maximum physical ID (must repeat to get the final
            // value)
            else if (strncasecmp(buffer, "physical id", 11) == 0)
            {
                char *extract = extractFromPoint(buffer, 5, ':', 2);
                if (extract)
                {
                    int val = atoi(extract) + 1;
                    // This can sometimes count backwards, so we need this
                    // check...
                    if (val > result->maxPhysID)
                        result->maxPhysID = val;
                    free(extract);
                }
            }
            // x86: get physical core count
            else if (result->cores == -1 && strncasecmp(buffer, "cpu cores", 9) == 0)
            {
                char *extract = extractFromPoint(buffer, 5, ':', 2);
                if (extract)
                {
                    if (result->arch == UNKNOWN)
                        result->arch = X86;

                    result->cores = atoi(extract);
                    free(extract);
                }
            }
            // x86: get logical thread count
            else if (result->threads == -1 && (strncasecmp(buffer, "siblings", 8) == 0 || strncasecmp(buffer, "Number of siblings", 18) == 0))
            {
                char *extract = extractFromPoint(buffer, 5, ':', 2);
                if (extract)
                {
                    if (result->arch == UNKNOWN)
                        result->arch = X86;

                    result->threads = atoi(extract);
                    free(extract);
                }
            }
            // RISC-V: get hardware thread (hart) count
            else if (strncasecmp(buffer, "hart", 4) == 0)
            {
                char *extract = extractFromPoint(buffer, 5, ':', 2);
                if (extract)
                {
                    if (result->arch == UNKNOWN)
                        result->arch = RISCV;

                    result->threads = atoi(extract) + 1;
                    free(extract);

                    // Whilst typically the exception and not the rule, some
                    // CPUs like SiFive Freedom U540 start from 1 instead of 0.
                    // If the hart count is more than 1 and is odd, we'll 
                    // decrement it.
                    if (result->threads > 1 && result->threads % 2 != 0)
                        result->threads--;
                }
            }
            // x86: get cache size in KB
            else if (result->cacheSize == -1 && strncasecmp(buffer, "cache size", 10) == 0)
            {
                char *extract = extractFromPoint(buffer, 16, ':', 2);
                if (extract)
                {
                    if (result->arch == UNKNOWN)
                        result->arch = X86;

                    result->cacheSize = atoi(extract);
                    free(extract);
                }
            }
            // x86: get CPU flags
            else if (result->flags[0] == '\0' && strncasecmp(buffer, "flags", 5) == 0)
            {
                char *extract = extractFromPoint(buffer, CPUINFO_BUFFER_LEN, ':', 2);
                if (extract)
                {
                    strncpy(result->flags, extract, FLAGS_LEN - 1);
                    result->flags[FLAGS_LEN - 1] = '\0';
                    free(extract);
                }
            }
        }
        fclose(fStream);
    }
    else 
    {
        free(result->uarch);
        free(result->vendor);
        free(result->name);
        free(result);
        return NULL;
    }



    if (result->arch == X86)
    {
        // If the model name has GPU name in it, we will extract it and save if
        // for later in case we need it as a fallback when GPU detection fails
        // or produces no results
        for (int i = 0; i < GPU_FROM_CPU_NEEDLES_LEN; i++)
        {
            char *found = strstr(result->name, GPU_FROM_CPU_NEEDLES[i]);
            if (found)
            {
                // Save it for later
                *gpuFromCPU = malloc(GPU_NAME_LEN);
                if (*gpuFromCPU)
                {
                    const char *gpuVendor = "AMD";
                    snprintf(*gpuFromCPU, GPU_NAME_LEN, "%s %s", gpuVendor, found + (strchr(GPU_FROM_CPU_NEEDLES[i], ' ') - GPU_FROM_CPU_NEEDLES[i]) + 1);

                    // Discard comma and after if present
                    // See: AMD A4-9120e
                    char *commaNeedle = strchr(*gpuFromCPU, ',');
                    if (commaNeedle)
                        *commaNeedle = '\0';

                    // Remove possible trailing suffixes like "Graphics" if
                    // present
                    for (int i = 0; i < GPU_FROM_CPU_SUFFIXES_LEN; i++)
                    {
                        size_t gpuLen = strlen(*gpuFromCPU);
                        size_t suffixLen = strlen(GPU_FROM_CPU_SUFFIXES[i]);
                        if (gpuLen > suffixLen && strcmp(*gpuFromCPU + gpuLen - suffixLen, GPU_FROM_CPU_SUFFIXES[i]) == 0)
                        {
                            (*gpuFromCPU)[gpuLen - suffixLen] = '\0';
                            break;
                        }
                    }

                    // Replace "RADEON" -> "Radeon"
                    // See: AMD A4-9120e
                    char *capsRadeonNeedle = strstr(*gpuFromCPU, "RADEON");
                    if (capsRadeonNeedle)
                        memcpy(capsRadeonNeedle, "Radeon", 6);

                    char *tmp = cleanGPUName(gpuVendor, *gpuFromCPU, 1);
                    strncpy(*gpuFromCPU, tmp, GPU_NAME_LEN - 1);
                    (*gpuFromCPU)[GPU_NAME_LEN - 1] = '\0';
                    free(tmp);
                }

                // Remove it from model name
                *found = '\0';

                // Make sure there isn't any trailing nonsense left
                char *end = found - 1;
                while (end > result->name && (*end == ' ' || *end == ',' || *end == '-'))
                    *end-- = '\0';
            }
        }
    }



    return result;
}

/**
 * Checks if the given CPU flag is present in the captured data.
 * @param cpu Initialised CPU_DATA struct containing the flags string to search
 * @param flag The flag to find ("ht", "pae", etc.)
 * @return 1 if flag found; 0 if not found or no data
 */
int hasFlag(const CPU_DATA *cpu, const char *flag)
{
    // No flags were found, so let's exit...
    if (cpu->flags[0] == '\0')
        return 0;

    const char *needle = cpu->flags;
    size_t flagLen = strlen(flag);

    while ((needle = strstr(needle, flag)))
    {
        // Unless we're at the end of the flags string, we want to search for
        // the flag with spaces surrounding it so we don't match "sse2" when we
        // want "sse" (etc.). If we're at the start of the flags string, we add
        // a "fake" space so the boundary check doesn't read before the buffer
        char before = (needle == cpu->flags) ? ' ' : *(needle - 1);
        char after  = *(needle + flagLen);

        if (before == ' ' && (after == ' ' || after == '\0' || after == '\n'))
            return 1;

        needle += flagLen;
    }

    return 0;
}

/**
 * Builds a complete CPU string from given CPU data. It also applies various
 * corrections to handle quirks and edgecases, and tries to differentiate
 * processors with similar data.
 * @param cpu A pointer to a CPU_DATA struct containing the data to intepret
 * @return String containing the CPU's name and core/thread specs; empty string
 *         if unknown/error
 */
char *interpretCPU(CPU_DATA *cpu)
{
    if (!cpu) return NULL;

    const int RESULT_LEN = NAME_LEN * 2;
    char *result = malloc(RESULT_LEN);
    if (!result) return NULL;
    result[0] = '\0';



    // Run through our x86-specific quirks, distinctions and manipulation
    if (cpu->arch == X86 && cpu->vendor && cpu->name && (cpu->vendor[0] != '\0' || cpu->name[0] != '\0'))
    {
        // Check if model name lacks the vendor name and if we need to try
        // adding it in manually
        if ((cpu->vendor[0] != '\0' && cpu->vendor[0] != 'u') && (cpu->name[0] != '\0' && cpu->name[0] != 'u'))
        {
            if (!strstr(cpu->name, "Intel") && !strstr(cpu->name, "AMD") && !strstr(cpu->name, "Cyrix") && !strstr(cpu->name, "IDT") && !strstr(cpu->name, "VIA") && !strstr(cpu->name, "Transmeta"))
            {
                char *tmp = malloc(NAME_LEN);
                if (tmp)
                {
                    if (strstr(cpu->vendor, "Intel") || strstr(cpu->vendor, "Iotel"))
                        snprintf(tmp, NAME_LEN, "%s %s", "Intel", cpu->name);
                    else if (strstr(cpu->vendor, "AMD"))
                        snprintf(tmp, NAME_LEN, "%s %s", "AMD", cpu->name);
                    else if (strstr(cpu->vendor, "Cyrix"))
                        snprintf(tmp, NAME_LEN, "%s %s", "Cyrix", cpu->name);
                    else if (strstr(cpu->vendor, "Centaur"))
                        snprintf(tmp, NAME_LEN, "%s %s", "IDT/Centaur", cpu->name);
                    else if (strstr(cpu->vendor, "VIA"))
                        snprintf(tmp, NAME_LEN, "%s %s", "VIA", cpu->name);
                    else if (strstr(cpu->vendor, "Transmeta") || strstr(cpu->vendor, "TM"))
                        snprintf(tmp, NAME_LEN, "%s %s", "Transmeta", cpu->name);
                    else
                        snprintf(tmp, NAME_LEN, "%s %s", cpu->vendor, cpu->name);
                    
                    strncpy(cpu->name, tmp, NAME_LEN);
                    free(tmp);
                    cpu->name[NAME_LEN-1] = '\0';
                }
            }
        }

        // 486
        if (cpu->family == 4)
        {
            if (cpu->vendor[0] == 'C' && cpu->vendor[1] == 'y')
            {
                // If we have a Cx486Dxxx with FPU, make sure 387 is included in
                // the model name
                if ((strstr(cpu->name, "Cx486DLC") || strstr(cpu->name, "Cx486DRx2")) && hasFlag(cpu, "fpu"))
                {
                    char tmp[NAME_LEN];
                    snprintf(tmp, NAME_LEN, "%s + 387", cpu->name);
                    strncpy(cpu->name, tmp, NAME_LEN-1);
                    cpu->name[NAME_LEN-1] = '\0';
                }
                // If we have a Cx486S with FPU, make sure 487 is included in the
                // model name
                else if (strstr(cpu->name, "Cx486S") && hasFlag(cpu, "fpu"))
                {
                    char tmp[NAME_LEN];
                    snprintf(tmp, NAME_LEN, "%s + 487", cpu->name);
                    strncpy(cpu->name, tmp, NAME_LEN-1);
                    cpu->name[NAME_LEN-1] = '\0';
                }
            }
            else
            {
                // If we have a vendorless and revisionless 486, we can at least
                // infer if its purely 486SX, or a 486DX, 487SX (true 486SX +
                // 487SX) or 486SX + 387 (eg, IBM 486BLx/486SLCx + 387), via the
                // presence of an FPU
                if (cpu->model == 0 && (cpu->vendor[0] == '\0' || cpu->vendor[0] == 'u') && cpu->name[0] != '\0' && strcmp(cpu->name, "486") == 0)
                {
                    if (hasFlag(cpu, "fpu"))
                        snprintf(cpu->name, NAME_LEN, "486DX/487SX/486SX + 387");
                    else
                        snprintf(cpu->name, NAME_LEN, "486SX");
                }
            }
        }
        // Pentium/P5 and K6
        else if (cpu->family == 5)
        {
            if (cpu->vendor[0] == 'A')
            {
                // If we have a K6, we will try to distinguish if it's a Model
                // 6 or Model 7
                if (cpu->model == 6)
                {
                    char tmp[NAME_LEN];
                    snprintf(tmp, NAME_LEN, "AMD K6 (Model 6)");
                    strncpy(cpu->name, tmp, NAME_LEN-1);
                    cpu->name[NAME_LEN-1] = '\0';
                }
                else if (cpu->model == 7)
                {
                    char tmp[NAME_LEN];
                    snprintf(tmp, NAME_LEN, "AMD K6 (Model 7)");
                    strncpy(cpu->name, tmp, NAME_LEN-1);
                    cpu->name[NAME_LEN-1] = '\0';
                }
                // If we have a supposed K6-III, it may actually be a K6-2+ or
                // K6-III+, and we may be able to tell from the stepping
                else if (cpu->model == 13)
                {
                    if (cpu->stepping == 0)
                    {
                        char tmp[NAME_LEN];
                        snprintf(tmp, NAME_LEN, "AMD K6-III+");
                        strncpy(cpu->name, tmp, NAME_LEN-1);
                        cpu->name[NAME_LEN-1] = '\0';
                    }
                    else if (cpu->stepping == 4)
                    {
                        char tmp[NAME_LEN];
                        snprintf(tmp, NAME_LEN, "AMD K6-2+");
                        strncpy(cpu->name, tmp, NAME_LEN-1);
                        cpu->name[NAME_LEN-1] = '\0';
                    }
                }
            }
            else if (cpu->vendor[0] == 'G' && cpu->vendor[1] == 'e')
            {
                // Pentium OverDrives for Sockets 4 and 5 do not distinguish themselves
                // by name from their base P5 or P54C Pentiums, but we can use
                // OverDrive's 100+MHz clockspeeds to tell them apart
                if (cpu->model == 1 && cpu->freq >= 100)
                {
                    char tmp[NAME_LEN];
                    snprintf(tmp, NAME_LEN, "Intel Pentium OverDrive (P5)");
                    strncpy(cpu->name, tmp, NAME_LEN-1);
                    cpu->name[NAME_LEN-1] = '\0';
                }
                else if (cpu->model == 2 && cpu->freq >= 100)
                {
                    char tmp[NAME_LEN];
                    snprintf(tmp, NAME_LEN, "Intel Pentium OverDrive (P54C)");
                    strncpy(cpu->name, tmp, NAME_LEN-1);
                    cpu->name[NAME_LEN-1] = '\0';
                }
                // The Pentium OverDrive for Socket 3 is guaranteed to be P54C, so
                // let's report that to distinguish it from P5 OverDrives for Socket 4
                else if (cpu->model == 3 && cpu->freq < 84)
                {
                    char tmp[NAME_LEN];
                    snprintf(tmp, NAME_LEN, "Intel Pentium OverDrive (P54C)");
                    strncpy(cpu->name, tmp, NAME_LEN-1);
                    cpu->name[NAME_LEN-1] = '\0';
                }
            }
            else if (cpu->vendor[0] == 'C' && cpu->vendor[1] == 'e')
            {
                // If we have a supposed WinChip 2-3D, we may be able to tell
                // if its a WinChip 2A from the stepping
                if (cpu->model == 8 && cpu->stepping == 7)
                {
                    char tmp[NAME_LEN];
                    snprintf(tmp, NAME_LEN, "IDT WinChip 2A");
                    strncpy(cpu->name, tmp, NAME_LEN-1);
                    cpu->name[NAME_LEN-1] = '\0';
                }
            }
        }
        // Pentium/P6 & K7
        else if (cpu->family == 6)
        {
            if (cpu->vendor[0] == 'A')
            {
                // AMD Mobile Athlon and Duron have the "mobile" part of their
                // name before "AMD" *and* is not capitalised
                if (strstr(cpu->name, "mobile AMD"))
                {
                    char *tmp = findReplace(cpu->name, NAME_LEN, "mobile AMD", "AMD Mobile");
                    if (tmp)
                    {
                        strncpy(cpu->name, tmp, NAME_LEN - 1);
                        cpu->name[NAME_LEN-1] = '\0';
                        free(tmp);
                    }
                }

                if (cpu->model == 3)
                {
                    // Some Spitfire Durons were either incorrectly programmed
                    // as "Athlon" or the Linux kernel once determined and
                    // assembled their name incorrectly
                    if (strstr(cpu->name, "Athlon"))
                    {
                        char *tmp = findReplace(cpu->name, NAME_LEN, "Athlon", "Duron");
                        if (tmp)
                        {
                            strncpy(cpu->name, tmp, NAME_LEN - 1);
                            cpu->name[NAME_LEN-1] = '\0';
                            free(tmp);
                        }
                    }
                }

                if (cpu->model == 6 || cpu->model == 8 || cpu->model == 10)
                {
                    // Athlon XP/MP (models 6, 8 and 10) sometimes just call
                    // themselves "Athlon", and given XP and MP are basically
                    // the same, it is hard to distinguish them. We can try
                    // looking for the obvious signs (multi-CPU configuration)
                    // or if the "mp" (Linux's multi-processor) flag is
                    // present.
                    if (strstr(cpu->name, "Athlon") && !strstr(cpu->name, " MP ") && !strstr(cpu->name, " XP "))
                    {
                        int hasMPFlag = hasFlag(cpu, "mp");
                        char *tmp = NULL;

                        // Literally checking for multiple CPUs counted is the
                        // only sure way to figure this out for all possible
                        // models
                        if (cpu->index > 1)
                            tmp = findReplace(cpu->name, NAME_LEN, "Athlon", "Athlon MP");
                        // Palomino XP and MP always receive the "mp" flag, so
                        // sans multiple CPUs counted, there is no reliable way
                        // to tell XP and MP apart
                        else if (cpu->model == 6)
                            tmp = findReplace(cpu->name, NAME_LEN, "Athlon", "Athlon XP/MP");
                        // Thoroughbred and Barton will be given the "mp" flag
                        // appropriately, so we can use that!
                        else if (cpu->model == 8 || cpu->model == 10)
                        {
                            if (hasMPFlag)
                                tmp = findReplace(cpu->name, NAME_LEN, "Athlon", "Athlon MP");
                            else
                                tmp = findReplace(cpu->name, NAME_LEN, "Athlon", "Athlon XP");
                        }

                        if (tmp)
                        {
                            strncpy(cpu->name, tmp, NAME_LEN - 1);
                            cpu->name[NAME_LEN-1] = '\0';
                            free(tmp);
                        }
                    }

                    // All Athlon MPs were single core, so if the processor
                    // index is higher than 1, we are definitely dealing with a
                    // multi-CPU config and should set the core/thread count to
                    // 1 to correctly flag this instead of 'ahving'' two cores
                    // for later
                    if (cpu->index > 1 && cpu->cores == -1 && cpu->threads == -1)
                        cpu->cores = cpu->threads = 1;
                }

                // There are multiple generations of K7-era Athlon and Duron
                // that can reuse model numbers or lack them completely, so we
                // should always append the core names to them
                // See: Athlon K7750MTR52B A (6-2-2), Athlon K7850MPR52B A
                //      (6-2-1), Athlon K7100MNR53B A (6-2-2)
                // Argon: Athlon "Classic"
                if (cpu->model == 1)
                    strncat(cpu->name, " (Argon)", NAME_LEN - strlen(cpu->name) - 1);
                // Pluto/Orion: Athlon "Classic"
                else if (cpu->model == 2)
                {
                    // Pluto and Orion are both model 2 and could have stepping
                    // 1 or 2, but we at least know Orion is 900MHz and
                    // higher...
                    if (cpu->freq > 0 && cpu->freq < 855)
                        strncat(cpu->name, " (Pluto)", NAME_LEN - strlen(cpu->name) - 1);
                    else if (cpu->freq > 895)
                        strncat(cpu->name, " (Orion)", NAME_LEN - strlen(cpu->name) - 1);
                    else
                        strncat(cpu->name, " (Pluto/Orion)", NAME_LEN - strlen(cpu->name) - 1);
                }
                // Spitfire: Duron
                else if (cpu->model == 3)
                    strncat(cpu->name, " (Spitfire)", NAME_LEN - strlen(cpu->name) - 1);
                // Thunderbird: Athlon "Classic"
                else if (cpu->model == 4)
                    strncat(cpu->name, " (Thunderbird)", NAME_LEN - strlen(cpu->name) - 1);
                // Palomino: Athlon XP/MP
                else if (cpu->model == 6)
                    strncat(cpu->name, " (Palomino)", NAME_LEN - strlen(cpu->name) - 1);
                // Morgan: Duron
                else if (cpu->model == 7)
                    strncat(cpu->name, " (Morgan)", NAME_LEN - strlen(cpu->name) - 1);
                // Thoroughbred: Athlon XP/MP
                else if (cpu->model == 8)
                    strncat(cpu->name, " (Thoroughbred)", NAME_LEN - strlen(cpu->name) - 1);
                // Barton: Athlon XP/MP
                else if (cpu->model == 10)
                    strncat(cpu->name, " (Barton)", NAME_LEN - strlen(cpu->name) - 1);
            }
            else if (cpu->vendor[0] == 'G' && cpu->vendor[1] == 'e')
            {
                // Deschutes & Covington
                if (cpu->model == 5)
                {
                    // Pentium II (Deschutes) and the Deschutes-based Pentium II Xeon
                    // and Celeron (Covington) have basically the same CPU ID, but we
                    // can tell *some* apart from the cache size. For sure: 32KB =
                    // Celeron; 512KB = Pentium II; 1024/2048KB = Pentium II Xeon. The
                    // 512KB Xeon cannot presently be distinguished, though...
                    if (cpu->cacheSize == 32)
                    {
                        char tmp[NAME_LEN];
                        snprintf(tmp, NAME_LEN, "Intel Celeron (Covington)");
                        strncpy(cpu->name, tmp, NAME_LEN-1);
                        cpu->name[NAME_LEN-1] = '\0';
                    }
                    else if (cpu->cacheSize == 512)
                    {
                        // Don't need to do anything, yet... At some point, we will try
                        // to distinguish standard II and II Xeon at this cache amount
                    }
                    else if (cpu->cacheSize >= 1024)
                    {
                        char tmp[NAME_LEN];
                        snprintf(tmp, NAME_LEN, "Intel Pentium II Xeon");
                        strncpy(cpu->name, tmp, NAME_LEN-1);
                        cpu->name[NAME_LEN-1] = '\0';
                    }
                }
                // Banias
                else if (cpu->model == 9)
                {
                    // Both generations of Pentium M and related Celeron M do not
                    // distinguish themselves in their model names nor model numbers,
                    // so we add "(Banias)" to the first gen's name to distinguish it
                    // from second-gen Dothan
                    if (strstr(cpu->name, "(R) M p"))
                    {
                        char *tmp = findReplace(cpu->name, NAME_LEN, "M processor", "M (Banias)");
                        if (tmp)
                        {
                            strncpy(cpu->name, tmp, NAME_LEN - 1);
                            cpu->name[NAME_LEN-1] = '\0';
                            free(tmp);
                        }
                    }
                }
                // Dothan
                else if (cpu->model == 13)
                {
                    // Both generations of Pentium M and related Celeron M do not
                    // distinguish themselves in their model names nor model numbers,
                    // so we add "(Dothan)" to the second gen's name to distinguish it
                    // from first-gen Banias
                    if (strstr(cpu->name, "(R) M p"))
                    {
                        char *tmp = findReplace(cpu->name, NAME_LEN, "M processor", "M (Dothan)");
                        if (tmp)
                        {
                            strncpy(cpu->name, tmp, NAME_LEN - 1);
                            cpu->name[NAME_LEN-1] = '\0';
                            free(tmp);
                        }
                    }
                }
                // Yonah
                else if (cpu->model == 14)
                {
                    // Core (Yonah) may not have "Core" in their name, so we will
                    // try to add it and a "Solo" or "Duo" suffix depending on the
                    // core count
                    // See: Core Solo T1300, Core Duo T2300
                    if (cpu->stepping == 8 && strstr(cpu->name, "Intel(R) CPU"))
                    {
                        char *tmp = NULL;
                        if (cpu->cores == 1 || cpu->index == 1)
                            tmp = findReplace(cpu->name, NAME_LEN, "CPU           ", "Core Solo ");
                        else if (cpu->cores == 2 || cpu->index == 2)
                            tmp = findReplace(cpu->name, NAME_LEN, "CPU           ", "Core Duo ");

                        if (tmp)
                        {
                            strncpy(cpu->name, tmp, NAME_LEN - 1);
                            cpu->name[NAME_LEN-1] = '\0';
                            free(tmp);
                        }
                    }
                    // Some Yonah-based Celeron Ms only report as simply "Celeron",
                    // so we will add the "M" in if so
                    // See: Celeron M 215
                    else if (cpu->stepping == 8 && strstr(cpu->name, "Celeron(R) CPU"))
                    {
                        char *tmp = findReplace(cpu->name, NAME_LEN, "Celeron(R) CPU", "Celeron M");
                        if (tmp)
                        {
                            strncpy(cpu->name, tmp, NAME_LEN - 1);
                            cpu->name[NAME_LEN-1] = '\0';
                            free(tmp);
                        }
                    }
                }
                // Merom
                else if (cpu->model == 15)
                {
                    // Core 2 Duo
                    if (cpu->cores == 2 || cpu->index == 2)
                    {
                        // Some Merom-based Pentium Dual-Cores have a rogue "Dual" in
                        // their name
                        // See: Pentium T3200
                        if (strstr(cpu->name, "Dual  CPU"))
                        {
                            char *tmp = findReplace(cpu->name, NAME_LEN, "Dual  CPU", " ");
                            if (tmp)
                            {
                                strncpy(cpu->name, tmp, NAME_LEN - 1);
                                cpu->name[NAME_LEN-1] = '\0';
                                free(tmp);
                            }
                        }
                        // Mobile Core 2 Duo (Merom) may not have "Duo" in their name,
                        // so we will try to add it in
                        // See: Core 2 Duo T7400
                        else if (strstr(cpu->name, "2 CPU"))
                        {
                            char *tmp = findReplace(cpu->name, NAME_LEN, "CPU         ", "Duo ");
                            if (tmp)
                            {
                                strncpy(cpu->name, tmp, NAME_LEN - 1);
                                cpu->name[NAME_LEN-1] = '\0';
                                free(tmp);
                            }
                        }
                    }
                }
                // Penryn
                else if (cpu->model == 23)
                {
                    if (cpu->stepping == 10)
                    {
                        // ULV Pentium Dual-Core Mobile lacks both the "Pentium"
                        // branding and the "S" in their model number
                        // See: Pentium SU2700, Pentium SU4100
                        if (strstr(cpu->name, "Intel(R) CPU           U"))
                        {
                            char *tmp = findReplace(cpu->name, NAME_LEN, "(R) CPU           ", " Pentium S");
                            if (tmp)
                            {
                                strncpy(cpu->name, tmp, NAME_LEN - 1);
                                cpu->name[NAME_LEN-1] = '\0';
                                free(tmp);
                            }
                        }
                        // LV Core 2 Duo lacks the "S" in their model number
                        // See: Core 2 Duo SL9600
                        else if (strstr(cpu->name, "Duo CPU     L"))
                        {
                            char *tmp = findReplace(cpu->name, NAME_LEN, "CPU     ", "S");
                            if (tmp)
                            {
                                strncpy(cpu->name, tmp, NAME_LEN - 1);
                                cpu->name[NAME_LEN-1] = '\0';
                                free(tmp);
                            }
                        }
                    }
                }
                // Nehalem (Bloomfield (26), Clarksfield/Lynnfield (30))
                // & Westmere (Arrandale/Clarkdale (37), Gulftown (44))
                else if (cpu->model == 26 || cpu->model == 30 || cpu->model == 37 || cpu->model == 44)
                {
                    // If present at all, Nehalem/Westmere has what should be the
                    // suffix as the prefix *and* Clarksfield specifically lacks
                    // the "M" to denote those are mobile chips
                    // See: Core i3-380UM, Core i5-540M, Core i5-750, Core i7-640LM
                    //      Core i7-740QM, Core i7-860S, Core i7-920XM, Core i7-980
                    //      Core i7-990X
                    if ((cpu->stepping == 2 || cpu->stepping == 5) && strstr(cpu->name, "Core"))
                    {
                        const char *suffix = NULL;
                        if (strstr(cpu->name, "       M"))
                            suffix = "M";
                        else if (strstr(cpu->name, "       Q"))
                            suffix = "QM";
                        else if (strstr(cpu->name, "       X"))
                            suffix = (cpu->model == 30) ? "XM" : (cpu->model == 44) ? "X" : suffix;
                        else if (strstr(cpu->name, "       S"))
                            suffix = "S";
                        else if (strstr(cpu->name, "       K"))
                            suffix = "K";
                        else if (strstr(cpu->name, "       L"))
                            suffix = "LM";
                        else if (strstr(cpu->name, "       U"))
                            suffix = "UM";

                        if (suffix)
                        {
                            // Remove the incorrect prefix
                            char search[32];
                            snprintf(search, 32, " CPU       %c ", suffix[0]);
                            char *tmp = findReplace(cpu->name, NAME_LEN, search, "-");
                            if (tmp)
                            {
                                strncpy(cpu->name, tmp, NAME_LEN - 1);
                                cpu->name[NAME_LEN - 1] = '\0';
                                free(tmp);
                            }

                            // Add the correct suffix and discard the clock speed
                            // whilst we're at it
                            char *needle = strchr(cpu->name, '@');
                            if (needle)
                            {
                                char *p = needle - 1;
                                while (p > cpu->name && *p == ' ') p--;
                                strcpy(p + 1, suffix);
                            }
                        }
                        // If no suffix was found/chosen, we still need to close
                        // the gap between "iX" and the model number...
                        else if (strstr(cpu->name, ") i") && strstr(cpu->name, " CPU         "))
                        {
                            char *tmp = findReplace(cpu->name, NAME_LEN, " CPU         ", "-");
                            if (tmp)
                            {
                                strncpy(cpu->name, tmp, NAME_LEN - 1);
                                cpu->name[NAME_LEN - 1] = '\0';
                                free(tmp);
                            }
                        }
                    }

                    // Bloomfield Core i7 Extremes do not call themselves "Extreme"
                    // See: Core i7-965 Extreme Edition (etc.)
                    if (cpu->model == 26 && (strstr(cpu->name, "965") || strstr(cpu->name, "975")))
                    {
                        char *needle = strrchr(cpu->name, '@');
                        if (needle)
                        {
                            char *p = needle - 1;
                            while (p > cpu->name && *p == ' ') p--;
                            strcpy(p + 1, " Extreme");
                        }
                    }
                }
                // Sandy Bridge
                else if (cpu->model == 42)
                {
                    if (cpu->stepping == 7)
                    {
                        // Sandy Bridge-based Xeon E3s may lack a "-" separating
                        // the "E3" prefix and the rest of the model number
                        // See: Xeon E3-1230, Xeon E3-1275
                        if (strstr(cpu->name, "E31"))
                        {
                            char *tmp = findReplace(cpu->name, NAME_LEN, "E31", " E3-1");
                            if (tmp)
                            {
                                strncpy(cpu->name, tmp, NAME_LEN - 1);
                                cpu->name[NAME_LEN-1] = '\0';
                                free(tmp);
                            }
                        }
                    }
                }
                // Sandy Bridge-EP
                else if (cpu->model == 45)
                {
                    // Some Xeon E5 names may have a rough "0" just after the model
                    // number
                    // See: Xeon E5-2690 (the original/v1)
                    if (strstr(cpu->name, " 0 @"))
                    {
                        char *tmp = findReplace(cpu->name, NAME_LEN, " 0 ", " ");
                        if (tmp)
                        {
                            strncpy(cpu->name, tmp, NAME_LEN - 1);
                            cpu->name[NAME_LEN-1] = '\0';
                            free(tmp);
                        }
                    }
                }
                // Ivy Bridge
                else if (cpu->model == 58)
                {
                    // Some Xeon refreshes may have a capital "V" in their version
                    // discriminator when the marketing names standardise a
                    // lowercase "v"
                    // See: Xeon E3-1230 v2
                    char *vNeedle = strstr(cpu->name, "V");
                    if (vNeedle) *vNeedle = 'v';
                }
                // Silvermont (Merriefield)
                else if (cpu->model == 74)
                {
                    // The 500MHz Intel Atom variant for the Intel Edison has a
                    // non-descriptive name like "Intel 4000", thus we discard it
                    // and create a new model name
                    if (cpu->stepping == 8 && cpu->freq == 500)
                    {
                        free(cpu->name);
                        cpu->name = strdup("Intel Atom Z34xx (Edison)");
                    }
                }

                // There are some examples of Intel Core processors with generic
                // names (likely from hosting/virtualisation platforms) that we
                // want to tidy up a bit. This can happen to several micro-
                // architectures...
                if (strstr(cpu->name, "Core Processor ("))
                {
                    char *uarch = NULL;
                    // Broadwell
                    if (cpu->model == 61)
                        uarch = "Broadwell";
                    // Skylake
                    else if (cpu->model == 94)
                        uarch = "Skylake";

                    if (uarch)
                    {
                        char *needle = strstr(cpu->name, " Processor");
                        if (needle)
                            snprintf(needle, NAME_LEN - (needle - cpu->name), " (%s)", uarch);
                    }
                }
            }
        }
        // Larrabee
        else if (cpu->family == 11)
        {
            // Xeon Phis may have 'nonsense' in the model name like simply
            // "0b/01" for 7110P, so we may discard it and create a new model
            // name with the microarchitecture revision included instead
            if (!strstr(cpu->name, "Xeon"))
            {
                free(cpu->name);
                // Knights Ferry
                if (cpu->model == 0)
                    cpu->name = strdup("Intel Xeon Phi (Knights Ferry)");
                // Knights Corner
                // See: Xeon Phi 7110P
                else if (cpu->model == 1)
                    cpu->name = strdup("Intel Xeon Phi (Knights Corner)");
                // Fallback
                else
                    cpu->name = strdup("Intel Xeon Phi");
            }
        }
        // NetBurst & K8
        else if (cpu->family == 15)
        {
            if (cpu->vendor[0] == 'A')
            {
                // SledgeHammer (5), Athens/Troy (37), San Diego (39)
                if (cpu->model == 5 || cpu->model == 37 || cpu->model == 39)
                {
                    // Some entire models of K8-based Opterons were always
                    // single core, so if the processor index is higher than 1,
                    // we are definitely dealing with a multi-CPU config and
                    // should set the core/thread count to 1 to correctly flag
                    // this instead of 'having' two cores for later
                    // See: Opteron 148 (E4), Opteron 246 (CG), Opteron 246
                    //      (E4), Opteron 848 (E4)
                    if (cpu->index > 1 && cpu->cores == -1 && cpu->threads == -1)
                        cpu->cores = cpu->threads = 1;
                }

                // Early AMD Mobile Semprons may have the "Mobile" part before
                // "AMD"
                if (strstr(cpu->name, "Mobile AMD Sempron"))
                {
                    char *tmp = findReplace(cpu->name, NAME_LEN, "Mobile AMD Sempron", "AMD Mobile Sempron");
                    if (tmp)
                    {
                        strncpy(cpu->name, tmp, NAME_LEN - 1);
                        cpu->name[NAME_LEN-1] = '\0';
                        free(tmp);
                    }
                }
            }
            else if (cpu->vendor[0] == 'G' && cpu->vendor[1] == 'e')
            {
                // Early Pentium 4s generally don't have a model number, and
                // the later ones that do don't report it, so we will
                // distinguish them via their core name
                if (strstr(cpu->name, "4 CPU"))
                {
                    char *tmp = NULL;
                    // Willamette
                    if (cpu->model == 0 || cpu->model == 1)
                        tmp = findReplace(cpu->name, NAME_LEN, "4 CPU", "4 (Willamette)");
                    // Northwood
                    else if (cpu->model == 2)
                        tmp = findReplace(cpu->name, NAME_LEN, "4 CPU", "4 (Northwood)");
                    // Prescott
                    // See: B80546PE0561M, RK80546PG0881M, RK80546PG0961M
                    else if (cpu->model == 3 || cpu->model == 4)
                        tmp = findReplace(cpu->name, NAME_LEN, "4 CPU", "4 (Prescott)");
                    // Cedar Mill
                    // See: Pentium 4 631 (5), Pentium 4 641 (2), Pentium 4 651
                    //      (4)
                    else if (cpu->model == 6)
                        tmp = findReplace(cpu->name, NAME_LEN, "4 CPU", "4 (Cedar Mill)");

                    if (tmp)
                    {
                        strncpy(cpu->name, tmp, NAME_LEN - 1);
                        cpu->name[NAME_LEN-1] = '\0';
                        free(tmp);
                    }
                }
                // Ditto for non-Extreme Pentium D
                else if (cpu->cores == 2)
                {
                    // Non-Extreme (no Hyper-Threading)
                    if (cpu->cores == cpu->threads)
                    {
                        char *tmp = NULL;
                        // Smithfield
                        // See: Pentium D 805 (7), Pentium D 830 (4)
                        if (cpu->model == 4)
                            tmp = findReplace(cpu->name, NAME_LEN, "D CPU", "D (Smithfield)");
                        // Presler
                        // See: Pentium D 920 (2), Pentium D 945 (5), Pentium D 960
                        //      (4)
                        else if (cpu->model == 6)
                            tmp = findReplace(cpu->name, NAME_LEN, "D CPU", "D (Presler)");

                        if (tmp)
                        {
                            strncpy(cpu->name, tmp, NAME_LEN - 1);
                            cpu->name[NAME_LEN-1] = '\0';
                            free(tmp);
                        }
                    }
                    // There are so few 2C/4T Pentium Extreme Editions (really
                    // Pentium D with Hyper-Threading) and all with unique
                    // clock speeds that we might as well just fully identify
                    // them instead of just adding the core name
                    // See: Pentium Extreme 840, Pentium Extreme 955, Pentium
                    //      Extreme 965
                    else if (cpu->cores == 2 && cpu->threads == 4)
                    {
                        char *tmp = NULL;
                        if (cpu->model == 4 && strstr(cpu->name, "3.2"))
                            tmp = findReplace(cpu->name, NAME_LEN, "D CPU", "Extreme 840");
                        else if (cpu->model == 6 && strstr(cpu->name, "3.4"))
                            tmp = findReplace(cpu->name, NAME_LEN, "CPU", "Pentium Extreme 955");
                        else if (cpu->model == 6 && strstr(cpu->name, "3.7"))
                            tmp = findReplace(cpu->name, NAME_LEN, "D CPU", "Extreme 965");

                        if (tmp)
                        {
                            strncpy(cpu->name, tmp, NAME_LEN - 1);
                            cpu->name[NAME_LEN-1] = '\0';
                            free(tmp);
                        }
                    }
                }
            }
        }
        // Bulldozer
        else if (cpu->family == 21)
        {
            // Bulldozer proper
            if (cpu->model == 1)
            {
                // Some AMD FXs are reported with physical ID count that starts
                // at 1 instead of 0, potentially flagging it as a multi-CPU
                // config when it isn't... We can ensure that we aren't in such
                // a config if the processor index count and per-CPU thread
                // count are equal (impossible on a real multi-CPU config).
                // See: AMD FX-8150
                if (cpu->maxPhysID > 1 && cpu->index == cpu->threads)
                    cpu->maxPhysID--;
            }
        }
    }



    // Use the CPU name as the core of the result string
    strncpy(result, cpu->name, RESULT_LEN-1);



    // If we have a vendor name, add it to the start (not for x86 since that is
    // handled above)
    if (cpu->arch != X86 && cpu->vendor && cpu->vendor[0] != '\0' && cpu->vendor[0] != 'u')
    {
        char *tmp = malloc(RESULT_LEN);
        snprintf(tmp, RESULT_LEN, "%s %s", cpu->vendor, result);
        strncpy(result, tmp, RESULT_LEN-1);
        free(tmp);
    }



    // Compile our cores/threads substring
    char coresAndThreads[16] = "";

    if (!COMPACT)
    {
        // If we don't have a cores value, set it to the same as threads
        // so we don't try to show them both separately later
        if (cpu->cores <= 0 && cpu->threads > 0)
            cpu->cores = cpu->threads;

        // If we don't have cores or threads, we use the processor index count in
        // its place
        if (cpu->cores <= 0 && cpu->threads <= 0 && cpu->index > 0)
        {
            // We don't have a good way to tell cores from threads for POWER
            // CPUs at the moment, so let's not imply our value is for cores
            if (cpu->arch == POWER)
                snprintf(coresAndThreads, 16, "%dT", cpu->index);
            else
                snprintf(coresAndThreads, 16, "%dC", cpu->index);
        }
        // If cores and threads are the same value, just show cores
        else if (cpu->cores > 0 && cpu->cores == cpu->threads)
            snprintf(coresAndThreads, 16, "%dC", cpu->cores);
        // If cores and threads are different values, show both
        else if (cpu->cores > 0 && cpu->threads > 0)
            snprintf(coresAndThreads, 16, "%dC/%dT", cpu->cores, cpu->threads);

        // If successful, add the substring to the result string
        if (coresAndThreads[0] != '\0')
        {
            // If our result string has vendor and/or name, we add our cores/
            // threads substring on the end
            if (result[0] != '\0')
            {
                char *tmp = malloc(RESULT_LEN);
                snprintf(tmp, RESULT_LEN, "%s (%s)", result, coresAndThreads);
                strncpy(result, tmp, RESULT_LEN-1);
                free(tmp);
            }
            // If our result is blank, we make it our cores/threads substring
            else
                strncpy(result, coresAndThreads, RESULT_LEN-1);
        }
    }



    // If the maximum physical ID found is more than 1, we should be dealing
    // with a multi-CPU configuration and need to indicate this
    if (cpu->maxPhysID > 1)
    {
        char tmp[RESULT_LEN];
        snprintf(tmp, RESULT_LEN, "%dx %s", cpu->maxPhysID, result);
        strncpy(result, tmp, RESULT_LEN-1);
    }
    // If maxPhysID was not computed or not x86, we can also infer likely
    // multi-CPU configuration when the processor index count is higher than
    // the thread count
    else if (cpu->index > cpu->threads && cpu->threads > 0)
    {
        int cpus = cpu->index / cpu->threads;
        char tmp[RESULT_LEN];
        snprintf(tmp, RESULT_LEN, "%dx %s", cpus, result);
        strncpy(result, tmp, RESULT_LEN-1);
    }



    // Clean up the result string
    char *cleanedResult = cleanCPUName(result, RESULT_LEN);
    strncpy(result, cleanedResult, RESULT_LEN-1);
    free(cleanedResult);

    return result;
}
