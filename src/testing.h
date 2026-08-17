/*
    ######################################################
    ##            SHORK UTILITY - SHORKFETCH            ##
    ######################################################
    ## Testing functions used when compiled with        ##
    ## TESTS=1                                          ##
    ######################################################
    ## Licence: GNU GENERAL PUBLIC LICENSE Version 3    ##
    ######################################################
    ## Kali (links.sharktastica.co.uk)                  ##
    ######################################################
*/



#ifdef TESTS

#include "cpu.h"
#include "gpu.h"

#include <ctype.h>
#include <dirent.h>
#include <linux/limits.h>



/**
 * Tests the getCPU (and by extension the cleanCPUName) function to ensure they
 * intepret our a set ofcpuinfo examples and extract a GPU name if present in
 * the CPU name
 */
void testGetCPU(void)
{
    DIR *testingDir = opendir("cpuinfo-ds");
    if (!testingDir) return;

    printf("##################\n");
    printf("## GET CPU TEST ##\n");
    printf("##################\n");

    const int MAX_CPUINFOS = 500;
    const int MAX_CPUINFO_PATH_LEN = 512;
    char cpuinfos[MAX_CPUINFOS][MAX_CPUINFO_PATH_LEN];
    int count = 0;
    int showRaw = 0;

    struct dirent *dirEntry;
    while ((dirEntry = readdir(testingDir)) != NULL)
    {
        if (count == MAX_CPUINFOS)
            break;

        if (dirEntry->d_type == DT_DIR)
        {
            if (strcmp(dirEntry->d_name, ".") == 0 || strcmp(dirEntry->d_name, "..") == 0 ||
                strcmp(dirEntry->d_name, "excluded") == 0)
                continue;

            char subPath[280];
            snprintf(subPath, sizeof(subPath), "cpuinfo-ds/%s", dirEntry->d_name);
            DIR *subDir = opendir(subPath);
            if (!subDir) continue;

            struct dirent *subEntry;
            while ((subEntry = readdir(subDir)) != NULL && count < MAX_CPUINFOS)
            {
                const char *ext = strrchr(subEntry->d_name, '.');
                if (ext == NULL || strcmp(ext, ".cpuinfo") != 0)
                    continue;
                snprintf(cpuinfos[count++], MAX_CPUINFO_PATH_LEN, "%s/%s", dirEntry->d_name, subEntry->d_name);
            }
            closedir(subDir);
            continue;
        }

        const char *ext = strrchr(dirEntry->d_name, '.');
        if (ext == NULL || strcmp(ext, ".cpuinfo") != 0)
            continue;
        snprintf(cpuinfos[count++], MAX_CPUINFO_PATH_LEN, "%s", dirEntry->d_name);
    }
    closedir(testingDir);

    qsort(cpuinfos, count, sizeof(cpuinfos[0]), natCmp);
    for (int i = 0; i < count; i++)
    {
        char cpuinfo[MAX_CPUINFO_PATH_LEN + 11];
        snprintf(cpuinfo, MAX_CPUINFO_PATH_LEN + 11, "cpuinfo-ds/%s", cpuinfos[i]);

        char bName[MAX_CPUINFO_PATH_LEN];
        strncpy(bName, cpuinfos[i], sizeof(bName) - 1);
        bName[sizeof(bName) - 1] = '\0';
        char *slash = strrchr(bName, '/');
        char *base = slash ? slash + 1 : bName;
        char *dot = strrchr(base, '.');
        if (dot) *dot = '\0';

        char *gpuFromCPU = NULL;
        CPU_DATA *cpu = getCPU(cpuinfo, &gpuFromCPU);
        char *cpuStr = interpretCPU(cpu);

        if (!showRaw)
        {
            int maxLeft = 41;
            int maxRight = 52;
            int colWidth = maxLeft + 1 + maxRight;

            int numCols;
            if (TERM_SIZE.ws_col >= colWidth * 5 + 4)
                numCols = 5;
            else if (TERM_SIZE.ws_col >= colWidth * 4 + 3)
                numCols = 4;
            else if (TERM_SIZE.ws_col >= colWidth * 3 + 2)
                numCols = 3;
            else if (TERM_SIZE.ws_col >= colWidth * 2 + 1)
                numCols = 2;
            else
                numCols = 1;

            char rightSide[512];
            if (gpuFromCPU)
                snprintf(rightSide, sizeof(rightSide), "%s (%s)", cpuStr, gpuFromCPU);
            else
                snprintf(rightSide, sizeof(rightSide), "%s", cpuStr ? cpuStr : "");

            printf("\033[31m%-*s\033[0m \033[32m%-*s\033[0m", maxLeft, base, (i + 1) % numCols == 0 || i + 1 == count ? 0 : maxRight, rightSide);
            if ((i + 1) % numCols == 0 || i + 1 == count)
                printf("\n");
            else
                printf("  ");
        }
        else
        {
            if (gpuFromCPU)
                printf("\033[31m%s:\033[0m \033[32m%s\033[0m \033[36m(%s)\033[0m\n", base, cpuStr, gpuFromCPU);
            else
                printf("\033[31m%s:\033[0m \033[32m%s\033[0m\n", base, cpuStr);

            if (cpu->arch == ARM)
                printf("    arch:               ARM\n");
            else if (cpu->arch == M68K)
                printf("    arch:               M68K\n");
            else if (cpu->arch == MIPS)
                printf("    arch:               MIPS\n");
            else if (cpu->arch == POWER)
                printf("    arch:               POWER\n");
            else if (cpu->arch == RISCV)
                printf("    arch:               RISCV\n");
            else if (cpu->arch == X86)
                printf("    arch:               X86\n");
            else
                printf("    arch:               UNKNOWN\n");
#ifndef X86_ONLY
            printf("    uarch:              %s\n", cpu->uarch ? cpu->uarch : "(null)");
#endif
            printf("    vendor:             %s\n", cpu->vendor ? cpu->vendor : "(null)");
#ifndef X86_ONLY
            printf("    platform:           %s\n", cpu->platform ? cpu->platform : "(null)");
            printf("    machine:            %s\n", cpu->machine ? cpu->machine : "(null)");
#endif
            printf("    name:               %s\n", cpu->name   ? cpu->name   : "(null)");
            printf("    family:             %d\n", cpu->family);
            printf("    model:              %d\n", cpu->model);
            printf("    stepping:           %d\n", cpu->stepping);
#ifndef X86_ONLY
            printf("    revisionNo:         %d\n", cpu->revisionNo);
            printf("    revisionStr:        %s\n", cpu->revisionStr);
#endif
            printf("    freq:               %.0f\n", cpu->freq);
            printf("    noUniquePhysIDs:    %d\n", cpu->physIDs.noUniquePhysIDs);
            printf("    maxPhysID:          %d\n", cpu->physIDs.maxPhysID);
            printf("    index:              %d\n", cpu->index);
            printf("    cores:              %d\n", cpu->cores);
            printf("    threads:            %d\n", cpu->threads);
            printf("    cacheSize:          %d\n", cpu->cacheSize);
            printf("    flags:              %s\n", cpu->flags);
            printf("    physAddrSize:       %d\n", cpu->physAddrSize);
            printf("    virtAddrSize:       %d\n", cpu->virtAddrSize);
        }

#ifndef X86_ONLY
        free(cpu->processor);
        free(cpu->uarch);
        free(cpu->platform);
        free(cpu->machine);
#endif
        free(cpu->vendor);
        free(cpu->name);
        free(cpu);
        free(gpuFromCPU);
    }
}

/**
 * Tests the interpretGPU function to ensure it finds and cleans GPU names as
 * we expect it to.
 */
void testInterpretGPU(void)
{
    printf("########################\n");
    printf("## INTERPRET GPU TEST ##\n");
    printf("########################\n");
    
    GPU_IDS gpus[] = {
        {
            "ATI 68800AX [Graphics Ultra Pro PCI]",
            0x1002,
            0x4158,
            0x00
        },
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
            "Tahiti XT GL [FirePro W9000]",
            0x1002,
            0x6780,
            0x00
        },
        {
            "Tahiti [FirePro Series Graphics Adapter]",
            0x1002,
            0x6784,
            -1
        },
        {
            "Navi 33 [Radeon RX 7600/7600 XT/7600M XT/7600S/7700S / PRO W7600]",
            0x1002,
            0x7480,
            -1
        },
        {
            "Navi 33 [Radeon RX 7600/7600 XT/7600M XT/7600S/7700S / PRO W7600]",
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
        char *gpu = interpretGPU(&gpus[i], NULL);
        if (gpu && gpu[0] != '\0')
        {
            if (gpus[i].revision == -1)
                printf("%04x:%04x:--: \033[31m%s\033[0m -> \033[32m%s\033[0m\n", gpus[i].vendor, gpus[i].device, gpus[i].name, gpu);
            else
                printf("%04x:%04x:%02X: \033[31m%s\033[0m -> \033[32m%s\033[0m\n", gpus[i].vendor, gpus[i].device, gpus[i].revision, gpus[i].name, gpu);
        }
        free(gpu);
    }
}

/**
 * Tests the interpretScreen function to ensure it assembles screen specs
 * strings as we expect it to.
 */
void testInterpretScreen(void)
{
    printf("###########################\n");
    printf("## INTERPRET SCREEN TEST ##\n");
    printf("###########################\n");

    Screen screens[] = {
        // table.flip 34"
        {
            strdup("DP-3"),
            1,
            800.000000,
            335.000000,
            3440,
            1440,
            0
        },
        // SN-MAIN 34"
        {
            strdup("DP-4"),
            1,
            798.000000,
            334.000000,
            3440,
            1440,
            100
        },
        // table.flip - 27"
        {
            strdup("DP-4"),
            1,
            597.000000,
            336.000000,
            2560,
            1440,
            0
        },
        // W530
        {
            strdup("LVDS-1-1"),
            1,
            344.000000,
            193.000000,
            1920,
            1080,
            60
        },
        // R500
        {
            strdup("LVDS"),
            1,
            331.000000,
            207.000000,
            1650,
            1050,
            60
        },
        // L430
        {
            strdup("LVDS-1"),
            1,
            309.000000,
            174.000000,
            1366,
            768,
            60
        },
        // T480
        {
            strdup("eDP-1"),
            1,
            309.000000,
            173.000000,
            1920,
            1080,
            60
        }
    };
    const int noScreens = sizeof(screens) / sizeof(screens[0]);

    for (int i = 0; i < noScreens; i++)
    {
        char *screen = interpretScreen(&screens[i]);
        float diagMm = fSqrt(screens[i].physX * screens[i].physX + screens[i].physY * screens[i].physY);
        float diagIn = (float)diagMm / 25.4f;
        if (screen && screen[0] != '\0')
            printf("\033[31m%f\"\033[0m -> \033[32m%s\033[0m\n", diagIn, screen);
        free(screen);
    }
}

#endif
