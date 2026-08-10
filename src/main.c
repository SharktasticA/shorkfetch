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



#include "art.h"
#include "colours.h"
#include "conf.h"
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



#define VERSION     "0.6-wip"
#define OUTPUT_LEN  8192



void showHelp(void)
{
    WORD_WRAPPED *desc = wordWrap("A tool that displays basic system and environment information in a summarised format.\n", TERM_SIZE.ws_col, NULL, 0, 0);
    printf("%s\n", desc->str);
    free(desc->str);
    free(desc);

    WORD_WRAPPED *usage = wordWrap("Usage: shorkfetch [OPTIONS]\n\n", TERM_SIZE.ws_col, NULL, 0, 0);
    printf("%s", usage->str);
    free(usage->str);
    free(usage);

    WORD_WRAPPED *options = wordWrap("Options:\n", TERM_SIZE.ws_col, NULL, 0, 0);
    printf("%s", options->str);
    free(options->str);
    free(options);

    WORD_WRAPPED *bullet = wordWrap("-b, --bullet      Specifies a custom character to use with bullet-point mode; no assignment returns the current character\n", TERM_SIZE.ws_col, "                  ", 0, 0);
    printf("%s", bullet->str);
    free(bullet->str);
    free(bullet);

    WORD_WRAPPED *colour = wordWrap("-cl, --colour     Specifies a custom accent colour; no assignment returns the current colour\n", TERM_SIZE.ws_col, "                  ", 0, 0);
    printf("%s", colour->str);
    free(colour->str);
    free(colour);

    WORD_WRAPPED *compact = wordWrap("-co, --compact    Compacts field names and field values\n", TERM_SIZE.ws_col, "                  ", 0, 0);
    printf("%s", compact->str);
    free(compact->str);
    free(compact);

    WORD_WRAPPED *help = wordWrap("-h, --help        Displays help information and exits\n", TERM_SIZE.ws_col, "                  ", 0, 0);
    printf("%s", help->str);
    free(help->str);
    free(help);

    WORD_WRAPPED *fields = wordWrap("-f, --fields      Specifies a custom fields list and order; no assignment returns list of current fields\n", TERM_SIZE.ws_col, "                  ", 0, 0);
    printf("%s", fields->str);
    free(fields->str);
    free(fields);

    WORD_WRAPPED *mode = wordWrap("-m, --mode        Select what view mode to use: [n]ormal, [b]ullets\n", TERM_SIZE.ws_col, "                  ", 0, 0);
    printf("%s", mode->str);
    free(mode->str);
    free(mode);

    WORD_WRAPPED *noArt = wordWrap("-na, --no-art     Disables the SHORK ASCII art\n", TERM_SIZE.ws_col, "                  ", 0, 0);
    printf("%s", noArt->str);
    free(noArt->str);
    free(noArt);

    WORD_WRAPPED *reset = wordWrap("-r, --reset       Resets to default, deletes configuration file and exits\n", TERM_SIZE.ws_col, "                  ", 0, 0);
    printf("%s", reset->str);
    free(reset->str);
    free(reset);

    WORD_WRAPPED *save = wordWrap("-s, --save        Saves chosen options to a configuration file\n", TERM_SIZE.ws_col, "                  ", 0, 0);
    printf("%s", save->str);
    free(save->str);
    free(save);

    WORD_WRAPPED *version = wordWrap("-v, --version     Displays version number and exits\n\n", TERM_SIZE.ws_col, "                  ", 0, 0);
    printf("%s", version->str);
    free(version->str);
    free(version);

    WORD_WRAPPED *colours = wordWrap("Colours: black, blue, bold_blue, bold_cyan, bold_green, bold_magenta, bold_red, bold_white, bold_yellow, cyan, green, grey, magenta, red, white, yellow, off\n\n", TERM_SIZE.ws_col, NULL, 0, 0);
    printf("%s", colours->str); 
    free(colours->str);
    free(colours);

    WORD_WRAPPED *fieldNames = wordWrap("Fields: os, krn, upt, pkgs, scn, de, wm, trm, sh, cpu, gpu, ram, swap, root, lip, clrs, --- (separator), single blank space (new line)\n\n", TERM_SIZE.ws_col, NULL, 0, 0);
    printf("%s", fieldNames->str);
    free(fieldNames->str);
    free(fieldNames);

    WORD_WRAPPED *notes = wordWrap("Note: by default, the SHORK ASCII art is disabled if the terminal's width is less than 62 columns or if less than 7 fields are present.\n", TERM_SIZE.ws_col, NULL, 0, 0);
    printf("%s", notes->str);
    free(notes->str);
    free(notes);
}



int main(int argc, char *argv[])
{
    COLOUR = strdup("bold_cyan");
    HOME =  getenv("HOME");
    TERM_SIZE = getTerminalSize();

    char bullet = '*';
#ifndef NO_STR_CLEANING
    char *fields = strdup("os,krn,upt,pkgs,scn,de,wm,trm,sh,cpu,gpu,ram,swap,root,lip, ,clrs, ");
#else
    char *fields = strdup("os,krn,upt,trm,sh,---,cpu,gpu,ram,swap,root, ");
#endif
    int noIP = 0;
    int saveConf = 0;
    int shorkLine = 0;
    int showShork = 1;
    VIEW_MODE mode = NORMAL;

    readConf(&bullet, &COLOUR, &COMPACT, &fields, &mode, &noIP, &showShork);

    for (int i = 1; i < argc; i++)
    {
        if ((strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "--help") == 0))
        {
            showHelp();
            free(COLOUR);
            free(fields);
            return 0;
        }
        else if (strncmp(argv[i], "-b", 2) == 0 || strncmp(argv[i], "--bullet", 8) == 0)
        {
            char *bulletChar = NULL;
            if (strncmp(argv[i], "-b=", 3) == 0)
                bulletChar = &argv[i][3];
            else if (strncmp(argv[i], "--bullet=", 9) == 0)
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
            else
            {
                printf("\"%c\"\n", bullet);
                free(COLOUR);
                free(fields);
                return 0;
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
                return 0;
            }

            equalsNeedle++;
            free(fields);
            fields = strdup(equalsNeedle);

            // Remove trailing comma if present
            size_t len = strlen(fields);
            if (len > 0 && fields[len - 1] == ',')
                fields[len - 1] = '\0';
        }
        else if (strncmp(argv[i], "-m", 2) == 0 || strncmp(argv[i], "--mode", 6) == 0)
        {
            char *modeVal = NULL;
            if (strncmp(argv[i], "-m=", 3) == 0)
                modeVal = &argv[i][3];
            else if (strncmp(argv[i], "--mode=", 7) == 0)
                modeVal = &argv[i][7];

            if (modeVal)
            {
                if (modeVal[0] == '\0')
                {
                    printf("ERROR: no mode given\n");
                    free(fields);
                    free(COLOUR);
                    return 1;
                }
                else if (strcmp(modeVal, "n") == 0 || strcmp(modeVal, "normal") == 0)
                    mode = NORMAL;
                else if (strcmp(modeVal, "b") == 0 || strcmp(modeVal, "bullet") == 0 || strcmp(modeVal, "bullets") == 0)
                    mode = BULLETS;
                else
                {
                    printf("ERROR: unrecognised mode \"%s\"\n", modeVal);
                    free(fields);
                    free(COLOUR);
                    return 1;
                }
            }
            else
            {
                if (mode == NORMAL)
                    printf("\"normal\"\n");
                else if (mode == BULLETS)
                    printf("\"bullets\"\n");
                free(COLOUR);
                free(fields);
                return 0;
            }
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



    MemInfo mi = getMemInfo();

    struct utsname u;
    int uStatus = uname(&u);

    char *envWay = getenv("WAYLAND_DISPLAY");
    WAYLAND_PRESENT = (envWay != NULL && envWay[0] != '\0');
    char *envX11 = getenv("DISPLAY");
    X11_PRESENT = (envX11 != NULL && envX11[0] != '\0');
    if (WAYLAND_PRESENT || X11_PRESENT)
        XDG_CURRENT_DESKTOP = getenv("XDG_CURRENT_DESKTOP");



    // Print SHORK (if needed)
    if (showShork)
    {
        printf("%s", colAccent);
        if (COMPACT)
        {
            for (int i = 0; i < SHORK_COMP_HEIGHT; i++)
                printf("%s\n", SHORK_COMP[i]);
        }
        else
        {
            for (int i = 0; i < SHORK_NORM_HEIGHT; i++)
                printf("%s\n", SHORK_NORM[i]);
        }
        printf("%s", colReset);
    }

    // Output buffer
    char output[OUTPUT_LEN];
    size_t outputPos = 0;

    // Print header
    char *username = getUsername();
    char *hostname = getHostname(u, uStatus);
    size_t headerWidth = 12;
    if (username[0] != '\0' && hostname[0] != '\0')
    {
        outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, "%s%s%s@%s%s%s\n", colAccent, username, colReset, colAccent, hostname, colReset);
        headerWidth = strlen(username) + 1 + strlen(hostname);
        for (size_t i = 0; i < headerWidth; i++)
            outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, "-");
        outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, "\n");
    }

    // Some things are dependent on others, so we have to look them up regardless
    char *os = getOS(u, uStatus);
    char *de = getDE();
    char *wm = getWM(&de);
    char *gpuFromCPU = NULL;
    CPU_DATA *cpu = getCPU("/proc/cpuinfo", &gpuFromCPU);

    // Assemble output
    for (int i = 0; i < noFields; i++)
    {
        if (strcmp(fieldsProcessed[i], " ") == 0)
            outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, "\n");
        else if (strcmp(fieldsProcessed[i], "---") == 0)
        {
            for (size_t i = 0; i < headerWidth; i++)
                outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, "-");
            outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, "\n");
        }
        else if (strcmp(fieldsProcessed[i], "os") == 0)
        {
            if (os && os[0] != '\0')
            {
                if (mode == NORMAL)
                {
                    if (!COMPACT)
                        outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, "%sOS:%s       %s\n", colAccent, colReset, os);
                    else
                        outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, "%sOS:%s  %s\n", colAccent, colReset, os);
                }
                else
                {
                    char icon[10] = {bullet};
                    outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, " %s%s%s %s\n", colAccent, icon, colReset, os);
                }
            }
        }
        else if (strcmp(fieldsProcessed[i], "krn") == 0)
        {
            char *kernel = getKernel(u, uStatus);
            if (kernel && kernel[0] != '\0')
            {
                if (mode == NORMAL)
                {
                    if (!COMPACT)
                        outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, "%sKernel:%s   %s\n", colAccent, colReset, kernel);
                    else
                        outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, "%sKrn:%s %s\n", colAccent, colReset, kernel);
                }
                else
                {
                    char icon[10] = {bullet};
                    outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, " %s%s%s %s\n", colAccent, icon, colReset, kernel);
                }
            }
            free(kernel);
        }
        else if (strcmp(fieldsProcessed[i], "upt") == 0)
        {
            char *uptime = getUptime();
            if (uptime && uptime[0] != '\0')
            {
                if (mode == NORMAL)
                {
                    if (!COMPACT)
                        outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, "%sUptime:%s   %s\n", colAccent, colReset, uptime);
                    else
                        outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, "%sUp:%s  %s\n", colAccent, colReset, uptime);
                }
                else
                {
                    char icon[10] = {bullet};
                    outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, " %s%s%s %s\n", colAccent, icon, colReset, uptime);
                }
            }
            free(uptime);
        }
        else if (strcmp(fieldsProcessed[i], "pkgs") == 0)
        {
            char *pkgs = getPackages(os);
            if (pkgs && pkgs[0] != '\0')
            {
                if (mode == NORMAL)
                {
                    if (!COMPACT)
                        outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, "%sPackages:%s %s\n", colAccent, colReset, pkgs);
                    else
                        outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, "%sPkg:%s %s\n", colAccent, colReset, pkgs);
                }
                else
                {
                    char icon[10] = {bullet};
                    outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, " %s%s%s %s\n", colAccent, icon, colReset, pkgs);
                }
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
                        if (mode == NORMAL)
                        {
                            if (!COMPACT)
                            {
                                // No compact - no bullet - single screen
                                if (noScreens == 1)
                                    outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, "%sScreen:%s   %s\n", colAccent, colReset, screen);
                                // No compact - no bullet - multiple screens - first screen
                                else if (!pastFirstScreen)
                                    outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, "%sScreens:%s  %s\n", colAccent, colReset, screen);
                                // No compact - no bullet - multiple screens - subsequent screens
                                else 
                                    outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, "          %s\n", screen);
                            }
                            else
                            {
                                // Compact - no bullet - single screen
                                if (noScreens == 1)
                                    outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, "%sScn:%s %s\n", colAccent, colReset, screen);
                                // Compact - no bullet - multiple screens - first screen
                                else if (!pastFirstScreen)
                                    outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, "%sScn:%s %s\n", colAccent, colReset, screen);
                                // Compact - no bullet - multiple screens - subsequent screens
                                else 
                                    outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, "     %s\n", screen);
                            }
                        }
                        else
                        {
                            char icon[10] = {bullet};
                            outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, " %s%s%s %s\n", colAccent, icon, colReset, screen);
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
                if (mode == NORMAL)
                {
                    if (!COMPACT)
                        outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, "%sDE:%s       %s\n", colAccent, colReset, de);
                    else
                        outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, "%sDE:%s  %s\n", colAccent, colReset, de);
                }
                else
                {
                    char icon[10] = {bullet};
                    outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, " %s%s%s %s\n", colAccent, icon, colReset, de);
                }
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
                if (mode == NORMAL)
                {
                    if (!COMPACT)
                        outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, "%sWM:%s       %s%s\n", colAccent, colReset, wm, server);
                    else
                        outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, "%sWM:%s  %s\n", colAccent, colReset, wm);
                }
                else 
                {
                    char icon[10] = {bullet};
                    if (!COMPACT)
                        outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, " %s%s%s %s%s\n", colAccent, icon, colReset, wm, server);
                    else
                        outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, " %s%s%s %s\n", colAccent, icon, colReset, wm);
                }
            }
        }
        else if (strcmp(fieldsProcessed[i], "trm") == 0)
        {
            char *trm = getTerminal();
            if (trm && trm[0] != '\0')
            {
                if (mode == NORMAL)
                {
                    if (!COMPACT)
                        outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, "%sTerminal:%s %s (%dx%d)\n", colAccent, colReset, trm, TERM_SIZE.ws_col, TERM_SIZE.ws_row);
                    else
                        outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, "%sTrm:%s %s\n", colAccent, colReset, trm);
                }
                else
                {
                    char icon[10] = {bullet};
                    if (!COMPACT)
                        outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, " %s%s%s %s (%dx%d)\n", colAccent, icon, colReset, trm, TERM_SIZE.ws_col, TERM_SIZE.ws_row);
                    else
                        outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, " %s%s%s %s\n", colAccent, icon, colReset, trm);
                }
            }
            // If we don't have a terminal name, we can at least still show the
            // console size
            else
            {
                if (mode == NORMAL)
                {
                    if (!COMPACT)
                        outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, "%sConsole:%s  %dx%d\n", colAccent, colReset, TERM_SIZE.ws_col, TERM_SIZE.ws_row);
                    else
                        outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, "%sCon:%s %dx%d\n", colAccent, colReset, TERM_SIZE.ws_col, TERM_SIZE.ws_row);
                }
                else
                {
                    char icon[10] = {bullet};
                    if (!COMPACT)
                        outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, " %s%s%s %dx%d console\n", colAccent, icon, colReset, TERM_SIZE.ws_col, TERM_SIZE.ws_row);
                    else
                        outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, " %s%s%s %dx%dch\n", colAccent, icon, colReset, TERM_SIZE.ws_col, TERM_SIZE.ws_row);
                }
            }
            free(trm);
        }
        else if (strcmp(fieldsProcessed[i], "sh") == 0)
        {
            char *shell = getShell();
            if (shell && shell[0] != '\0')
            {
                if (mode == NORMAL)
                {
                    if (!COMPACT)
                        outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, "%sShell:%s    %s\n", colAccent, colReset, shell);
                    else
                        outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, "%sSh:%s  %s\n", colAccent, colReset, shell);
                }
                else
                {
                    char icon[10] = {bullet};
                    outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, " %s%s%s %s\n", colAccent, icon, colReset, shell);
                }
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
                    if (mode == NORMAL)
                    {
                        if (!COMPACT)
                            outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, "%sCPU:%s      %s\n", colAccent, colReset, cpuStr);
                        else
                            outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, "%sCPU:%s %s\n", colAccent, colReset, cpuStr);
                    }
                    else
                    {
                        char icon[10] = {bullet};
                        outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, " %s%s%s %s\n", colAccent, icon, colReset, cpuStr);
                    }
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
                        if (mode == NORMAL)
                        {
                            if (!COMPACT)
                            {
                                if (noGPUs == 1)
                                    outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, "%sGPU:%s      %s\n", colAccent, colReset, gpuStr);
                                else if (!pastFirstGPU)
                                    outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, "%sGPUs:%s     %s\n", colAccent, colReset, gpuStr);
                                else
                                    outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, "          %s\n", gpuStr);
                            }
                            else
                            {
                                if (noGPUs == 1 || !pastFirstGPU)
                                    outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, "%sGPU:%s %s\n", colAccent, colReset, gpuStr);
                                else
                                    outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, "     %s\n", gpuStr);
                            }
                        }
                        else
                        {
                            char icon[10] = {bullet};
                            outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, " %s%s%s %s\n", colAccent, icon, colReset, gpuStr);
                        }
                    }

                    free(gpuStr);
                    pastFirstGPU = 1;
                }
            }
            // If we found no GPUs the "traditional" way, at least check if we received
            // a fallback found during CPU name processing
            else if (gpuFromCPU && gpuFromCPU[0] != '\0')
            {
                if (mode == NORMAL)
                {
                    if (!COMPACT)
                        outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, "%sGPU:%s      %s\n", colAccent, colReset, gpuFromCPU);
                    else
                        outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, "%sGPU:%s %s\n", colAccent, colReset, gpuFromCPU);
                }
                else
                {
                    char icon[10] = {bullet};
                    outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, " %s%s%s %s\n", colAccent, icon, colReset, gpuFromCPU);
                }
            }
            free(gpus);
        }
        else if (strcmp(fieldsProcessed[i], "ram") == 0 && mi.memTotal > 0)
        {
            char *ram = getRAM(mi);
            if (ram && ram[0] != '\0')
            {
                if (mode == NORMAL)
                {
                    if (!COMPACT)
                        outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, "%sRAM:%s      %s\n", colAccent, colReset, ram);
                    else
                        outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, "%sRAM:%s %s\n", colAccent, colReset, ram);
                }
                else 
                {
                    char icon[10] = {bullet};
                    if (!COMPACT)
                        outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, " %s%s%s %s RAM\n", colAccent, icon, colReset, ram);
                    else
                        outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, " %s%s%s %s (R)\n", colAccent, icon, colReset, ram);
                }
            }
            free(ram);
        }
        else if (strcmp(fieldsProcessed[i], "swap") == 0 && mi.swapTotal > 0)
        {
            char *swap = getSwap(mi);
            if (swap && swap[0] != '\0')
            {
                if (mode == NORMAL)
                {
                    if (!COMPACT)
                        outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, "%sSwap:%s     %s\n", colAccent, colReset, swap);
                    else
                        outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, "%sSwp:%s %s\n", colAccent, colReset, swap);
                }
                else 
                {
                    char icon[10] = {bullet};
                    if (!COMPACT)
                        outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, " %s%s%s %s swap\n", colAccent, icon, colReset, swap);
                    else
                        outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, " %s%s%s %s (S)\n", colAccent, icon, colReset, swap);
                }
            }
            free(swap);
        }
        else if (strcmp(fieldsProcessed[i], "root") == 0)
        {
            char *root = getRoot();
            if (root && root[0] != '\0')
            {
                if (mode == NORMAL)
                {
                    if (!COMPACT)
                        outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, "%sRoot:%s     %s\n", colAccent, colReset, root);
                    else
                        outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, "%s/:%s   %s\n", colAccent, colReset, root);
                }
                else 
                {
                    char icon[10] = {bullet};
                    if (!COMPACT)
                        outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, " %s%s%s %s root\n", colAccent, icon, colReset, root);
                    else
                        outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, " %s%s%s %s (/)\n", colAccent, icon, colReset, root);
                }
            }
            free(root);
        }
        else if (strcmp(fieldsProcessed[i], "lip") == 0 && !noIP)
        {
            char *localIP = getLocalIP();
            if (localIP)
            {
                if (mode == NORMAL)
                {
                    if (!COMPACT)
                        outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, "%sLocal IP:%s %s\n", colAccent, colReset, localIP);
                    else
                        outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, "%sLoc:%s %s\n", colAccent, colReset, localIP);
                }
                else 
                {
                    char icon[10] = {bullet};
                    if (!COMPACT)
                        outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, " %s%s%s %s local\n", colAccent, icon, colReset, localIP);
                    else
                        outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, " %s%s%s %s (L)\n", colAccent, icon, colReset, localIP);
                }
                free(localIP);
            }
        }
        else if (strcmp(fieldsProcessed[i], "clrs") == 0)
        {
            ColourPalette palette = getColourPalette(showShork);
            outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, "%s\n", palette.normalCols);
            outputPos += snprintf(output + outputPos, OUTPUT_LEN - outputPos, "%s\n", palette.boldCols);
        }
    }

    // Print output
    int shorkWidth = SHORK_NORM_WIDTH;
    int shorkHeight = SHORK_NORM_HEIGHT;
    if (!showShork)
        shorkWidth = 0;
    else if (COMPACT)
    {
        shorkWidth = SHORK_COMP_WIDTH;
        shorkHeight = SHORK_COMP_HEIGHT;
    }

    WORD_WRAPPED *data = NULL;
    if (mode == BULLETS)
        data = wordWrap(output, TERM_SIZE.ws_col - shorkWidth, "   ", 1, 0);
    else
    {
        if (COMPACT)
            data = wordWrap(output, TERM_SIZE.ws_col - shorkWidth, "     ", 1, 0);
        else
            data = wordWrap(output, TERM_SIZE.ws_col - shorkWidth, "          ", 1, 0);
    }

    if (data)
    {
        if (showShork)
        {
            // Move cursor across art to where data should start
            printf("\033[%dA", shorkHeight);
            printf("\033[%dC", shorkWidth);
        }

        for (size_t i = 0; i < data->len; i++)
        {
            putchar(data->str[i]);
            if (showShork)
            {
                if (data->str[i] == '\n')
                    printf("\033[%dC", shorkWidth);
            }
        }

        if (showShork && data->lines < shorkHeight)
            printf("\033[%dB", shorkHeight - data->lines);

        free(data->str);
        free(data);

        if (showShork)
            printf("\r");
    }
    else
        printf("ERROR: could not process output string\n");



    if (saveConf)
        writeConf(bullet, COLOUR, COMPACT, fieldsOrig, mode, noIP, showShork);

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
#ifndef X86_ONLY
        free(cpu->processor);
        free(cpu->uarch);
#endif
        free(cpu->vendor);
        free(cpu->name);
        free(cpu);
    }

    return 0;
}
