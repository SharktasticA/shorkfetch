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
 * @param cpuInfo A file path to a cpuinfo file to read
 * @param gpuFromCPU A pointer to a string for returning an extracted GPU name
 * @return String containing the CPU's name and core/thread specs; empty string
 *         if unknown
 */
char *getCPU(char *cpuInfo, char **gpuFromCPU)
{
    CPUArch arch;

    char *result = malloc(134);
    char *cpu = malloc(128);
    char *vendor = malloc(16);
    char *implementer = malloc(16);
    char *isa = malloc(128);
    char *model = malloc(4);
    char *modelName = malloc(128);
    char *stepping = malloc(4);
    char *architecture = malloc(4);
    char *processor = malloc(5);
    char *cores = malloc(5);
    char *threads = malloc(5);
    char *hart = malloc(5);
    char *fpu = malloc(4);
    if (!result || !cpu || !vendor || !implementer || !isa || !model || !modelName || !stepping || !architecture || !processor || !cores || !threads || !hart || !fpu) 
    {
        free(result);
        free(cpu);
        free(vendor);
        free(implementer);
        free(isa);
        free(model);
        free(modelName);
        free(stepping);
        free(architecture);
        free(processor);
        free(cores);
        free(threads);
        free(hart);
        free(fpu);
        return NULL;
    }
    result[0] = cpu[0] = vendor[0] = implementer[0] = isa[0] = model[0] = modelName[0] = stepping[0] = architecture[0] = processor[0] = cores[0] = threads[0] = hart[0] = fpu[0] = '\0';



    FILE *fStream = fopen(cpuInfo, "r");
    if (fStream)
    {
        // Use these to stop parsing once we have everything we need!
        // lookingFor's default value is x86 orientated - the ARM-based path
        // can change this to 2, hence not a const.
        int lookingFor = 7;
        int parsed = 0;

        char buffer[256];
        while (fgets(buffer, sizeof(buffer), fStream) && parsed < lookingFor)
        {
            if (strncmp(buffer, "processor", 9) == 0)
            {
                char *extract = extractFromPoint(buffer, 5, ':', 2);
                if (extract)
                {
                    strncpy(processor, extract, 4);
                    processor[4] = '\0';
                    free(extract);
                }
            }
            else if (strncmp(buffer, "hart", 4) == 0)
            {
                char *extract = extractFromPoint(buffer, 5, ':', 2);
                if (extract)
                {
                    strncpy(hart, extract, 4);
                    hart[4] = '\0';
                    free(extract);
                }
            }
            else if (implementer[0] == '\0' && strncmp(buffer, "CPU implementer", 15) == 0)
            {
                char *extract = extractFromPoint(buffer, 16, ':', 2);
                if (extract)
                {
                    strncpy(implementer, extract, 15);
                    implementer[15] = '\0';
                    free(extract);
                }
            }
            else if (isa[0] == '\0' && strncmp(buffer, "isa", 3) == 0)
            {
                char *extract = extractFromPoint(buffer, 128, ':', 2);
                if (extract)
                {
                    strncpy(isa, extract, 127);
                    isa[127] = '\0';
                    free(extract);
                }
            }
            else if (vendor[0] == '\0' && strncmp(buffer, "vendor_id", 9) == 0)
            {
                char *extract = extractFromPoint(buffer, 16, ':', 2);
                if (extract)
                {
                    strncpy(vendor, extract, 15);
                    vendor[15] = '\0';
                    free(extract);
                }
                parsed++;
            }
            else if (modelName[0] == '\0' && strncmp(buffer, "model name", 10) == 0)
            {
                char *extract = extractFromPoint(buffer, 128, ':', 2);
                if (extract)
                {
                    strncpy(modelName, extract, 127);
                    modelName[127] = '\0';
                    free(extract);
                }
                parsed++;
            }
            else if (model[0] == '\0' && strncmp(buffer, "model", 5) == 0)
            {
                char *extract = extractFromPoint(buffer, 4, ':', 2);
                if (extract)
                {
                    strncpy(model, extract, 3);
                    model[3] = '\0';
                    free(extract);
                }
                parsed++;
            }
            else if (stepping[0] == '\0' && strncmp(buffer, "stepping", 8) == 0)
            {
                char *extract = extractFromPoint(buffer, 4, ':', 2);
                if (extract)
                {
                    strncpy(stepping, extract, 3);
                    stepping[3] = '\0';
                    free(extract);
                }
                parsed++;
            }
            else if (architecture[0] == '\0' && strncmp(buffer, "CPU architecture", 16) == 0)
            {
                char *extract = extractFromPoint(buffer, 4, ':', 2);
                if (extract)
                {
                    strncpy(architecture, extract, 3);
                    architecture[3] = '\0';
                    free(extract);
                }
                lookingFor = 2;
            }
            else if (cores[0] == '\0' && strncmp(buffer, "cpu cores", 9) == 0)
            {
                char *extract = extractFromPoint(buffer, 5, ':', 2);
                if (extract)
                {
                    strncpy(cores, extract, 4);
                    cores[4] = '\0';
                    free(extract);
                }
                parsed++;
            }
            else if (cpu[0] == '\0' && strncmp(buffer, "cpu", 3) == 0)
            {
                char *extract = extractFromPoint(buffer, 128, ':', 2);
                if (extract)
                {
                    strncpy(cpu, extract, 127);
                    cpu[127] = '\0';
                    free(extract);
                }
            }
            else if (threads[0] == '\0' && strncmp(buffer, "siblings", 8) == 0)
            {
                char *extract = extractFromPoint(buffer, 5, ':', 2);
                if (extract)
                {
                    strncpy(threads, extract, 4);
                    threads[4] = '\0';
                    free(extract);
                }
                parsed++;
            }
            else if (fpu[0] == '\0' && strncmp(buffer, "fpu", 3) == 0)
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



        // Typical x86 path
        if (vendor[0] != '\0' || modelName[0] != '\0')
        {
            arch = X86;

            // Check if model name lacks the vendor name and if we need to try adding it in manually
            if ((vendor[0] != '\0' && vendor[0] != 'u') && (modelName[0] != '\0' && modelName[0] != 'u'))
            {
                if (!strstr(modelName, "Intel") && !strstr(modelName, "AMD") && !strstr(modelName, "Cyrix") && !strstr(modelName, "IDT") && !strstr(modelName, "VIA") && !strstr(modelName, "Transmeta"))
                {
                    char *tmp = malloc(128);
                    if (tmp)
                    {
                        if (strstr(vendor, "Intel") || strstr(vendor, "Iotel"))
                            snprintf(tmp, 128, "%s %s", "Intel", modelName);
                        else if (strstr(vendor, "AMD"))
                            snprintf(tmp, 128, "%s %s", "AMD", modelName);
                        else if (strstr(vendor, "Cyrix"))
                            snprintf(tmp, 128, "%s %s", "Cyrix", modelName);
                        else if (strstr(vendor, "Centaur"))
                            snprintf(tmp, 128, "%s %s", "IDT/Centaur", modelName);
                        else if (strstr(vendor, "VIA"))
                            snprintf(tmp, 128, "%s %s", "VIA", modelName);
                        else if (strstr(vendor, "Transmeta") || strstr(vendor, "TM"))
                            snprintf(tmp, 128, "%s %s", "Transmeta", modelName);
                        else
                            snprintf(tmp, 128, "%s %s", vendor, modelName);
                        
                        strncpy(modelName, tmp, 128);
                        free(tmp);
                        modelName[127] = '\0';
                    }
                }
            }



            // If we have a K6, we will try to distinguish if it's a Model 6 or
            // Model 7
            if (strstr(modelName, "AMD-K6tm w"))
            {
                if (model[0] == '6')
                {
                    char tmp[128];
                    snprintf(tmp, 128, "AMD K6 Model 6", modelName);
                    strncpy(modelName, tmp, 127);
                    modelName[127] = '\0';
                }
                else if (model[0] == '7')
                {
                    char tmp[128];
                    snprintf(tmp, 128, "AMD K6 Model 7", modelName);
                    strncpy(modelName, tmp, 127);
                    modelName[127] = '\0';
                }
            }

            // If we have a supposed K6-III, it may actually be a K6-2+ or
            // K6-III+, and we may be able to tell from the stepping
            if (strstr(modelName, "AMD-K6(tm)-III P"))
            {
                if (stepping[0] == '0')
                {
                    char tmp[128];
                    snprintf(tmp, 128, "AMD K6-III+", modelName);
                    strncpy(modelName, tmp, 127);
                    modelName[127] = '\0';
                }
                else if (stepping[0] == '4')
                {
                    char tmp[128];
                    snprintf(tmp, 128, "AMD K6-2+", modelName);
                    strncpy(modelName, tmp, 127);
                    modelName[127] = '\0';
                }
            }

            // If we have a Cx486Dxxx with FPU, make sure 387 is included in
            // the model name
            if ((strstr(modelName, "Cx486DLC") || strstr(modelName, "Cx486DRx2")) && fpu[0] == '1')
            {
                char tmp[128];
                snprintf(tmp, 128, "%s + 387", modelName);
                strncpy(modelName, tmp, 127);
                modelName[127] = '\0';
            }

            // If we have a Cx486S with FPU, make sure 487 is included in the
            // model name
            if (strstr(modelName, "Cx486S") && fpu[0] == '1')
            {
                char tmp[128];
                snprintf(tmp, 128, "%s + 487", modelName);
                strncpy(modelName, tmp, 127);
                modelName[127] = '\0';
            }

            // If we have a supposed WinChip 2-3D, we may be able to tell if
            // its a WinChip 2A from the stepping
            if (strstr(modelName, "WinChip 2-3D") && stepping[0] == '7')
            {
                char tmp[128];
                snprintf(tmp, 128, "IDT WinChip 2A", modelName);
                strncpy(modelName, tmp, 127);
                modelName[127] = '\0';
            }

            // If we have for certain an Intel 486SX with FPU, make sure 487 is
            // included in the model name
            if (strstr(modelName, "486") && strstr(modelName, "SX") && fpu[0] == '1')
            {
                char tmp[128];
                snprintf(tmp, 128, "%s + 487", modelName);
                strncpy(modelName, tmp, 127);
                modelName[127] = '\0';
            }

            // If we have a vendorless and revisionless 486, we can at least
            // infer if its purely 486SX, or a 486DX, 487SX (true 486SX +
            // 487SX) or 486SX + 387 (eg, IBM 486BLx/486SLCx  + 387), from the
            // presence of an FPU
            if ((vendor[0] == '\0' || vendor[0] == 'u') && modelName[0] != '\0' && strcmp(modelName, "486") == 0)
            {
                if (fpu[0] == '0')
                    snprintf(modelName, 127, "486SX");
                else if (fpu[0] == '1')
                    snprintf(modelName, 127, "486DX/487SX/486SX + 387");
            }
        }
        // Possible ARM CPU path
        else if (architecture[0] != '\0')
        {
            arch = ARM;

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
                snprintf(modelName, 128, "%s Armv%d", implementerName, atoi(architecture));
            else
                snprintf(modelName, 128, "Armv%d", atoi(architecture));
        }
        // Possible POWER CPU path
        else if (cpu[0] == 'P' && modelName[0] == '\0')
        {
            arch = POWER;

            strncpy(modelName, cpu, 127);
            modelName[127] = '\0';

            // In cases like "POWER9, altivec supported", we want to remove the
            // comma and everything after
            char *comma = strchr(modelName, ',');
            if (comma) *comma = '\0';
        }
        // RISC-V CPU path
        else if (isa[0] == 'r' && isa[1] == 'v')
        {
            arch = RISCV;
            snprintf(modelName, 128, "RISC-V");
        }
        // Leave if have nothing to show...
        else if (cores[0] == '\0' && threads[0] == '\0' && processor[0] == '\0')
        {
            free(result);
            free(cpu);
            free(vendor);
            free(implementer);
            free(isa);
            free(model);
            free(modelName);
            free(stepping);
            free(architecture);
            free(processor);
            free(cores);
            free(threads);
            free(hart);
            free(fpu);
            return NULL;
        }



        // If we don't have a cores value, set it to the same as threads
        // so we don't try to show them separately later
        if (cores[0] == '\0' && threads[0] != '\0')
            strncpy(cores, threads, 3);

        char coresAndThreads[16] = "";

        // For most arches, we work with cores, threads and fallback processor
        // count
        if (arch != RISCV)
        {
            // If we don't have cores or threads, we use the processor field in its
            // place
            if (cores[0] == '\0' && threads[0] == '\0' && processor[0] != '\0')
            {
                int processorInt = atoi(processor);
                processorInt++;

                // We don't have a good way to tell cores from threads for POWER
                // CPUs at the moment, so let's not imply our value is for cores
                if (arch == POWER)
                    snprintf(coresAndThreads, 16, "%dT", processorInt);
                else
                    snprintf(coresAndThreads, 16, "%dC", processorInt);
            }
            // If cores and threads are the same value, just show cores
            else if (strcmp(cores, threads) == 0)
                snprintf(coresAndThreads, 16, "%sC", cores);
            // If cores and threads are different values, show both
            else
                snprintf(coresAndThreads, 16, "%sC/%sT", cores, threads);
        }
        // For RISC-V specifically, we have processor and hart (hardware
        // thread) to work with
        else
        {
            // If we have processor...
            if (processor[0] != '\0')
            {
                int processorInt = atoi(processor);
                processorInt++;

                // ...and hart
                if (hart[0] != '\0')
                {
                    int hartInt = atoi(hart);
                    // The SiFive Freedom U540 (etc.) count from 1 instead of 0...
                    if (hartInt % 2 != 0)
                        hartInt++;

                    // If processors and hart are the same value, just show
                    // processors
                    if (processorInt == hartInt || hartInt == 0)
                        snprintf(coresAndThreads, 16, "%dC", processorInt);
                    // If processors and hart are different values, show both
                    else
                        snprintf(coresAndThreads, 16, "%dC%dT", processorInt, hartInt);
                }
                // ...only
                else
                    snprintf(coresAndThreads, 16, "%dC", processorInt);
            }
            // If we only have hart...
            else if (hart[0] != '\0')
            {
                int hartInt = atoi(hart);
                // The SiFive Freedom U540 (etc.) count from 1 instead of 0...
                if (hartInt % 2 != 0)
                    hartInt++;
                snprintf(coresAndThreads, 16, "%dC", hartInt);
            }
        }



        // If the model name has GPU name in it, we will extract it and save if
        // for later in case we need it as a fallback when GPU detection fails
        // or produces no results
        const char *gpuNeedles[] = { "with Radeon", "w/ Radeon", "with GeForce", "w/ GeForce" };
        for (int i = 0; i < 4; i++)
        {
            char *found = strstr(modelName, gpuNeedles[i]);
            if (found)
            {
                // Save it for later
                *gpuFromCPU = malloc(134);
                if (*gpuFromCPU)
                {
                    const char *gpuVendor = (i < 2) ? "AMD " : "NVIDIA ";
                    snprintf(*gpuFromCPU, 134, "%s%s", gpuVendor, found + (strchr(gpuNeedles[i], ' ') - gpuNeedles[i]) + 1);
                }

                // Remove it from model name
                *found = '\0';

                // Make sure there isn't any trailing nonsense left
                char *end = found - 1;
                while (end > modelName && (*end == ' ' || *end == ',' || *end == '-'))
                    *end-- = '\0';
            }
        }



        // If we have no model name but we have core/thread count, just show
        // the latter
        if (modelName[0] == '\0' && coresAndThreads[0] != '\0')
            strncpy(result, coresAndThreads, 133);
        // If we're in compact mode, we just show the model name
        else if (COMPACT)
            strncpy(result, modelName, 133);
        // Normal view
        else
        {
            if (coresAndThreads[0] != '\0')
                snprintf(result, 134, "%s (%s)", modelName, coresAndThreads);
            else
                strncpy(result, modelName, 133);
        }
    }
    else 
    {
        free(result);
        free(cpu);
        free(vendor);
        free(implementer);
        free(isa);
        free(model);
        free(modelName);
        free(stepping);
        free(architecture);
        free(processor);
        free(cores);
        free(threads);
        free(hart);
        free(fpu);
        return NULL;
    }



    int coreCount = 0;
    if (cores && cores[0] != '\0')
        coreCount = (int)strtol(cores, NULL, 10);

    free(cpu);
    free(vendor);
    free(implementer);
    free(isa);
    free(model);
    free(modelName);
    free(stepping);
    free(architecture);
    free(processor);
    free(cores);
    free(threads);
    free(hart);
    free(fpu);

    char *cleanedCPU = cleanCPUName(result, 134, coreCount);
    strncpy(result, cleanedCPU, 133);
    free(cleanedCPU);

    return result;
}
