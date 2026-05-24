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



#include "cpu.h"
#include "general.h"
#include "globals.h"
#include "replacements.h"

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
        .cores = -1,
        .threads = -1,
        .cacheSize = -1,
        .hasFPU = -1
    };



    FILE *fStream = fopen(cpuInfo, "r");
    if (fStream)
    {
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), fStream))
        {
            // RISC-V: get micro architecture (uarch)
            if (!result->uarch && strncmp(buffer, "uarch", 5) == 0)
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
            else if (!result->vendor && strncmp(buffer, "vendor_id", 9) == 0)
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
            else if (!result->vendor && strncmp(buffer, "CPU implementer", 15) == 0)
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
            else if (!result->name && strncmp(buffer, "model name", 10) == 0)
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
            else if (!result->name && strncmp(buffer, "CPU architecture", 16) == 0)
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
            else if (result->family == -1 && strncmp(buffer, "cpu family", 10) == 0)
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
            else if (!result->name && strncmp(buffer, "cpu", 3) == 0)
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
            else if (!result->name && strncmp(buffer, "isa", 3) == 0)
            {
                char *extract = extractFromPoint(buffer, 128, ':', 2);
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
            else if (result->model == -1 && strncmp(buffer, "model", 5) == 0)
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
            else if (result->stepping == -1 && strncmp(buffer, "stepping", 8) == 0)
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
            else if (result->freq < 0 && strncmp(buffer, "cpu MHz", 7) == 0)
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
            else if (result->freq < 0 && strncmp(buffer, "clock", 5) == 0)
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
            else if (result->freq < 0 && strncmp(buffer, "cpu-freq", 8) == 0)
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
            else if (strncmp(buffer, "processor", 9) == 0)
            {
                char *extract = extractFromPoint(buffer, 5, ':', 2);
                if (extract)
                {
                    result->index = (atoi(extract) + 1);
                    free(extract);
                }
            }
            // x86: get physical core count
            else if (result->cores == -1 && strncmp(buffer, "cpu cores", 9) == 0)
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
            else if (result->threads == -1 && strncmp(buffer, "siblings", 8) == 0)
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
            else if (strncmp(buffer, "hart", 4) == 0)
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
            else if (result->cacheSize == -1 && strncmp(buffer, "cache size", 10) == 0)
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
            // x86: get whether an FPU is present
            else if (result->hasFPU == -1 && strncmp(buffer, "fpu", 3) == 0)
            {
                char *extract = extractFromPoint(buffer, 4, ':', 2);
                if (extract)
                {
                    if (result->arch == UNKNOWN)
                        result->arch = X86;

                    if (strncmp(extract, "yes", 3) == 0)
                        result->hasFPU = 1;
                    else if (strncmp(extract, "no", 2) == 0)
                        result->hasFPU = 0;
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
        const char *gpuNeedles[] = { "with Radeon", "w/ Radeon", "with GeForce", "w/ GeForce" };
        for (int i = 0; i < 4; i++)
        {
            char *found = strstr(result->name, gpuNeedles[i]);
            if (found)
            {
                // Save it for later
                *gpuFromCPU = malloc(NAME_LEN);
                if (*gpuFromCPU)
                {
                    const char *gpuVendor = (i < 2) ? "AMD " : "NVIDIA ";
                    snprintf(*gpuFromCPU, NAME_LEN, "%s%s", gpuVendor, found + (strchr(gpuNeedles[i], ' ') - gpuNeedles[i]) + 1);
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

        // 486 era
        if (cpu->family == 4)
        {
            if (cpu->vendor[0] == 'C' && cpu->vendor[1] == 'y')
            {
                // If we have a Cx486Dxxx with FPU, make sure 387 is included in
                // the model name
                if ((strstr(cpu->name, "Cx486DLC") || strstr(cpu->name, "Cx486DRx2")) && cpu->hasFPU)
                {
                    char tmp[NAME_LEN];
                    snprintf(tmp, NAME_LEN, "%s + 387", cpu->name);
                    strncpy(cpu->name, tmp, NAME_LEN-1);
                    cpu->name[NAME_LEN-1] = '\0';
                }
                // If we have a Cx486S with FPU, make sure 487 is included in the
                // model name
                else if (strstr(cpu->name, "Cx486S") && cpu->hasFPU)
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
                    if (cpu->hasFPU)
                        snprintf(cpu->name, NAME_LEN, "486DX/487SX/486SX + 387");
                    else
                        snprintf(cpu->name, NAME_LEN, "486SX");
                }
            }
        }
        // Pentium/P5 and K6 era
        else if (cpu->family == 5)
        {
            if (cpu->vendor[0] == 'A')
            {
                // If we have a K6, we will try to distinguish if it's a Model
                // 6 or Model 7
                if (cpu->model == 6)
                {
                    char tmp[NAME_LEN];
                    snprintf(tmp, NAME_LEN, "AMD K6 Model 6", cpu->name);
                    strncpy(cpu->name, tmp, NAME_LEN-1);
                    cpu->name[NAME_LEN-1] = '\0';
                }
                else if (cpu->model == 7)
                {
                    char tmp[NAME_LEN];
                    snprintf(tmp, NAME_LEN, "AMD K6 Model 7", cpu->name);
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
                        snprintf(tmp, NAME_LEN, "AMD K6-III+", cpu->name);
                        strncpy(cpu->name, tmp, NAME_LEN-1);
                        cpu->name[NAME_LEN-1] = '\0';
                    }
                    else if (cpu->stepping == 4)
                    {
                        char tmp[NAME_LEN];
                        snprintf(tmp, NAME_LEN, "AMD K6-2+", cpu->name);
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
                    snprintf(tmp, NAME_LEN, "Intel Pentium OverDrive (P5)", cpu->name);
                    strncpy(cpu->name, tmp, NAME_LEN-1);
                    cpu->name[NAME_LEN-1] = '\0';
                }
                else if (cpu->model == 2 && cpu->freq >= 100)
                {
                    char tmp[NAME_LEN];
                    snprintf(tmp, NAME_LEN, "Intel Pentium OverDrive (P54C)", cpu->name);
                    strncpy(cpu->name, tmp, NAME_LEN-1);
                    cpu->name[NAME_LEN-1] = '\0';
                }
                // The Pentium OverDrive for Socket 3 is guaranteed to be P54C, so
                // let's report that to distinguish it from P5 OverDrives for Socket 4
                else if (cpu->model == 3 && cpu->freq < 84)
                {
                    char tmp[NAME_LEN];
                    snprintf(tmp, NAME_LEN, "Intel Pentium OverDrive (P54C)", cpu->name);
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
                    snprintf(tmp, NAME_LEN, "IDT WinChip 2A", cpu->name);
                    strncpy(cpu->name, tmp, NAME_LEN-1);
                    cpu->name[NAME_LEN-1] = '\0';
                }
            }
        }
        // Pentium/P6 and later
        else if (cpu->family == 6)
        {
            // Pentium II (Deschutes) and the Deschutes-based Pentium II Xeon
            // and Celeron (Covington) have basically the same CPU ID, but we
            // can tell *some* apart from the cache size. For sure: 32KB =
            // Celeron; 512KB = Pentium II; 1024/2048KB = Pentium II Xeon. The
            // 512KB Xeon cannot presently be distinguished, though...
            if (cpu->model == 5)
            {
                if (cpu->cacheSize == 32)
                {
                    char tmp[NAME_LEN];
                    snprintf(tmp, NAME_LEN, "Intel Celeron (Covington)", cpu->name);
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
                    snprintf(tmp, NAME_LEN, "Intel Pentium II Xeon", cpu->name);
                    strncpy(cpu->name, tmp, NAME_LEN-1);
                    cpu->name[NAME_LEN-1] = '\0';
                }
            }
            // Core (Yonah) may not have "Core" in their name, so we will try to
            // add it and a "Solo" or "Duo" suffix depending on the core count 
            else if (cpu->model == 14 && (cpu->cores > 0 || cpu->index > 0))
            {
                if (strstr(cpu->name, "Intel(R) CPU") && (cpu->cores > 0 || cpu->index > 0))
                {
                    char *tmp = NULL;
                    if (cpu->cores == 1 || cpu->index == 1)
                        tmp = findReplace(cpu->name, NAME_LEN, "CPU           T", "Core Solo T");
                    else if (cpu->cores == 2 || cpu->index == 2)
                        tmp = findReplace(cpu->name, NAME_LEN, "CPU           T", "Core Duo T");

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

    // If the processor count is higher than the thread count, it's likely
    // we're dealing with a multi-CPU configuration
    if (cpu->index > cpu->threads && cpu->threads > 0)
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
