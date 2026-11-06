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



#include "../shorkcommon/colours.h"
#include "../shorkcommon/general.h"
#include "../shorkcommon/shorkmenu.h"

#include "art.h"
#include "conf.h"
#include "cpu.h"
#include "de-wm.h"
#include "disk.h"
#include "globals.h"
#include "gpu.h"
#include "hostname.h"
#include "ip.h"
#include "kernel.h"
#include "locale.h"
#include "memory.h"
#include "os.h"
#include "packages.h"
#include "palette.h"
#include "screen.h"
#include "shell.h"
#include "terminal.h"
#include "testing.h"
#include "uptime.h"
#include "username.h"

#include <stdarg.h>



#ifndef SHORK_DISKETTE

#include <getopt.h>

enum { OPT_NO_ART = 256, OPT_NO_ESC, OPT_NO_IHA, OPT_NO_IP };

static const char *shortOpts = "ha::B::b::l::m::H::cCf::o::u::rsv";

static const struct option longOpts[] = {
    { "help",       no_argument,        0,  'h' },
    { "accent-col", optional_argument,  0,  'a' },
    { "bullet-col", optional_argument,  0,  'B' },
    { "bullet",     optional_argument,  0,  'b' },
    { "low-col",    optional_argument,  0,  'l' },
    { "med-col",    optional_argument,  0,  'm' },
    { "high-col",   optional_argument,  0,  'H' },
    { "compact",    no_argument,        0,  'c' },
    { "conf",       no_argument,        0,  'C' },
    { "fields",     optional_argument,  0,  'f' },
    { "mode",       optional_argument,  0,  'o' },
    { "max-unit",   optional_argument,  0,  'u' },
    { "no-art",     no_argument,        0,  OPT_NO_ART },
    { "no-esc",     no_argument,        0,  OPT_NO_ESC },
    { "no-iha",     no_argument,        0,  OPT_NO_IHA },
    { "no-ip",      no_argument,        0,  OPT_NO_IP },
    { "reset",      no_argument,        0,  'r' },
    { "save",       no_argument,        0,  's' },
    { "version",    no_argument,        0,  'v' },
    { 0, 0, 0, 0}
};

#endif



/**
 * Prints a single line of the SHORK ASCII art. It is intended to be used
 * during the "assemble output" loop when in no-escape-codes mode.
 * @param stopOnceDone Flags if this function should stop once the SHORK is
 *                     complete
 * @return 1 if complete or inapplicable; 0 in progress
 */
int printShorkLine(int stopOnceDone)
{
    if (!CONFIG.showArt)
        return 1;

    if (COMPACT)
    {
        if (SHORK_LINE < SHORK_COMP_HEIGHT - 1)
            printf("%s", SHORK_COMP[SHORK_LINE++]);
        else if (!stopOnceDone)
            printf("%*s", SHORK_COMP_WIDTH, "");
        else
            return 1;
    }
    else
    {
        if (SHORK_LINE < SHORK_NORM_HEIGHT - 1)
            printf("%s", SHORK_NORM[SHORK_LINE++]);
        else if (!stopOnceDone)
            printf("%*s", SHORK_NORM_WIDTH, "");
        else
            return 1;
    }

    return 0;
}

#ifndef SHORK_DISKETTE

void showHelp(void)
{
    WORD_WRAPPED *desc = wordWrap("A cute, shark-themed Linux fetch tool "
        "that displays system, hardware, and environment information in a "
        "summarised format.\n", TERM_SIZE.ws_col, NULL, NULL, 0, 0);
    printf("%s\n", desc->str);
    free(desc->str);
    free(desc);

    WORD_WRAPPED *usage = wordWrap("Usage: shorkfetch [OPTIONS]\n\n",
        TERM_SIZE.ws_col, NULL, NULL, 0, 0);
    printf("%s", usage->str);
    free(usage->str);
    free(usage);

    WORD_WRAPPED *options = wordWrap("Options:\n", TERM_SIZE.ws_col, NULL,
        NULL, 0, 0);
    printf("%s", options->str);
    free(options->str);
    free(options);

    WORD_WRAPPED *accentCol = wordWrap("-a, --accent-col  Specifies a "
        "custom accent colour; no assignment returns the current colour "
        "and exits\n", TERM_SIZE.ws_col, "                  ", NULL, 0, 0);
    printf("%s", accentCol->str);
    free(accentCol->str);
    free(accentCol);

    WORD_WRAPPED *bulletCol = wordWrap("-B, --bullet-col  Specifies a "
        "custom bullet-point colour; no assignment returns the current "
        "colour and exits\n", TERM_SIZE.ws_col, "                  ", NULL,
        0, 0);
    printf("%s", bulletCol->str);
    free(bulletCol->str);
    free(bulletCol);

    WORD_WRAPPED *bullet = wordWrap("-b, --bullet      Specifies a custom "
        "single-byte character to use with bulleted list mode and "
        "sublists; empty assignment allows lists with no bullet; no "
        "assignment returns the current character and exits\n",
        TERM_SIZE.ws_col, "                  ", NULL, 0, 0);
    printf("%s", bullet->str);
    free(bullet->str);
    free(bullet);

    WORD_WRAPPED *compact = wordWrap("-c, --compact     Compacts field "
        "names and field values\n", TERM_SIZE.ws_col, "                  ",
        NULL, 0, 0);
    printf("%s", compact->str);
    free(compact->str);
    free(compact);

    WORD_WRAPPED *conf = wordWrap("-C, --conf        Launches SHORKFETCH "
        "Configurator and exits\n", TERM_SIZE.ws_col, "                  ",
        NULL, 0, 0);
    printf("%s", conf->str);
    free(conf->str);
    free(conf);

    WORD_WRAPPED *fields = wordWrap("-f, --fields      Specifies a custom "
        "fields list and order; no assignment returns list of current "
        "fields and exits\n", TERM_SIZE.ws_col, "                  ", NULL,
        0, 0);
    printf("%s", fields->str);
    free(fields->str);
    free(fields);

    WORD_WRAPPED *help = wordWrap("-h, --help        Displays help "
        "information and exits\n", TERM_SIZE.ws_col, "                  ",
        NULL, 0, 0);
    printf("%s", help->str);
    free(help->str);
    free(help);

    WORD_WRAPPED *highCol = wordWrap("-H, --high-col    Specifies a "
        "custom high percentage colour; no assignment returns the current "
        "colour and exits\n", TERM_SIZE.ws_col, "                  ", NULL,
        0, 0);
    printf("%s", highCol->str);
    free(highCol->str);
    free(highCol);

    WORD_WRAPPED *lowCol = wordWrap("-l, --low-col     Specifies a custom "
        "low percentage colour; no assignment returns the current colour "
        "and exits\n", TERM_SIZE.ws_col, "                  ", NULL, 0, 0);
    printf("%s", lowCol->str);
    free(lowCol->str);
    free(lowCol);

    WORD_WRAPPED *medCol = wordWrap("-m, --med-col     Specifies a custom "
        "medium percentage colour; no assignment returns the current "
        "colour and exits\n", TERM_SIZE.ws_col, "                  ", NULL,
        0, 0);
    printf("%s", medCol->str);
    free(medCol->str);
    free(medCol);

    WORD_WRAPPED *mode = wordWrap("-o, --mode        Specifies which view "
        "mode to use: [n]ormal (default), [b]ullet list; no assignment "
        "returns the current mode and exits\n",
        TERM_SIZE.ws_col, "                  ", NULL, 0, 0);
    printf("%s", mode->str);
    free(mode->str);
    free(mode);

    WORD_WRAPPED *reset = wordWrap("-r, --reset       Resets to default, "
        "deletes configuration file and exits\n", TERM_SIZE.ws_col,
        "                  ", NULL, 0, 0);
    printf("%s", reset->str);
    free(reset->str);
    free(reset);

    WORD_WRAPPED *save = wordWrap("-s, --save        Saves chosen options "
        "to a configuration file\n", TERM_SIZE.ws_col,
        "                  ", NULL, 0, 0);
    printf("%s", save->str);
    free(save->str);
    free(save);

    WORD_WRAPPED *maxUnit = wordWrap("-u, --max-unit    Specifies the "
        "largest data unit that can be displayed: [b]ytes, [k]ibi, [m]ebi, "
        "[g]ibi, [t]ebi, [p]ebi (default); no assignment returns the "
        "current unit and exits\n", TERM_SIZE.ws_col,
        "                  ", NULL, 0, 0);
    printf("%s", maxUnit->str);
    free(maxUnit->str);
    free(maxUnit);

    WORD_WRAPPED *noArt = wordWrap("    --no-art      Disables the SHORK "
        "ASCII art\n", TERM_SIZE.ws_col, "                  ", NULL, 0, 0);
    printf("%s", noArt->str);
    free(noArt->str);
    free(noArt);

    WORD_WRAPPED *noEsc = wordWrap("    --no-esc      Disables all ANSI "
        "escape codes and colour features\n", TERM_SIZE.ws_col,
        "                  ", NULL, 0, 0);
    printf("%s", noEsc->str);
    free(noEsc->str);
    free(noEsc);

    WORD_WRAPPED *noIha = wordWrap("    --no-iha      Disables Intel "
        "Hybrid Architecture-specific core/thread counting\n",
        TERM_SIZE.ws_col, "                  ", NULL, 0, 0);
    printf("%s", noIha->str);
    free(noIha->str);
    free(noIha);

    WORD_WRAPPED *noIp = wordWrap("    --no-ip       Disables any IP "
        "address display\n", TERM_SIZE.ws_col, "                  ", NULL,
        0, 0);
    printf("%s", noIp->str);
    free(noIp->str);
    free(noIp);

    WORD_WRAPPED *version = wordWrap("-v, --version     Displays version "
        "number and exits\n\n", TERM_SIZE.ws_col, "                  ",
        NULL, 0, 0);
    printf("%s", version->str);
    free(version->str);
    free(version);

    WORD_WRAPPED *colours = wordWrap("Colours: black, blue, bright_blue, "
        "bright_cyan, bright_green, bright_magenta, bright_red, "
        "bright_white, bright_yellow, cyan, green, grey, magenta, red, "
        "white, yellow, off\n\n", TERM_SIZE.ws_col, NULL, NULL, 0, 0);
    printf("%s", colours->str); 
    free(colours->str);
    free(colours);

    WORD_WRAPPED *fieldNames = wordWrap("Fields: log (user@hostname), os, "
        "krn, upt, pkgs, loc, scn, de, wm, trm, sh, cpu, gpu, ram, swap, "
        "dsk, root, lip, xip, clrs, clba, clbr, --- (separator), single"
        "blank space (blank line)\n", TERM_SIZE.ws_col, NULL, NULL, 0, 0);
    printf("%s", fieldNames->str);
    free(fieldNames->str);
    free(fieldNames);
}

#endif

/**
 * A signature-compatible replacement for snprintf that outputs to stdout
 * instead. It used with the writeOutput func* in main() to allow selecting
 * between this and real snprintf depending on no-escape-codes' status.
 */
int snprintfStdout(char *__restrict __s, size_t __maxlen,
    const char *__restrict __format, ...)
{
    // Discard buffer and size since stdout doesn't need either
    (void)__s;
    (void)__maxlen;

    // Mirror how snprintf would gather its trailing arguments
    va_list ap;
    va_start(ap, __format);

    // Pass what's needed to vprintf and forward its return val
    int r = vprintf(__format, ap);
    va_end(ap);
    return r;
}



int main(int argc, char *argv[])
{
    HOME = getenv("HOME");
    TERM_SIZE = getTerminalSize();

    int saveConf = 0;

    readConf(&CONFIG);

#ifndef SHORK_DISKETTE
    int opt;
    while ((opt = getopt_long(argc, argv, shortOpts, longOpts, NULL)) != -1)
    {
        switch (opt)
        {
        case 'h':
            showHelp();
            return 0;

        case 'a':
            if (!optarg)
            {
                printf("%s\n", CONFIG.colAccent);
                return 0;
            }
            snprintf(CONFIG.colAccent, CONF_COL_LEN, "%s", optarg);
            break;

        case 'B':
            if (!optarg)
            {
                printf("%s\n", CONFIG.colBullet);
                return 0;
            }
            snprintf(CONFIG.colBullet, CONF_COL_LEN, "%s", optarg);
            break;

        case 'l':
            if (!optarg)
            {
                printf("%s\n", CONFIG.colPctLow);
                return 0;
            }
            snprintf(CONFIG.colPctLow, CONF_COL_LEN, "%s", optarg);
            break;

        case 'm':
            if (!optarg)
            {
                printf("%s\n", CONFIG.colPctMed);
                return 0;
            }
            snprintf(CONFIG.colPctMed, CONF_COL_LEN, "%s", optarg);
            break;

        case 'H':
            if (!optarg)
            {
                printf("%s\n", CONFIG.colPctHigh);
                return 0;
            }
            snprintf(CONFIG.colPctHigh, CONF_COL_LEN, "%s", optarg);
            break;

        case 'b':
            if (!optarg)
            {
                printf("\"%c\"\n", CONFIG.charBullet);
                return 0;
            }
            if (strlen(optarg) > 1)
            {
                printf("ERROR: custom bullet point character can only "
                    "be a single character\n");
                return 1;
            }
            CONFIG.charBullet = optarg[0];
            break;

        case 'c':
            COMPACT = 1;
            break;

        case 'C':
            return launchConf();

        case 'f':
        {
            if (!optarg)
            {
                printf("\"%s\"\n", CONFIG.fields);
                return 0;
            }
            snprintf(CONFIG.fields, CONF_FIELDS_LEN, "%s", optarg);

            // Remove trailing comma if present
            int len = strlen(CONFIG.fields);
            if (len > 0 && CONFIG.fields[len - 1] == ',')
                CONFIG.fields[len - 1] = '\0';
            break;
        }

        case 'o':
            if (!optarg)
            {
                if (CONFIG.mode == NORMAL)
                    printf("\"normal\"\n");
                else if (CONFIG.mode == BULLETS)
                    printf("\"bullets\"\n");
                return 0;
            }
            if (optarg[0] == '\0')
            {
                printf("ERROR: no mode given\n");
                return 1;
            }
            else if (strcmp(optarg, "n") == 0 ||
                strcmp(optarg, "N") == 0 ||
                strcmp(optarg, "normal") == 0)
                CONFIG.mode = NORMAL;
            else if (strcmp(optarg, "b") == 0 ||
                strcmp(optarg, "B") == 0 ||
                strcmp(optarg, "bullet") == 0 ||
                strcmp(optarg, "bullets") == 0)
                CONFIG.mode = BULLETS;
            else
            {
                printf("ERROR: unrecognised mode \"%s\"\n", optarg);
                return 1;
            }
            break;

        case 'u':
            if (!optarg)
            {
                printf("'%c'\n", CONFIG.maxUnit);
                return 0;
            }
            if (optarg[0] == '\0')
            {
                printf("ERROR: no unit given\n");
                return 1;
            }
            else if (strcmp(optarg, "b") == 0 ||
                strcmp(optarg, "B") == 0 ||
                strcmp(optarg, "byte") == 0)
                CONFIG.maxUnit = 'b';
            else if (strcmp(optarg, "k") == 0 ||
                strcmp(optarg, "K") == 0 ||
                strcmp(optarg, "kibi") == 0 ||
                strcmp(optarg, "Kibi") == 0)
                CONFIG.maxUnit = 'k';
            else if (strcmp(optarg, "m") == 0 ||
                strcmp(optarg, "M") == 0 ||
                strcmp(optarg, "mebi") == 0 ||
                strcmp(optarg, "Mebi") == 0)
                CONFIG.maxUnit = 'm';
            else if (strcmp(optarg, "g") == 0 ||
                strcmp(optarg, "G") == 0 ||
                strcmp(optarg, "gibi") == 0 ||
                strcmp(optarg, "Gibi") == 0)
                CONFIG.maxUnit = 'g';
            else if (strcmp(optarg, "t") == 0 ||
                strcmp(optarg, "T") == 0 ||
                strcmp(optarg, "tebi") == 0 ||
                strcmp(optarg, "Tebi") == 0)
                CONFIG.maxUnit = 't';
            else if (strcmp(optarg, "p") == 0 ||
                strcmp(optarg, "P") == 0 ||
                strcmp(optarg, "pebi") == 0 ||
                strcmp(optarg, "Pebi") == 0)
                CONFIG.maxUnit = 'p';
            else
            {
                printf("ERROR: unrecognised unit \"%s\"\n", optarg);
                return 1;
            }
            break;

        case OPT_NO_ART:
            CONFIG.showArt = 0;
            break;

        case OPT_NO_ESC:
            CONFIG.showEsc = 0;
            break;

        case OPT_NO_IHA:
            CONFIG.showIHA = 0;
            break;

        case OPT_NO_IP:
            CONFIG.showIP = 0;
            break;

        case 'r':
        {
            int result = deleteConf();
            if (result)
                printf("SHORKFETCH configuration reset\n");
            else
                printf("WARNING: SHORKFETCH configuration already "
                    "default\n");
            return 0;
        }

        case 's':
            saveConf = 1;
            break;

        case 'v':
            printf("SHORKFETCH %s\n", VERSION);
            return 0;

        default:
            return 1;
        }
    }

    if (optind < argc)
    {
        printf("ERROR: unrecognised option \"%s\"\n", argv[optind]);
        return 1;
    }
#else
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-v") == 0 ||
            strcmp(argv[i], "--version") == 0)
        {
            printf("SHORKFETCH %s\n", VERSION);
            return 0;
        }
        else
        {
            printf("ERROR: unrecognised option \"%s\"\n", argv[i]);
            return 1;
        }
    }
#endif

    // Field name accent colour escape sequence
    char *colAccent = NULL;
    // Bullet-point colour escape sequence
    char *colBullet = NULL;
    // Colour escape sequence for percentages >= 0 and < 50
    char *colPctLow = NULL;
    // Colour escape sequence for percentages >= 50 and < 80
    char *colPctMed = NULL;
    // Colour escape sequence for percentages >= 80
    char *colPctHigh = NULL;
    // General colour reset escape sequence
    char *colReset = NULL;
    // Func* for selecting which *printf-style output backend to use
    int (*writeOutput)
        (char *__restrict, size_t, const char *__restrict, ...);

    // Write to stdout and disable & disable colour output
    if (!CONFIG.showEsc)
    {
        writeOutput = snprintfStdout;
        colAccent = strdup("");
        colBullet = strdup("");
        colPctLow = strdup("");
        colPctMed = strdup("");
        colPctHigh = strdup("");
        colReset = "";
    }
    // Write to output buffer & permit colour output
    else
    {
        writeOutput = snprintf;
        colAccent = validateColour(CONFIG.colAccent);
        if (!colAccent)
        {
            printf("ERROR: unrecognised colour \"%s\"\n", CONFIG.colAccent);
            return 1;
        }
        colBullet = validateColour(CONFIG.colBullet);
        if (!colBullet)
        {
            printf("ERROR: unrecognised colour \"%s\"\n", CONFIG.colBullet);
            return 1;
        }
        colPctLow = validateColour(CONFIG.colPctLow);
        if (!colPctLow)
        {
            printf("ERROR: unrecognised colour \"%s\"\n", CONFIG.colPctLow);
            return 1;
        }
        colPctMed = validateColour(CONFIG.colPctMed);
        if (!colPctMed)
        {
            printf("ERROR: unrecognised colour \"%s\"\n", CONFIG.colPctMed);
            return 1;
        }
        colPctHigh = validateColour(CONFIG.colPctHigh);
        if (!colPctHigh)
        {
            printf("ERROR: unrecognised colour \"%s\"\n",
                CONFIG.colPctHigh);
            return 1;
        }
        colReset = (!CONFIG.showEsc) ? "" : "\033[" COL_RESET "m";
    }

#ifdef TESTS
    testInterpretScreen();
    testInterpretGPU();
    testGetCPU();
    return 0;
#endif



    // Validate which field to display
    char fieldsProcessed[MAX_FIELDS][5];
    int noFields = validateFields(CONFIG.fields, fieldsProcessed,
        CONFIG.showEsc);
    if (noFields == 0)
    {
        printf("ERROR: no field names were given to display\n");
        return 1;
    }
    else if (noFields == -1)
        return 1;



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
    if (CONFIG.showArt && CONFIG.showEsc)
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
    int outputPos = 0;

    // Print header
    char *username = getUsername();
    char *hostname = getHostname(u, uStatus);
    int headerWidth = 12;
    if (username[0] != '\0' && hostname[0] != '\0')
        headerWidth = strlen(username) + 1 + strlen(hostname);

    // Some things are dependent on others, so we have to look them up
    // regardless
    char *os = getOS(u, uStatus);
    char *de = getDE();
    char *wm = getWM(&de);
    char *gpuFromCPU = NULL;
    CPU_DATA *cpu = getCPU("/proc/cpuinfo", &gpuFromCPU);

    // Preprepared bullet icon to use if needed.
    char icon[16] = {0};
    if (CONFIG.charBullet != '\0')
        snprintf(icon, 16, " %s%c%s ", colBullet, CONFIG.charBullet,
            colReset);

    // Assemble output
    for (int i = 0; i < noFields; i++)
    {
        if (strcmp(fieldsProcessed[i], " ") == 0)
        {
            if (!CONFIG.showEsc) printShorkLine(0);
            outputPos += writeOutput(output + outputPos,
                OUTPUT_LEN - outputPos, "\n");
        }
        else if (strcmp(fieldsProcessed[i], "---") == 0)
        {
            if (!CONFIG.showEsc) printShorkLine(0);
            for (int i = 0; i < headerWidth; i++)
                outputPos += writeOutput(output + outputPos,
                    OUTPUT_LEN - outputPos, "-");
            outputPos += writeOutput(output + outputPos,
                OUTPUT_LEN - outputPos, "\n");
        }
        else if (strcmp(fieldsProcessed[i], "log") == 0)
        {
            if (username[0] != '\0' && hostname[0] != '\0')
            {
                headerWidth = strlen(username) + 1 + strlen(hostname);
                if (!CONFIG.showEsc) printShorkLine(0);
                if (i == 0)
                {
                    outputPos += writeOutput(output + outputPos,
                        OUTPUT_LEN - outputPos, "%s%s%s@%s%s%s\n",
                        colAccent, username, colReset, colAccent, hostname,
                        colReset);
                }
                else
                {
                    if (CONFIG.mode == NORMAL)
                    {
                        if (!COMPACT)
                            outputPos += writeOutput(output + outputPos,
                                OUTPUT_LEN - outputPos,
                                "%sLogin:%s    %s@%s\n",
                                colAccent, colReset, username, hostname);
                        else
                            outputPos += writeOutput(output + outputPos,
                                OUTPUT_LEN - outputPos, "%sLog:%s %s@%s\n",
                                colAccent, colReset, username, hostname);
                    }
                    else
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, "%s%s@%s\n", icon,
                            username, hostname);
                    }
            }
        }
        else if (strcmp(fieldsProcessed[i], "os") == 0)
        {
            if (os && os[0] != '\0')
            {
                if (!CONFIG.showEsc) printShorkLine(0);
                if (CONFIG.mode == NORMAL)
                {
                    if (!COMPACT)
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, "%sOS:%s       %s\n",
                            colAccent, colReset, os);
                    else
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, "%sOS:%s  %s\n",
                            colAccent, colReset, os);
                }
                else
                {
                    outputPos += writeOutput(output + outputPos,
                        OUTPUT_LEN - outputPos, "%s%s\n", icon, os);
                }
            }
        }
        else if (strcmp(fieldsProcessed[i], "krn") == 0)
        {
            char *kernel = getKernel(u, uStatus);
            if (kernel && kernel[0] != '\0')
            {
                if (!CONFIG.showEsc) printShorkLine(0);
                if (CONFIG.mode == NORMAL)
                {
                    if (!COMPACT)
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, "%sKernel:%s   %s\n",
                            colAccent, colReset, kernel);
                    else
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, "%sKrn:%s %s\n",
                            colAccent, colReset, kernel);
                }
                else
                {
                    outputPos += writeOutput(output + outputPos,
                        OUTPUT_LEN - outputPos, "%s%s\n", icon, kernel);
                }
            }
            free(kernel);
        }
        else if (strcmp(fieldsProcessed[i], "upt") == 0)
        {
            char *uptime = getUptime();
            if (uptime && uptime[0] != '\0')
            {
                if (!CONFIG.showEsc) printShorkLine(0);
                if (CONFIG.mode == NORMAL)
                {
                    if (!COMPACT)
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, "%sUptime:%s   %s\n",
                            colAccent, colReset, uptime);
                    else
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, "%sUp:%s  %s\n",
                            colAccent, colReset, uptime);
                }
                else
                {
                    outputPos += writeOutput(output + outputPos,
                        OUTPUT_LEN - outputPos, "%s%s\n", icon, uptime);
                }
            }
            free(uptime);
        }
        else if (strcmp(fieldsProcessed[i], "pkgs") == 0)
        {
            char *pkgs = getPackages(os);
            if (pkgs && pkgs[0] != '\0')
            {
                if (!CONFIG.showEsc) printShorkLine(0);
                if (CONFIG.mode == NORMAL)
                {
                    if (!COMPACT)
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, "%sPackages:%s %s\n",
                            colAccent, colReset, pkgs);
                    else
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, "%sPkg:%s %s\n",
                            colAccent, colReset, pkgs);
                }
                else
                {
                    outputPos += writeOutput(output + outputPos,
                        OUTPUT_LEN - outputPos, "%s%s\n", icon, pkgs);
                }
            }
            free(pkgs);
        }
        else if (strcmp(fieldsProcessed[i], "loc") == 0)
        {
            LOCALES *locales = getLocales();
            if (locales && locales->locales)
            {
                if (!CONFIG.showEsc) printShorkLine(0);
                if (CONFIG.mode == NORMAL)
                {
                    if (!COMPACT)
                    {
                        if (locales->count == 1)
                            outputPos += writeOutput(output + outputPos,
                                OUTPUT_LEN - outputPos,
                                "%sLocale:%s   %s\n", colAccent, colReset,
                                locales->locales);
                        else
                            outputPos += writeOutput(output + outputPos,
                                OUTPUT_LEN - outputPos,
                                "%sLocales:%s  %s\n", colAccent, colReset,
                                locales->locales);
                    }
                    else
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, "%sLoc:%s %s\n",
                            colAccent, colReset, locales->locales);
                }
                else 
                {
                    outputPos += writeOutput(output + outputPos,
                        OUTPUT_LEN - outputPos, "%s%s\n", icon,
                        locales->locales);
                }
                free(locales->locales);
                free(locales);
            }
        }
        else if (strcmp(fieldsProcessed[i], "scn") == 0)
        {
            int noScreens = 0;
            Screen *screens = getScreens(&noScreens);
            if (screens)
            {
                int pastFirst = 0;
                char icon[100] = {0};
                if (CONFIG.charBullet != '\0')
                    snprintf(icon, sizeof(icon), " %s%c%s ", colBullet,
                        CONFIG.charBullet, colReset);

                for (int j = 0; j < noScreens; j++)
                {
                    char *screen = interpretScreen(&screens[j]);

                    if (screen && screen[0] != '\0')
                    {
                        if (!CONFIG.showEsc) printShorkLine(0);
                        if (CONFIG.mode == NORMAL)
                        {
                            if (!COMPACT)
                            {
                                // No compact - no bullet - single screen
                                if (noScreens == 1)
                                    outputPos += writeOutput(
                                        output + outputPos,
                                        OUTPUT_LEN - outputPos,
                                        "%sScreen:%s   %s\n", colAccent,
                                        colReset, screen);
                                // No compact - no bullet - multiple screens
                                // - first screen
                                else if (!pastFirst)
                                    outputPos += writeOutput(
                                        output + outputPos,
                                        OUTPUT_LEN - outputPos,
                                        "%sScreens:%s  %s%s\n", colAccent,
                                        colReset, icon, screen);
                                // No compact - no bullet - multiple screens
                                // - subsequent screens
                                else 
                                    outputPos += writeOutput(
                                        output + outputPos,
                                        OUTPUT_LEN - outputPos,
                                        "          %s%s\n", icon, screen);
                            }
                            else
                            {
                                // Compact - no bullet - single screen OR
                                // multiple screens - first screen
                                if (noScreens == 1 || !pastFirst)
                                    outputPos += writeOutput(
                                        output + outputPos,
                                        OUTPUT_LEN - outputPos,
                                        "%sScn:%s %s\n", colAccent,
                                        colReset, screen);
                                // Compact - no bullet - multiple screens -
                                // subsequent screens
                                else 
                                    outputPos += writeOutput(
                                        output + outputPos,
                                        OUTPUT_LEN - outputPos,
                                        "     %s\n", screen);
                            }
                        }
                        else
                        {
                            outputPos += writeOutput(output + outputPos,
                                OUTPUT_LEN - outputPos, "%s%s\n", icon,
                                screen);
                        }
                    }

                    free(screen);
                    pastFirst = 1;
                }
                free(screens);
            }
        }
        else if (strcmp(fieldsProcessed[i], "de") == 0)
        {
            if (de && de != wm && de[0] != '\0')
            {
                if (!CONFIG.showEsc) printShorkLine(0);
                if (CONFIG.mode == NORMAL)
                {
                    if (!COMPACT)
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, "%sDE:%s       %s\n",
                            colAccent, colReset, de);
                    else
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, "%sDE:%s  %s\n",
                            colAccent, colReset, de);
                }
                else
                {
                    outputPos += writeOutput(output + outputPos,
                        OUTPUT_LEN - outputPos, "%s%s\n", icon, de);
                }
            }
        }
        else if (strcmp(fieldsProcessed[i], "wm") == 0)
        {
            if (wm && wm[0] != '\0')
            {
                if (!CONFIG.showEsc) printShorkLine(0);
                char server[32] = "";
                if (!COMPACT)
                {
                    if (WAYLAND_PRESENT)
                        snprintf(server, 32, " (Wayland)");
                    else if (X11_PRESENT)
                        snprintf(server, 32, " (X11)");
                }
                if (CONFIG.mode == NORMAL)
                {
                    if (!COMPACT)
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, "%sWM:%s       %s%s\n",
                            colAccent, colReset, wm, server);
                    else
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, "%sWM:%s  %s\n",
                            colAccent, colReset, wm);
                }
                else 
                {
                    if (!COMPACT)
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, "%s%s%s\n",
                            icon, wm, server);
                    else
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, "%s%s\n",
                            icon, wm);
                }
            }
        }
        else if (strcmp(fieldsProcessed[i], "trm") == 0)
        {
            char *trm = getTerminal();
            if (trm)
            {
                if (!CONFIG.showEsc) printShorkLine(0);
                if (CONFIG.mode == NORMAL)
                {
                    if (!COMPACT)
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos,
                            "%sTerminal:%s %s\n", colAccent,
                            colReset, trm);
                    else
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, "%sTrm:%s %s\n",
                            colAccent, colReset, trm);
                }
                else
                {
                    if (!COMPACT)
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, "%s%s\n",
                            icon, trm);
                    else
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, "%s%s\n",
                            icon, trm);
                }
            }
            // If we don't have a terminal name, we can at least still show
            // the console size
            else
            {
                if (!CONFIG.showEsc) printShorkLine(0);
                if (CONFIG.mode == NORMAL)
                {
                    if (!COMPACT)
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, "%sConsole:%s  %dx%d\n",
                            colAccent, colReset, TERM_SIZE.ws_col,
                            TERM_SIZE.ws_row);
                    else
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, "%sCon:%s %dx%d\n",
                            colAccent, colReset, TERM_SIZE.ws_col,
                            TERM_SIZE.ws_row);
                }
                else
                {
                    if (!COMPACT)
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos,
                            "%s%dx%d console\n", icon, TERM_SIZE.ws_col,
                            TERM_SIZE.ws_row);
                    else
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, "%s%dx%dch\n",
                            icon, TERM_SIZE.ws_col, TERM_SIZE.ws_row);
                }
            }
            free(trm);
        }
        else if (strcmp(fieldsProcessed[i], "sh") == 0)
        {
            char *shell = getShell();
            if (shell && shell[0] != '\0')
            {
                if (!CONFIG.showEsc) printShorkLine(0);
                if (CONFIG.mode == NORMAL)
                {
                    if (!COMPACT)
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, "%sShell:%s    %s\n",
                            colAccent, colReset, shell);
                    else
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, "%sSh:%s  %s\n",
                            colAccent, colReset, shell);
                }
                else
                {
                    outputPos += writeOutput(output + outputPos,
                        OUTPUT_LEN - outputPos, "%s%s\n", icon, shell);
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
                    if (!CONFIG.showEsc) printShorkLine(0);
                    if (CONFIG.mode == NORMAL)
                    {
                        if (!COMPACT)
                            outputPos += writeOutput(output + outputPos,
                                OUTPUT_LEN - outputPos,
                                "%sCPU:%s      %s\n", colAccent, colReset,
                                cpuStr);
                        else
                            outputPos += writeOutput(output + outputPos,
                                OUTPUT_LEN - outputPos, "%sCPU:%s %s\n",
                                colAccent, colReset, cpuStr);
                    }
                    else
                    {
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, "%s%s\n",  icon,
                            cpuStr);
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
                int pastFirst = 0;
                char icon[100] = {0};
                if (CONFIG.charBullet != '\0')
                    snprintf(icon, sizeof(icon), " %s%c%s ", colBullet,
                        CONFIG.charBullet, colReset);

                for (int j = 0; j < noGPUs; j++)
                {
                    char *gpuStr = interpretGPU(&gpus[j], os);

                    if (gpuStr && gpuStr[0] != '\0')
                    {
                        if (!CONFIG.showEsc) printShorkLine(0);
                        if (CONFIG.mode == NORMAL)
                        {
                            if (!COMPACT)
                            {
                                if (noGPUs == 1)
                                    outputPos += writeOutput(
                                        output + outputPos,
                                        OUTPUT_LEN - outputPos,
                                        "%sGPU:%s      %s\n", colAccent,
                                        colReset, gpuStr);
                                else if (!pastFirst)
                                    outputPos += writeOutput(
                                        output + outputPos,
                                        OUTPUT_LEN - outputPos,
                                        "%sGPUs:%s     %s%s\n", colAccent,
                                        colReset, icon, gpuStr);
                                else
                                    outputPos += writeOutput(
                                        output + outputPos,
                                        OUTPUT_LEN - outputPos,
                                        "          %s%s\n", icon, gpuStr);
                            }
                            else
                            {
                                if (noGPUs == 1 || !pastFirst)
                                    outputPos += writeOutput(
                                        output + outputPos,
                                        OUTPUT_LEN - outputPos,
                                        "%sGPU:%s %s\n", colAccent,
                                        colReset, gpuStr);
                                else
                                    outputPos += writeOutput(
                                        output + outputPos,
                                        OUTPUT_LEN - outputPos,
                                        "     %s\n", gpuStr);
                            }
                        }
                        else
                        {
                            outputPos += writeOutput(output + outputPos,
                                OUTPUT_LEN - outputPos, "%s%s\n", icon,
                                gpuStr);
                        }
                    }

                    free(gpuStr);
                    pastFirst = 1;
                }
            }
            // If we found no GPUs the "traditional" way, at least check if
            // we received a fallback found during CPU name processing
            else if (gpuFromCPU && gpuFromCPU[0] != '\0')
            {
                if (!CONFIG.showEsc) printShorkLine(0);
                if (CONFIG.mode == NORMAL)
                {
                    if (!COMPACT)
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, "%sGPU:%s      %s\n",
                            colAccent, colReset, gpuFromCPU);
                    else
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, "%sGPU:%s %s\n",
                            colAccent, colReset, gpuFromCPU);
                }
                else
                {
                    outputPos += writeOutput(output + outputPos,
                        OUTPUT_LEN - outputPos, "%s%s\n", icon, gpuFromCPU);
                }
            }
            free(gpus);
        }
        else if (strcmp(fieldsProcessed[i], "ram") == 0 && mi.memTotal > 0)
        {
            char *ram = getRAM(mi, colPctLow, colPctMed, colPctHigh,
                colReset);
            if (ram)
            {
                if (!CONFIG.showEsc) printShorkLine(0);
                if (CONFIG.mode == NORMAL)
                {
                    if (!COMPACT)
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, "%sRAM:%s      %s\n",
                            colAccent, colReset, ram);
                    else
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, "%sRAM:%s %s\n",
                            colAccent, colReset, ram);
                }
                else 
                {
                    if (!COMPACT)
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, "%s%s RAM\n", icon,
                            ram);
                    else
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, "%s%s (R)\n", icon,
                            ram);
                }
            }
            free(ram);
        }
        else if (strcmp(fieldsProcessed[i], "swap") == 0 &&
            mi.swapTotal > 0)
        {
            char *swap = getSwap(mi, colPctLow, colPctMed, colPctHigh,
                colReset);
            if (swap && swap[0] != '\0')
            {
                if (!CONFIG.showEsc) printShorkLine(0);
                if (CONFIG.mode == NORMAL)
                {
                    if (!COMPACT)
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, "%sSwap:%s     %s\n",
                            colAccent, colReset, swap);
                    else
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, "%sSwp:%s %s\n",
                            colAccent, colReset, swap);
                }
                else 
                {
                    if (!COMPACT)
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, "%s%s swap\n",
                            icon, swap);
                    else
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, "%s%s (S)\n",
                            icon, swap);
                }
            }
            free(swap);
        }
        else if (strcmp(fieldsProcessed[i], "dsk") == 0)
        {
            DISKS *disks = getDisks();
            if (disks && disks->count > 0)
            {
                int pastFirst = 0;
                char icon[100] = {0};
                if (CONFIG.charBullet != '\0')
                    snprintf(icon, sizeof(icon), " %s%c%s ", colBullet,
                        CONFIG.charBullet, colReset);

                for (int i = 0; i < disks->count; i++)
                {
                    if (disks->disks[i][0] != '\0')
                    {
                        if (!CONFIG.showEsc) printShorkLine(0);
                        if (CONFIG.mode == NORMAL)
                        {
                            if (!COMPACT)
                            {
                                // No compact - no bullet - single disk
                                if (disks->count == 1)
                                    outputPos += writeOutput(
                                        output + outputPos,
                                        OUTPUT_LEN - outputPos,
                                        "%sDisk:%s     %s\n", colAccent,
                                        colReset, disks->disks[i]);
                                // No compact - no bullet - multiple disks -
                                // first disk
                                else if (!pastFirst)
                                    outputPos += writeOutput(output + 
                                        outputPos, OUTPUT_LEN - outputPos,
                                        "%sDisks:%s    %s%s\n", colAccent,
                                        colReset, icon, disks->disks[i]);
                                // No compact - no bullet - multiple disks -
                                // subsequent disks
                                else 
                                    outputPos += writeOutput(output +
                                        outputPos, OUTPUT_LEN - outputPos,
                                        "          %s%s\n", icon,
                                        disks->disks[i]);
                            }
                            else
                            {
                                // Compact - no bullet - single disks OR
                                // multiple disks - first screen
                                if (disks->count == 1 || !pastFirst)
                                    outputPos += writeOutput(
                                        output + outputPos,
                                        OUTPUT_LEN - outputPos,
                                        "%sDsk:%s %s\n", colAccent,
                                        colReset, disks->disks[i]);
                                // Compact - no bullet - multiple disks -
                                // subsequent disks
                                else 
                                    outputPos += writeOutput(
                                        output + outputPos,
                                        OUTPUT_LEN - outputPos, "     %s\n",
                                        disks->disks[i]);
                            }
                        }
                        else
                        {
                            outputPos += writeOutput(output + outputPos,
                                OUTPUT_LEN - outputPos, "%s%s\n", icon,
                                disks->disks[i]);
                        }
                    }
                    pastFirst = 1;
                }
            }
            free(disks);
        }
        else if (strcmp(fieldsProcessed[i], "root") == 0)
        {
            char *root = getRoot(colPctLow, colPctMed, colPctHigh,
                colReset);
            if (root && root[0] != '\0')
            {
                if (!CONFIG.showEsc) printShorkLine(0);
                if (CONFIG.mode == NORMAL)
                {
                    if (!COMPACT)
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, "%sRoot:%s     %s\n",
                            colAccent, colReset, root);
                    else
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, "%s/:%s   %s\n",
                            colAccent, colReset, root);
                }
                else 
                {
                    if (!COMPACT)
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, "%s%s root\n",
                            icon, root);
                    else
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, "%s%s (/)\n",
                            icon, root);
                }
            }
            free(root);
        }
        else if (strcmp(fieldsProcessed[i], "lip") == 0 && CONFIG.showIP)
        {
            char *localIP = getLocalIP();
            if (localIP)
            {
                if (!CONFIG.showEsc) printShorkLine(0);
                if (CONFIG.mode == NORMAL)
                {
                    if (!COMPACT)
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, "%sLocal IP:%s %s\n",
                            colAccent, colReset, localIP);
                    else
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, "%sLIP:%s %s\n",
                            colAccent, colReset, localIP);
                }
                else 
                {
                    if (!COMPACT)
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, "%s%s local\n",
                            icon, localIP);
                    else
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, "%s%s (L)\n",
                            icon, localIP);
                }
                free(localIP);
            }
        }
        else if (strcmp(fieldsProcessed[i], "xip") == 0 && CONFIG.showIP)
        {
            char *extIP = getExternalIP();
            if (extIP)
            {
                if (!CONFIG.showEsc) printShorkLine(0);
                if (CONFIG.mode == NORMAL)
                {
                    if (!COMPACT)
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, "%sExt. IP:%s  %s\n",
                            colAccent, colReset, extIP);
                    else
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, "%sXIP:%s %s\n",
                            colAccent, colReset, extIP);
                }
                else 
                {
                    if (!COMPACT)
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, "%s%s ext.\n",
                            icon, extIP);
                    else
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, "%s%s (X)\n",
                            icon, extIP);
                }
                free(extIP);
            }
        }
        else if (strcmp(fieldsProcessed[i], "clrs") == 0)
        {
            ColourPalette palette = getColourPalette();
            outputPos += writeOutput(output + outputPos,
                OUTPUT_LEN - outputPos, "%s\n", palette.baseCols);
            outputPos += writeOutput(output + outputPos,
                OUTPUT_LEN - outputPos, "%s\n", palette.brightCols);
        }
        else if (strcmp(fieldsProcessed[i], "clba") == 0)
        {
            ColourPalette palette = getColourPalette();
            outputPos += writeOutput(output + outputPos,
                OUTPUT_LEN - outputPos, "%s\n", palette.baseCols);
        }
        else if (strcmp(fieldsProcessed[i], "clbr") == 0)
        {
            ColourPalette palette = getColourPalette();
            outputPos += writeOutput(output + outputPos,
                OUTPUT_LEN - outputPos, "%s\n", palette.brightCols);
        }
    }

    // Print buffered output
    if (CONFIG.showEsc)
    {
        int shorkWidth = SHORK_NORM_WIDTH;
        int shorkHeight = SHORK_NORM_HEIGHT;
        if (!CONFIG.showArt)
            shorkWidth = 0;
        else if (COMPACT)
        {
            shorkWidth = SHORK_COMP_WIDTH;
            shorkHeight = SHORK_COMP_HEIGHT;
        }

        WORD_WRAPPED *data = NULL;
        if (CONFIG.mode == BULLETS)
            data = wordWrap(output, TERM_SIZE.ws_col - shorkWidth,
                "   ", NULL, 1, 0);
        else
        {
            if (COMPACT)
                data = wordWrap(output, TERM_SIZE.ws_col - shorkWidth,
                    "     ", NULL, 1, 0);
            else
            {
                const int prefixStrLen = 2 + strlen(colBullet) +
                    strlen(colReset);
                char *prefixStr = malloc(prefixStrLen);
                snprintf(prefixStr, prefixStrLen, " %s%c%s ", colBullet,
                    CONFIG.charBullet, colReset);
                data = wordWrap(output, TERM_SIZE.ws_col - shorkWidth,
                    "          ", prefixStr, 1, 0);
                free(prefixStr);
            }
        }

        if (data)
        {
            if (CONFIG.showArt)
            {
                printf("\033[%dA", shorkHeight);
                printf("\033[%dC", shorkWidth);
            }

            for (int i = 0; i < data->len; i++)
            {
                putchar(data->str[i]);
                if (CONFIG.showArt)
                {
                    if (data->str[i] == '\n')
                        printf("\033[%dC", shorkWidth);
                }
            }

            if (CONFIG.showArt && data->lines < shorkHeight)
                printf("\033[%dB", shorkHeight - data->lines);

            free(data->str);
            free(data);

            if (CONFIG.showArt)
                printf("\r");
        }
        else
            printf("ERROR: could not process output string\n");
    }
    // If needed, finish off printing the SHORK
    else if (CONFIG.showArt)
    {
        int target = SHORK_NORM_HEIGHT;
        if (COMPACT)
            target = SHORK_COMP_HEIGHT;
        for (int i = SHORK_LINE; i < target - 1; i++)
        {
            if (!printShorkLine(0))
                putchar ('\n');
        }
    }

    if (saveConf)
        writeConf(CONFIG);

    free(colAccent);
    free(colBullet);
    free(colPctLow);
    free(colPctMed);
    free(colPctHigh);
    free(hostname);
    free(os);
    if (de != wm)
        free(de);
    free(wm);
    free(gpuFromCPU);
    if (cpu)
    {
#ifndef X86_ONLY
        free(cpu->processor);
        free(cpu->uarch);
        free(cpu->platform);
        free(cpu->machine);
#endif
        free(cpu->vendor);
        free(cpu->name);
        free(cpu);
    }

    return 0;
}
