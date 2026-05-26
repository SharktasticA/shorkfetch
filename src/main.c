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



static const char *VERSION = "0.3-wip";



#include "art.h"
#include "colours.h"
#include "cpu.h"
#include "de-wm.h"
#include "disk.h"
#include "general.h"
#include "globals.h"
#include "gpu.h"
#include "hostname.h"
#include "ip.h"
#include "kernel.h"
#include "memory.h"
#include "os.h"
#include "packages.h"
#include "screen.h"
#include "shell.h"
#include "terminal.h"
#include "testing.h"
#include "uptime.h"
#include "username.h"



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

    char noCol[100] = "-nc, --no-col      Disables all coloured output (if compiled with colour support)\n";
    formatNewLines(noCol, TERM_SIZE.ws_col, "                   ", 0);
    printf("%s", noCol);

    char noIP[100] = "-ni, --no-ip       Hides the local IP field\n";
    formatNewLines(noIP, TERM_SIZE.ws_col, "                   ", 0);
    printf("%s", noIP);

    char version[100] = "-v, --version      Displays version number and exits\n\n";
    formatNewLines(version, TERM_SIZE.ws_col, "                   ", 0);
    printf("%s", version);

    char fieldNames[100] = "Field names:\nos, krn, upt, pkgs, scn, de, wm, trm, sh, cpu, gpu, ram, swap, root, lip\n";
    formatNewLines(fieldNames, TERM_SIZE.ws_col, NULL, 0);
    printf("%s", fieldNames);
}



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
        if ((strcmp(argv[i], "-v") == 0) || (strcmp(argv[i], "--version") == 0))
        {
            printf("shorkfetch %s\n", VERSION);
            return 0;
        }
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
    testInterpretScreen();
    testInterpretGPU();
    testGetCPU();
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

    char *gpuFromCPU = NULL;
    CPU_DATA *cpu = showCPU ? getCPU("/proc/cpuinfo", &gpuFromCPU) : NULL;
    int noGPUs = 0;
    GPU_IDS *gpus = showGPU ? getGPUs(&noGPUs) : NULL;
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

    if (cpu)
    {
        char *cpuStr = interpretCPU(cpu);

        if (cpuStr && cpuStr[0] != '\0')
        {
            if (showShork) printf("%s%s%s", colAccent, SHORK[shorkLine++], colReset);
            if (!useBullets)
            {
                if (!COMPACT)
                    printf("%sCPU:%s      %s\n", colAccent, colReset, cpuStr);
                else
                    printf("%sCPU:%s %s\n", colAccent, colReset, cpuStr);
            }
            else printf(" %s%c%s %s\n", colAccent, bullet, colReset, cpuStr);
        }

        free(cpuStr);
    }

    if (gpus && noGPUs > 0)
    {
        int pastFirstGPU = 0;
        for (int i = 0; i < noGPUs; i++)
        {
            char *gpuStr = interpretGPU(&gpus[i]);

            if (gpuStr && gpuStr[0] != '\0')
            {
                if (showShork) printf("%s%s%s", colAccent, SHORK[shorkLine++], colReset);
                if (!useBullets)
                {
                    if (!COMPACT)
                    {
                        if (noGPUs == 1)
                            printf("%sGPU:%s      %s\n", colAccent, colReset, gpuStr);
                        else if (!pastFirstGPU)
                            printf("%sGPUs:%s     %s\n", colAccent, colReset, gpuStr);
                        else
                            printf("          %s\n", gpuStr);
                    }
                    else
                    {
                        if (noGPUs == 1 || !pastFirstGPU)
                            printf("%sGPU:%s %s\n", colAccent, colReset, gpuStr);
                        else
                            printf("     %s\n", gpuStr);
                    }
                }
                else printf(" %s%c%s %s\n", colAccent, bullet, colReset, gpuStr);
            }

            free(gpuStr);
            pastFirstGPU = 1;
        }
    }
    // If we found no GPUs the "traditional" way, at least check if we received
    // a fallback found during CPU name processing
    else if (gpuFromCPU && gpuFromCPU[0] != '\0')
    {
        if (showShork) printf("%s%s%s", colAccent, SHORK[shorkLine++], colReset);
        if (!useBullets)
        {
            if (!COMPACT)
                printf("%sGPU:%s      %s\n", colAccent, colReset, gpuFromCPU);
            else
                printf("%sGPU:%s %s\n", colAccent, colReset, gpuFromCPU);
        }
        else printf(" %s%c%s %s\n", colAccent, bullet, colReset, gpuFromCPU);
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

    free(gpuFromCPU);
    if (cpu) 
    {
        free(cpu->uarch);
        free(cpu->vendor);
        free(cpu->name);
        free(cpu);
    }
    if (gpus) free(gpus);
    free(root);

    if (!noIP) free(localIP);

    return 0;
}
