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



static const char *VERSION = "0.4.1-wip";



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
#include "shorkconf.h"
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

    char bullets[130] = "-b, --bullets   Uses bullet points instead of field headings; can also be used to specify a custom character\n";
    formatNewLines(bullets, TERM_SIZE.ws_col, "                ", 0);
    printf("%s", bullets);

    char colour[100] = "-cl, --colour   Specifies a custom accent colour; no assignment returns the current colour\n";
    formatNewLines(colour, TERM_SIZE.ws_col, "                ", 0);
    printf("%s", colour);

    char compact[100] = "-co, --compact  Compacts field names (if not using bullets) and field values\n";
    formatNewLines(compact, TERM_SIZE.ws_col, "                ", 0);
    printf("%s", compact);

    char help[70] = "-h, --help      Displays help information and exits\n";
    formatNewLines(help, TERM_SIZE.ws_col, "                ", 0);
    printf("%s", help);

    char fields[150] = "-f, --fields    Specifies a custom fields list and order; no assignment returns list of current fields\n";
    formatNewLines(fields, TERM_SIZE.ws_col, "                ", 0);
    printf("%s", fields);

    char noArt[100] = "-na, --no-art   Disables the SHORK ASCII art\n";
    formatNewLines(noArt, TERM_SIZE.ws_col, "                ", 0);
    printf("%s", noArt);

    char reset[80] = "-r, --reset     Resets to default, deletes configuration file and exits\n";
    formatNewLines(reset, TERM_SIZE.ws_col, "                ", 0);
    printf("%s", reset);

    char save[100] = "-s, --save      Saves chosen options to a custom configuration file\n";
    formatNewLines(save, TERM_SIZE.ws_col, "                ", 0);
    printf("%s", save);

    char version[100] = "-v, --version   Displays version number and exits\n\n";
    formatNewLines(version, TERM_SIZE.ws_col, "                ", 0);
    printf("%s", version);

    char colours[180] = "Colours: black, blue, bold_blue, bold_cyan, bold_green, bold_magenta, bold_red, bold_white, bold_yellow, cyan, green, grey, magenta, red, white, yellow, off\n\n";
    formatNewLines(colours, TERM_SIZE.ws_col, NULL, 0);
    printf("%s", colours);

    char fieldNames[200] = "Fields: os, krn, upt, pkgs, scn, de, wm, trm, sh, cpu, gpu, ram, swap, root, lip, clrs, --- (category separator), single blank space (new line)\n";
    formatNewLines(fieldNames, TERM_SIZE.ws_col, NULL, 0);
    printf("%s", fieldNames);
}



int main(int argc, char *argv[])
{
    COLOUR = strdup("bold_cyan");
    HOME =  getenv("HOME");
    TERM_SIZE = getTerminalSize();

    char bullet = '*';
    char *fields = strdup("os,krn,upt,pkgs,scn,de,wm,trm,sh,cpu,gpu,ram,swap,root,lip, ,clrs, ");
    int noIP = 0;
    int saveConf = 0;
    int shorkLine = 0;
#ifdef NO_ART
    int showShork = 0;
#else
    int showShork = 1;
#endif
    int useBullets = 0;

    readConf(&bullet, &COLOUR, &COMPACT, &fields, &noIP, &showShork, &useBullets);

    for (int i = 1; i < argc; i++)
    {
        if ((strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "--help") == 0))
        {
            showHelp();
            free(COLOUR);
            free(fields);
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
                    free(fields);
                    return 1;
                }
                else if (bulletChar[1] != '\0')
                {
                    printf("ERROR: custom bullet point character can only be a single character\n");
                    free(fields);
                    return 1;
                }
                bullet = bulletChar[0];
            }
        }
        else if (strncmp(argv[i], "-cl", 3) == 0 || strncmp(argv[i], "--colour", 8) == 0)
        {
            // Find "=" as our needle
            char *equalsNeedle = strchr(argv[i], '=');
            if (!equalsNeedle) 
            {
                printf("%s\n", COLOUR);
                free(COLOUR);
                free(fields);
                return 1;
            }

            free(COLOUR);
            equalsNeedle++;
            COLOUR = strdup(equalsNeedle);
        }
        else if ((strcmp(argv[i], "-co") == 0) || (strcmp(argv[i], "--compact") == 0))
            COMPACT = 1;
        else if (strncmp(argv[i], "-f", 2) == 0 || strncmp(argv[i], "--fields", 8) == 0)
        {
            // Find "=" as our needle
            char *equalsNeedle = strchr(argv[i], '=');
            if (!equalsNeedle) 
            {
                printf("\"%s\"\n", fields);
                free(COLOUR);
                free(fields);
                return 1;
            }

            equalsNeedle++;
            free(fields);
            fields = strdup(equalsNeedle);

            // Remove trailing comma if present
            size_t len = strlen(fields);
            if (len > 0 && fields[len - 1] == ',')
                fields[len - 1] = '\0';
        }
        else if ((strcmp(argv[i], "-na") == 0) || (strcmp(argv[i], "--no-art") == 0))
            showShork = 0;
        else if ((strcmp(argv[i], "-ni") == 0) || (strcmp(argv[i], "--no-ip") == 0))
            noIP = 1;
        else if ((strcmp(argv[i], "-r") == 0) || (strcmp(argv[i], "--reset") == 0))
        {
            int result = deleteConf();
            if (result)
                printf("SHORKFETCH configuration reset\n");
            else
                printf("WARNING: SHORKFETCH configuration already default\n");
            free(COLOUR);
            free(fields);
            return 0;
        }
        else if ((strcmp(argv[i], "-s") == 0) || (strcmp(argv[i], "--save") == 0))
            saveConf = 1;
        else if ((strcmp(argv[i], "-v") == 0) || (strcmp(argv[i], "--version") == 0))
        {
            printf("SHORKFETCH %s\n", VERSION);
            free(fields);
            free(COLOUR);
            return 0;
        }
        else
        {
            printf("ERROR: unrecognised option \"%s\"\n", argv[i]);
            free(fields);
            free(COLOUR);
            return 1;
        }
    }

    char *colAccent = validateColour(COLOUR);
    if (!colAccent)
    {
        printf("ERROR: unrecognised colour \"%s\"\n", COLOUR);
        free(COLOUR);
        free(fields);
        return 1;
    }
    char *colReset = (colAccent[0] == '\0') ? "" : "\033[" COL_RESET "m";

#ifdef TESTS
    testInterpretScreen();
    testInterpretGPU();
    testGetCPU();
    return 0;
#endif



    // Validate which field to display
    char *fieldsOrig = strdup(fields);
    char fieldsProcessed[MAX_FIELDS][5];
    int noFields = 0;
    if (fields && fields[0] != '\0')
    {
        char *currTok = strtok(fields, ",");
        while (currTok)
        {
            // Make sure current field is a known one
            int valid = 0;
            for (int i = 0; i < POSSIBLE_FIELDS_LEN; i++)
            {
                if (strcmp(currTok, POSSIBLE_FIELDS[i]) == 0)
                {
                    valid = 1;
                    break;
                }
            }

            if (valid)
            {
                if (noFields >= MAX_FIELDS)
                {
                    printf("ERROR: too many fields given (max %d)\n", MAX_FIELDS);
                    free(COLOUR);
                    free(fields);
                    return 1;
                }

                // Queue this field up
                strncpy(fieldsProcessed[noFields], currTok, 4);
                fieldsProcessed[noFields][4] = '\0';
                noFields++;
            }
            else
            {
                printf("ERROR: unrecognised field name \"%s\"\n", currTok);
                free(COLOUR);
                free(fields);
                return 1;
            }

            currTok = strtok(NULL, ",");
        }
    }
    else
    {
        printf("ERROR: no field names were given to display\n");
        free(COLOUR);
        free(fields);
        return 1;
    }

    int showShorkOrig = showShork;
    if (noFields <= 6)
        showShork = 0;



    MemInfo mi = getMemInfo();

    struct utsname u;
    int uStatus = uname(&u);

    char *envWay = getenv("WAYLAND_DISPLAY");
    WAYLAND_PRESENT = (envWay != NULL && envWay[0] != '\0');
    char *envX11 = getenv("DISPLAY");
    X11_PRESENT = (envX11 != NULL && envX11[0] != '\0');
    if (WAYLAND_PRESENT || X11_PRESENT)
        XDG_CURRENT_DESKTOP = getenv("XDG_CURRENT_DESKTOP");



    // Print header
    char *username = getUsername();
    char *hostname = getHostname(u, uStatus);
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



    // Some things are dependent on others, so we have to look them up regardless
    char *os = getOS(u, uStatus);
    char *de = getDE();
    char *wm = getWM(&de);
    char *gpuFromCPU = NULL;
    CPU_DATA *cpu = getCPU("/proc/cpuinfo", &gpuFromCPU);



    for (int i = 0; i < noFields; i++)
    {
        if (strcmp(fieldsProcessed[i], " ") == 0)
        {
            if (showShork) printf("%s%s%s", colAccent, SHORK[shorkLine++], colReset);
            putchar('\n');
        }
        else if (strcmp(fieldsProcessed[i], "---") == 0)
        {
            if (showShork) printf("%s%s%s", colAccent, SHORK[shorkLine++], colReset);
            for (size_t i = 0; i < headerWidth; i++) printf("-");
            putchar('\n');
        }
        else if (strcmp(fieldsProcessed[i], "os") == 0)
        {
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
        }
        else if (strcmp(fieldsProcessed[i], "krn") == 0)
        {
            char *kernel = getKernel(u, uStatus);
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
            free(kernel);
        }
        else if (strcmp(fieldsProcessed[i], "upt") == 0)
        {
            char *uptime = getUptime();
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
            free(uptime);
        }
        else if (strcmp(fieldsProcessed[i], "pkgs") == 0)
        {
            char *pkgs = getPackages(os);
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
            free(pkgs);
        }
        else if (strcmp(fieldsProcessed[i], "scn") == 0)
        {
            int noScreens = 0;
            Screen *screens = getScreens(&noScreens);
            if (screens)
            {
                int pastFirstScreen = 0;
                for (int j = 0; j < noScreens; j++)
                {
                    char *screen = interpretScreen(&screens[j]);

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
                            }
                        }
                    }

                    free(screen);
                    pastFirstScreen = 1;
                }
                free(screens);
            }
        }
        else if (strcmp(fieldsProcessed[i], "de") == 0)
        {
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
        }
        else if (strcmp(fieldsProcessed[i], "wm") == 0)
        {
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
        }
        else if (strcmp(fieldsProcessed[i], "trm") == 0)
        {
            char *trm = getTerminal();
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
            // If we don't have a terminal name, we can at least still show the
            // console size
            else
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
            free(trm);
        }
        else if (strcmp(fieldsProcessed[i], "sh") == 0)
        {
            char *shell = getShell();
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
            free(shell);
        }
        else if (strcmp(fieldsProcessed[i], "cpu") == 0)
        {
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
        }
        else if (strcmp(fieldsProcessed[i], "gpu") == 0)
        {
            int noGPUs = 0;
            GPU_IDS *gpus = getGPUs(&noGPUs);
            if (gpus && noGPUs > 0)
            {
                int pastFirstGPU = 0;
                for (int j = 0; j < noGPUs; j++)
                {
                    char *gpuStr = interpretGPU(&gpus[j], os);

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
            free(gpus);
        }
        else if (strcmp(fieldsProcessed[i], "ram") == 0 && mi.memTotal > 0)
        {
            char *ram = getRAM(mi);
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
            free(ram);
        }
        else if (strcmp(fieldsProcessed[i], "swap") == 0 && mi.swapTotal > 0)
        {
            char *swap = getSwap(mi);
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
            free(swap);
        }
        else if (strcmp(fieldsProcessed[i], "root") == 0)
        {
            char *root = getRoot();
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
            free(root);
        }
        else if (strcmp(fieldsProcessed[i], "lip") == 0 && !noIP)
        {
            char *localIP = getLocalIP();
            if (localIP)
            {
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
                free(localIP);
            }
        }
        else if (strcmp(fieldsProcessed[i], "clrs") == 0)
        {
            ColourPalette palette = getColourPalette();

            if (showShork) printf("%s%s%s", colAccent, SHORK[shorkLine++], colReset);
            printf("%s\n", palette.normalCols);
            if (showShork) printf("%s%s%s", colAccent, SHORK[shorkLine++], colReset);
            printf("%s\n", palette.boldCols);
        }
    }



    if (saveConf)
        writeConf(bullet, COLOUR, COMPACT, fieldsOrig, noIP, showShorkOrig, useBullets);

    free(COLOUR);
    free(colAccent);
    free(fieldsOrig);
    free(fields);
    free(hostname);
    free(os);
    if (de != wm) free(de);
    free(wm);
    free(gpuFromCPU);
    if (cpu)
    {
        free(cpu->uarch);
        free(cpu->vendor);
        free(cpu->name);
        free(cpu);
    }

    return 0;
}
