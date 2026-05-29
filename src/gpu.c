/*
    ######################################################
    ##            SHORK UTILITY - SHORKFETCH            ##
    ######################################################
    ## Functions and data relating to handling GPUs/    ##
    ## graphics cards                                   ##
    ######################################################
    ## Licence: GNU GENERAL PUBLIC LICENSE Version 3    ##
    ######################################################
    ## Kali (links.sharktastica.co.uk)                  ##
    ######################################################
*/



#include "exclusions.h"
#include "general.h"
#include "globals.h"
#include "gpu.h"
#include "replacements.h"

#include <dirent.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>



/**
 * Cleans a GPU's name so it is less needlessly verbose and 'to the point'.
 * @param vendor Input vendor name string
 * @param device Input device name string
 * @param isGPUFromCPU Flags if the GPU name being cleaned is one extracted
 *                     from a CPU name
 * @return String containing the result after cleaning
 */
char *cleanGPUName(const char *vendor, const char *device, const int isGPUFromCPU)
{
    if (!vendor || !device) return strdup("");

    // Prepare result strings
    const size_t RESULT_SIZE = (GPU_NAME_LEN * 2) + 1;
    char *result = malloc(RESULT_SIZE);
    if (!result) return strdup("");
    char *cleanedVendor = NULL;
    char *cleanedDevice = strdup(device);
    char *cleanedDeviceNorm = NULL;
    char *cleanedDeviceBrac = NULL;



    // Shorten " / " to "/"
    if (strstr(cleanedDevice, " / "))
    {
        char *tmp = findReplace(cleanedDevice, GPU_NAME_LEN, " / ", "/");
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
    if (vendor[0] == 'A' && (strncmp(vendor, "AMD", 3) == 0 || strncmp(vendor, "Advanced Micro", 14) == 0))
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
                snprintf(cleanedDevice, GPU_NAME_LEN, "%s (%s)", cleanedDeviceBrac, cleanedDeviceNorm);
            // Otherwise, we just use the bracketed info
            else
                snprintf(cleanedDevice, GPU_NAME_LEN, "%s", cleanedDeviceBrac);
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
            char *tmp = findReplace(cleanedDevice, GPU_NAME_LEN, " Series/Vega Mobile Series", "/Vega Mobile");
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
            snprintf(cleanedDevice, GPU_NAME_LEN, "%s", cleanedDeviceBrac);

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
            snprintf(cleanedDevice, GPU_NAME_LEN, "%s", cleanedDeviceBrac);
    }
    // 3Dfx Interactive, Inc.
    else if (vendor[0] == '3' && strncmp(vendor, "3Dfx", 4) == 0)
    {
        cleanedVendor = strdup("3Dfx");

        // If this Voodoo is probably a Velocity (entry-level)
        if (cleanedDeviceBrac && cleanedDeviceBrac[0] == 'V')
            snprintf(cleanedDevice, GPU_NAME_LEN, "%s", cleanedDeviceBrac);

        // E.g., Voodoo 4/Voodoo 5 -> Voodoo 4/5
        if (strstr(cleanedDevice, "/Voodoo "))
        {
            char *tmp = findReplace(cleanedDevice, GPU_NAME_LEN, "/Voodoo ", "/");
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
                snprintf(cleanedDevice, GPU_NAME_LEN, "%s", cleanedDeviceNorm);

            // Remove space between "GD" and model number
            if (cleanedDevice[0] == 'G' && cleanedDevice[1] == 'D' && cleanedDevice[2] == ' ')
            {
                char *tmp = findReplace(cleanedDevice, GPU_NAME_LEN, "GD ", "GD");
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
            snprintf(cleanedDevice, GPU_NAME_LEN, "%s", cleanedDeviceBrac);
    }
    // S3 Graphics Ltd.
    else if (vendor[0] == 'S' && strncmp(vendor, "S3 ", 3) == 0)
    {
        cleanedVendor = strdup("S3 Graphics");

        // If we have bracketed info, we discard the norm (usually just
        // containing the core name)
        if (cleanedDeviceBrac)
            snprintf(cleanedDevice, GPU_NAME_LEN, "%s", cleanedDeviceBrac);
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
        // Apply generic deletions to vender name
        cleanedVendor = strdup(vendor);
        for (size_t i = 0; i < DELETIONS_LEN; i++)
        {
            const char *pattern = DELETIONS[i];
            char *tmp = findErase(cleanedVendor, GPU_NAME_LEN, pattern);
            free(cleanedVendor);
            cleanedVendor = tmp;
        }
    }

    // Apply generic deletions to device name
    for (size_t i = 0; i < DELETIONS_LEN; i++)
    {
        const char *pattern = DELETIONS[i];
        char *tmp = findErase(cleanedDevice, GPU_NAME_LEN, pattern);
        free(cleanedDevice);
        cleanedDevice = tmp;
    }

    // Combine and return final result
    snprintf(result, RESULT_SIZE, "%s %s", cleanedVendor, cleanedDevice);

    // Compact mode specific cleaning
    if (COMPACT && !isGPUFromCPU)
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
 * @param count Number of GPUs actually detected (intended to be used by reference)
 * @return Pointer to up to 4 GPU_IDS structs containing detected GPUs
 */
GPU_IDS* getGPUs(int *count)
{
    if (!count) return NULL;

    DIR *dir = opendir("/sys/bus/pci/devices");
    if (!dir)
    {
        *count = 0;
        return NULL;
    }

    struct dirent *entry;
    GPU_IDS *gpus = malloc(4 * sizeof(GPU_IDS));
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
 * @param gpu GPU_IDS struct containing detected vendor and device IDs and
 *            revision number
 * @param os String containing the OS name (used for OS-specific checks)
 * @return String containing the GPU's assembled and cleaned full name; vendor
 *         and device IDs as hex if interpreting failed
 */
char *interpretGPU(GPU_IDS *gpu, const char *os)
{
    char *gpuStr = malloc(GPU_NAME_LEN);
    if (!gpuStr) return strdup("unknown");
    gpuStr[0] = '\0';



    // If Intel GPU, query our pre-defined iGPU list
    if (gpu->vendor == 0x8086)
    {
        const char *name = INTEL_IGPUS[gpu->device];
        if (name)
        {
            char *tmp = cleanGPUName("Intel", name, 0);
            strncpy(gpuStr, tmp, GPU_NAME_LEN - 1);
            gpuStr[GPU_NAME_LEN - 1] = '\0';
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
                        char *tmp = cleanGPUName("Advanced Micro", name, 0);
                        strncpy(gpuStr, tmp, GPU_NAME_LEN - 1);
                        gpuStr[GPU_NAME_LEN - 1] = '\0';
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
    else if (os && strstr(os, "NixOS") != NULL)
    {
        DIR *store = opendir("/nix/store");
        if (store)
        {
            static char nixPciIds[PATH_MAX];
            struct dirent *entry;
            while ((entry = readdir(store)) != NULL)
            {
                snprintf(nixPciIds, PATH_MAX, "/nix/store/%s/share/hwdata/pci.ids", entry->d_name);
                if (access(nixPciIds, F_OK) == 0)
                {
                    pciids = nixPciIds;
                    break;
                }
            }
            closedir(store);
        }
    }
    else
    {
        snprintf(gpuStr, GPU_NAME_LEN, "%04x:%04x", gpu->vendor, gpu->device);
        return gpuStr;
    }

    FILE *fStream = fopen(pciids, "r");
    if (!fStream)
    {
        snprintf(gpuStr, GPU_NAME_LEN, "%04x:%04x", gpu->vendor, gpu->device);
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
        snprintf(gpuStr, GPU_NAME_LEN, "%04x:%04x", gpu->vendor, gpu->device);
    else
        snprintf(gpuStr, GPU_NAME_LEN, "%s", cleanGPUName(vendor, device, 0));

    if (vendor) free(vendor);
    if (device) free(device);

    return gpuStr;
}
