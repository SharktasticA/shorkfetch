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
#include "locale.h"
#include "memory.h"
#include "os.h"
#include "packages.h"
#include "screen.h"
#include "shell.h"
#include "terminal.h"
#include "testing.h"
#include "uptime.h"
#include "username.h"

#include <stdarg.h>



#define VERSION     "0.7-wip"



/**
 * Frees any malloc'd global variables.
 */
void freeGlobals(void)
{
    free(COL_ACCENT);
    free(COL_BULLET);
    free(COL_PCT_LOW);
    free(COL_PCT_MED);
    free(COL_PCT_HIGH);
}

/**
 * Prints a single line of the SHORK ASCII art. It is intended to be used
 * during the "assemble output" loop when in no-escape-codes mode.
 * @param stopOnceDone Flags if this function should stop once the SHORK is
 *                     complete
 * @return 1 if complete or inapplicable; 0 in progress
 */
int printShorkLine(int stopOnceDone)
{
    if (!SHOW_SHORK)
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

void showHelp(void)
{
    WORD_WRAPPED *desc = wordWrap("A tool that displays system, hardware "
        "and environment information in a summarised format.\n",
        TERM_SIZE.ws_col, NULL, NULL, 0, 0);
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

    WORD_WRAPPED *accentCol = wordWrap("-ac, --accent-col  Specifies a "
        "custom accent colour; no assignment returns the current colour "
        "and exits\n", TERM_SIZE.ws_col, "                   ", NULL, 0, 0);
    printf("%s", accentCol->str);
    free(accentCol->str);
    free(accentCol);

    WORD_WRAPPED *bullet = wordWrap("-b, --bullet       Specifies a custom "
        "character to use with bullet-point mode; no assignment returns "
        "the current character and exits\n", TERM_SIZE.ws_col,
        "                   ", NULL, 0, 0);
    printf("%s", bullet->str);
    free(bullet->str);
    free(bullet);

    WORD_WRAPPED *bulletCol = wordWrap("-bc, --bullet-col  Specifies a "
        "custom bullet-point colour; no assignment returns the current "
        "colour and exits\n", TERM_SIZE.ws_col, "                   ", NULL,
        0, 0);
    printf("%s", bulletCol->str);
    free(bulletCol->str);
    free(bulletCol);

    WORD_WRAPPED *compact = wordWrap("-co, --compact     Compacts field "
        "names and field values\n", TERM_SIZE.ws_col, "                   ",
        NULL, 0, 0);
    printf("%s", compact->str);
    free(compact->str);
    free(compact);

    WORD_WRAPPED *fields = wordWrap("-f, --fields       Specifies a custom "
        "fields list and order; no assignment returns list of current "
        "fields and exits\n", TERM_SIZE.ws_col, "                   ", NULL,
        0, 0);
    printf("%s", fields->str);
    free(fields->str);
    free(fields);

    WORD_WRAPPED *help = wordWrap("-h, --help         Displays help "
        "information and exits\n", TERM_SIZE.ws_col, "                   ",
        NULL, 0, 0);
    printf("%s", help->str);
    free(help->str);
    free(help);

    WORD_WRAPPED *highCol = wordWrap("-hc, --high-col    Specifies a "
        "custom high percentage colour; no assignment returns the current "
        "colour and exits\n", TERM_SIZE.ws_col, "                   ", NULL,
        0, 0);
    printf("%s", highCol->str);
    free(highCol->str);
    free(highCol);

    WORD_WRAPPED *lowCol = wordWrap("-lc, --low-col     Specifies a custom "
        "low percentage colour; no assignment returns the current colour "
        "and exits\n", TERM_SIZE.ws_col, "                   ", NULL, 0, 0);
    printf("%s", lowCol->str);
    free(lowCol->str);
    free(lowCol);

    WORD_WRAPPED *medCol = wordWrap("-mc, --med-col     Specifies a custom "
        "medium percentage colour; no assignment returns the current "
        "colour and exits\n", TERM_SIZE.ws_col, "                   ", NULL,
        0, 0);
    printf("%s", medCol->str);
    free(medCol->str);
    free(medCol);

    WORD_WRAPPED *mode = wordWrap("-mo, --mode        Specifies which view "
        "mode to use: [n]ormal (default), [b]ullets\n", TERM_SIZE.ws_col,
        "                   ", NULL, 0, 0);
    printf("%s", mode->str);
    free(mode->str);
    free(mode);

    WORD_WRAPPED *maxUnit = wordWrap("-mu, --max-unit    Specifies the "
        "largest data unit that can be displayed: [b]ytes, [k]ibi, [m]ebi, "
        "[g]ibi, [t]ebi, [p]ebi (default)\n", TERM_SIZE.ws_col,
        "                   ", NULL, 0, 0);
    printf("%s", maxUnit->str);
    free(maxUnit->str);
    free(maxUnit);

    WORD_WRAPPED *noArt = wordWrap("-na, --no-art      Disables the SHORK "
        "ASCII art\n", TERM_SIZE.ws_col, "                   ", NULL, 0, 0);
    printf("%s", noArt->str);
    free(noArt->str);
    free(noArt);

    WORD_WRAPPED *noEsc = wordWrap("-ne, --no-esc      Disables all ANSI "
        "escape codes and colour features\n", TERM_SIZE.ws_col,
        "                   ", NULL, 0, 0);
    printf("%s", noEsc->str);
    free(noEsc->str);
    free(noEsc);

    WORD_WRAPPED *reset = wordWrap("-r, --reset        Resets to default, "
        "deletes configuration file and exits\n", TERM_SIZE.ws_col,
        "                   ", NULL, 0, 0);
    printf("%s", reset->str);
    free(reset->str);
    free(reset);

    WORD_WRAPPED *save = wordWrap("-s, --save         Saves chosen options "
        "to a configuration file\n", TERM_SIZE.ws_col,
        "                   ", NULL, 0, 0);
    printf("%s", save->str);
    free(save->str);
    free(save);

    WORD_WRAPPED *version = wordWrap("-v, --version      Displays version "
        "number and exits\n\n", TERM_SIZE.ws_col, "                   ",
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

    WORD_WRAPPED *fieldNames = wordWrap("Fields: os, krn, upt, pkgs, loc, "
        "scn, de, wm, trm, sh, cpu, gpu, ram, swap, dsk, root, lip, clrs, "
        "clba, clbr, --- (separator), single blank space (blank line)\n",
        TERM_SIZE.ws_col, NULL, NULL, 0, 0);
    printf("%s", fieldNames->str);
    free(fieldNames->str);
    free(fieldNames);
}

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
    COL_ACCENT = strdup("bright_cyan");
    COL_BULLET = strdup("cyan");
    COL_PCT_LOW = strdup("green");
    COL_PCT_MED = strdup("yellow");
    COL_PCT_HIGH = strdup("red");
    HOME =  getenv("HOME");
    TERM_SIZE = getTerminalSize();

#ifndef EMBEDDED
    char *fields = strdup("---,os,krn,upt,pkgs,loc,scn,de,wm,trm,sh,cpu,"
        "gpu,ram,swap,dsk,root,lip, ,clrs, ");
#else
    char *fields = strdup("---,os,krn,upt,trm,sh,cpu,gpu,ram,swap,dsk,root,"
        " ");
#endif
    int noIP = 0;
    int saveConf = 0;
    VIEW_MODE mode = NORMAL;

    readConf(&CHAR_BULLET, &COL_ACCENT, &COL_BULLET, &COL_PCT_HIGH,
        &COL_PCT_LOW, &COL_PCT_MED, &COMPACT, &fields, &MAX_UNIT, &mode,
        &NO_ESC, &noIP, &SHOW_SHORK);

    for (int i = 1; i < argc; i++)
    {
        if ((strcmp(argv[i], "-h") == 0) ||
            (strcmp(argv[i], "--help") == 0))
        {
            showHelp();
            freeGlobals();
            free(fields);
            return 0;
        }
        else if (strncmp(argv[i], "-ac", 3) == 0 ||
            strncmp(argv[i], "--accent-col", 12) == 0)
        {
            // Find "=" as our needle
            char *equalsNeedle = strchr(argv[i], '=');
            if (!equalsNeedle) 
            {
                printf("%s\n", COL_ACCENT);
                freeGlobals();
                free(fields);
                return 1;
            }

            free(COL_ACCENT);
            equalsNeedle++;
            COL_ACCENT = strdup(equalsNeedle);
        }
        else if (strncmp(argv[i], "-bc", 3) == 0 ||
            strncmp(argv[i], "--bullet-col", 12) == 0)
        {
            // Find "=" as our needle
            char *equalsNeedle = strchr(argv[i], '=');
            if (!equalsNeedle) 
            {
                printf("%s\n", COL_BULLET);
                freeGlobals();
                free(fields);
                return 1;
            }

            free(COL_BULLET);
            equalsNeedle++;
            COL_BULLET = strdup(equalsNeedle);
        }
        else if (strncmp(argv[i], "-b", 2) == 0 ||
            strncmp(argv[i], "--bullet", 8) == 0)
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
                    printf("ERROR: custom bullet point character not "
                        "given\n");
                    free(fields);
                    return 1;
                }
                else if (bulletChar[1] != '\0')
                {
                    printf("ERROR: custom bullet point character can only "
                        "be a single character\n");
                    free(fields);
                    return 1;
                }
                CHAR_BULLET = bulletChar[0];
            }
            else
            {
                printf("\"%c\"\n", CHAR_BULLET);
                freeGlobals();
                free(fields);
                return 0;
            }
        }
        else if (strncmp(argv[i], "-lc", 3) == 0 ||
            strncmp(argv[i], "--low-col", 9) == 0)
        {
            // Find "=" as our needle
            char *equalsNeedle = strchr(argv[i], '=');
            if (!equalsNeedle) 
            {
                printf("%s\n", COL_PCT_LOW);
                freeGlobals();
                free(fields);
                return 1;
            }

            free(COL_PCT_LOW);
            equalsNeedle++;
            COL_PCT_LOW = strdup(equalsNeedle);
        }
        else if (strncmp(argv[i], "-mc", 3) == 0 ||
            strncmp(argv[i], "--med-col", 9) == 0)
        {
            // Find "=" as our needle
            char *equalsNeedle = strchr(argv[i], '=');
            if (!equalsNeedle) 
            {
                printf("%s\n", COL_PCT_MED);
                freeGlobals();
                free(fields);
                return 1;
            }

            free(COL_PCT_MED);
            equalsNeedle++;
            COL_PCT_MED = strdup(equalsNeedle);
        }
        else if (strncmp(argv[i], "-hc", 3) == 0 ||
            strncmp(argv[i], "--high-col", 10) == 0)
        {
            // Find "=" as our needle
            char *equalsNeedle = strchr(argv[i], '=');
            if (!equalsNeedle) 
            {
                printf("%s\n", COL_PCT_HIGH);
                freeGlobals();
                free(fields);
                return 1;
            }

            free(COL_PCT_HIGH);
            equalsNeedle++;
            COL_PCT_HIGH = strdup(equalsNeedle);
        }
        else if (strcmp(argv[i], "-co") == 0 ||
            strcmp(argv[i], "--compact") == 0)
            COMPACT = 1;
        else if (strncmp(argv[i], "-f", 2) == 0 ||
            strncmp(argv[i], "--fields", 8) == 0)
        {
            // Find "=" as our needle
            char *equalsNeedle = strchr(argv[i], '=');
            if (!equalsNeedle) 
            {
                printf("\"%s\"\n", fields);
                freeGlobals();
                free(fields);
                return 0;
            }

            equalsNeedle++;
            free(fields);
            fields = strdup(equalsNeedle);

            // Remove trailing comma if present
            int len = strlen(fields);
            if (len > 0 && fields[len - 1] == ',')
                fields[len - 1] = '\0';
        }
        else if (strncmp(argv[i], "-mo", 3) == 0 ||
            strncmp(argv[i], "--mode", 6) == 0)
        {
            char *modeVal = NULL;
            if (strncmp(argv[i], "-mo=", 4) == 0)
                modeVal = &argv[i][4];
            else if (strncmp(argv[i], "--mode=", 7) == 0)
                modeVal = &argv[i][7];

            if (modeVal)
            {
                if (modeVal[0] == '\0')
                {
                    printf("ERROR: no mode given\n");
                    free(fields);
                    freeGlobals();
                    return 1;
                }
                else if (strcmp(modeVal, "n") == 0 ||
                    strcmp(modeVal, "N") == 0 ||
                    strcmp(modeVal, "normal") == 0)
                    mode = NORMAL;
                else if (strcmp(modeVal, "b") == 0 ||
                    strcmp(modeVal, "B") == 0 ||
                    strcmp(modeVal, "bullet") == 0 ||
                    strcmp(modeVal, "bullets") == 0)
                    mode = BULLETS;
                else
                {
                    printf("ERROR: unrecognised mode \"%s\"\n", modeVal);
                    free(fields);
                    freeGlobals();
                    return 1;
                }
            }
            else
            {
                if (mode == NORMAL)
                    printf("\"normal\"\n");
                else if (mode == BULLETS)
                    printf("\"bullets\"\n");
                freeGlobals();
                free(fields);
                return 0;
            }
        }
        else if (strncmp(argv[i], "-mu", 3) == 0 ||
            strncmp(argv[i], "--max-unit", 10) == 0)
        {
            char *modeVal = NULL;
            if (strncmp(argv[i], "-mu=", 4) == 0)
                modeVal = &argv[i][4];
            else if (strncmp(argv[i], "--max-unit=", 11) == 0)
                modeVal = &argv[i][11];

            if (modeVal)
            {
                if (modeVal[0] == '\0')
                {
                    printf("ERROR: no unit given\n");
                    free(fields);
                    freeGlobals();
                    return 1;
                }
                else if (strcmp(modeVal, "b") == 0 ||
                    strcmp(modeVal, "B") == 0 ||
                    strcmp(modeVal, "byte") == 0)
                    MAX_UNIT = 'b';
                else if (strcmp(modeVal, "k") == 0 ||
                    strcmp(modeVal, "K") == 0 ||
                    strcmp(modeVal, "kibi") == 0 ||
                    strcmp(modeVal, "Kibi") == 0)
                    MAX_UNIT = 'k';
                else if (strcmp(modeVal, "m") == 0 ||
                    strcmp(modeVal, "M") == 0 ||
                    strcmp(modeVal, "mebi") == 0 ||
                    strcmp(modeVal, "Mebi") == 0)
                    MAX_UNIT = 'm';
                else if (strcmp(modeVal, "g") == 0 ||
                    strcmp(modeVal, "G") == 0 ||
                    strcmp(modeVal, "gibi") == 0 ||
                    strcmp(modeVal, "Gibi") == 0)
                    MAX_UNIT = 'g';
                else if (strcmp(modeVal, "t") == 0 ||
                    strcmp(modeVal, "T") == 0 ||
                    strcmp(modeVal, "tebi") == 0 ||
                    strcmp(modeVal, "Tebi") == 0)
                    MAX_UNIT = 't';
                else if (strcmp(modeVal, "p") == 0 ||
                    strcmp(modeVal, "P") == 0 ||
                    strcmp(modeVal, "pebi") == 0 ||
                    strcmp(modeVal, "Pebi") == 0)
                    MAX_UNIT = 'p';
                else
                {
                    printf("ERROR: unrecognised unit \"%s\"\n", modeVal);
                    free(fields);
                    freeGlobals();
                    return 1;
                }
            }
            else
            {
                printf("'%c'\n", MAX_UNIT);
                freeGlobals();
                free(fields);
                return 0;
            }
        }
        else if ((strcmp(argv[i], "-na") == 0) ||
            (strcmp(argv[i], "--no-art") == 0))
            SHOW_SHORK = 0;
        else if ((strcmp(argv[i], "-ne") == 0) ||
            (strcmp(argv[i], "--no-esc") == 0))
            NO_ESC = 1;
        else if ((strcmp(argv[i], "-ni") == 0) ||
            (strcmp(argv[i], "--no-ip") == 0))
            noIP = 1;
        else if ((strcmp(argv[i], "-r") == 0) ||
            (strcmp(argv[i], "--reset") == 0))
        {
            int result = deleteConf();
            if (result)
                printf("SHORKFETCH configuration reset\n");
            else
                printf("WARNING: SHORKFETCH configuration already "
                "default\n");
            freeGlobals();
            free(fields);
            return 0;
        }
        else if (strcmp(argv[i], "-s") == 0 ||
            strcmp(argv[i], "--save") == 0)
            saveConf = 1;
        else if (strcmp(argv[i], "-v") == 0 ||
            strcmp(argv[i], "--version") == 0)
        {
            printf("SHORKFETCH %s\n", VERSION);
            free(fields);
            freeGlobals();
            return 0;
        }
        else
        {
            printf("ERROR: unrecognised option \"%s\"\n", argv[i]);
            free(fields);
            freeGlobals();
            return 1;
        }
    }

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
    if (NO_ESC)
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
        colAccent = validateColour(COL_ACCENT);
        if (!colAccent)
        {
            printf("ERROR: unrecognised colour \"%s\"\n", COL_ACCENT);
            freeGlobals();
            free(fields);
            return 1;
        }
        colBullet = validateColour(COL_BULLET);
        if (!colBullet)
        {
            printf("ERROR: unrecognised colour \"%s\"\n", COL_BULLET);
            freeGlobals();
            free(fields);
            return 1;
        }
        colPctLow = validateColour(COL_PCT_LOW);
        if (!colPctLow)
        {
            printf("ERROR: unrecognised colour \"%s\"\n", COL_PCT_LOW);
            freeGlobals();
            free(fields);
            return 1;
        }
        colPctMed = validateColour(COL_PCT_MED);
        if (!colPctMed)
        {
            printf("ERROR: unrecognised colour \"%s\"\n", COL_PCT_MED);
            freeGlobals();
            free(fields);
            return 1;
        }
        colPctHigh = validateColour(COL_PCT_HIGH);
        if (!colPctHigh)
        {
            printf("ERROR: unrecognised colour \"%s\"\n", COL_PCT_HIGH);
            freeGlobals();
            free(fields);
            return 1;
        }
        colReset = (colAccent[0] == '\0') ? "" : "\033[" COL_RESET "m";
    }

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
                    printf("ERROR: too many fields given (max %d)\n",
                        MAX_FIELDS);
                    freeGlobals();
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
                freeGlobals();
                free(fields);
                return 1;
            }

            currTok = strtok(NULL, ",");
        }

        // If no-escape-codes mode, we need to string out colour palette
        // fields since they are not supported. For good measure, we also
        // remove any blank space padding above them.
        if (NO_ESC)
        {
            int write = 0;
            for (int read = 0; read < noFields; read++)
            {
                // Skip clrs, clba or clbr
                if (strcmp(fieldsProcessed[read], "clrs") == 0 ||
                    strcmp(fieldsProcessed[read], "clba") == 0 ||
                    strcmp(fieldsProcessed[read], "clbr") == 0)
                    continue;
                // Skip space if previous ele was clrs
                if (strcmp(fieldsProcessed[read], " ") == 0 && read > 0 &&
                    (strcmp(fieldsProcessed[read - 1], "clrs") == 0 ||
                    strcmp(fieldsProcessed[read - 1], "clba") == 0 ||
                    strcmp(fieldsProcessed[read - 1], "clbr") == 0))
                    continue;
                // Keep ele
                if (write != read)
                    strcpy(fieldsProcessed[write], fieldsProcessed[read]);
                write++;
            }
            noFields = write;
        }
    }
    else
    {
        printf("ERROR: no field names were given to display\n");
        freeGlobals();
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
    if (SHOW_SHORK && !NO_ESC)
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
    {
        if (NO_ESC) printShorkLine(0);
        outputPos += writeOutput(output + outputPos, OUTPUT_LEN - outputPos,
            "%s%s%s@%s%s%s\n", colAccent, username, colReset, colAccent,
            hostname, colReset);
        headerWidth = strlen(username) + 1 + strlen(hostname);
    }

    // Some things are dependent on others, so we have to look them up
    // regardless
    char *os = getOS(u, uStatus);
    char *de = getDE();
    char *wm = getWM(&de);
    char *gpuFromCPU = NULL;
    CPU_DATA *cpu = getCPU("/proc/cpuinfo", &gpuFromCPU);

    // Assemble output
    for (int i = 0; i < noFields; i++)
    {
        if (strcmp(fieldsProcessed[i], " ") == 0)
        {
            if (NO_ESC) printShorkLine(0);
            outputPos += writeOutput(output + outputPos,
                OUTPUT_LEN - outputPos, "\n");
        }
        else if (strcmp(fieldsProcessed[i], "---") == 0)
        {
            if (NO_ESC) printShorkLine(0);
            for (int i = 0; i < headerWidth; i++)
                outputPos += writeOutput(output + outputPos,
                    OUTPUT_LEN - outputPos, "-");
            outputPos += writeOutput(output + outputPos,
                OUTPUT_LEN - outputPos, "\n");
        }
        else if (strcmp(fieldsProcessed[i], "os") == 0)
        {
            if (os && os[0] != '\0')
            {
                if (NO_ESC) printShorkLine(0);
                if (mode == NORMAL)
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
                    char icon[10] = {CHAR_BULLET};
                    outputPos += writeOutput(output + outputPos,
                        OUTPUT_LEN - outputPos, " %s%s%s %s\n", colBullet,
                        icon, colReset, os);
                }
            }
        }
        else if (strcmp(fieldsProcessed[i], "krn") == 0)
        {
            char *kernel = getKernel(u, uStatus);
            if (kernel && kernel[0] != '\0')
            {
                if (NO_ESC) printShorkLine(0);
                if (mode == NORMAL)
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
                    char icon[10] = {CHAR_BULLET};
                    outputPos += writeOutput(output + outputPos,
                        OUTPUT_LEN - outputPos, " %s%s%s %s\n", colBullet,
                        icon, colReset, kernel);
                }
            }
            free(kernel);
        }
        else if (strcmp(fieldsProcessed[i], "upt") == 0)
        {
            char *uptime = getUptime();
            if (uptime && uptime[0] != '\0')
            {
                if (NO_ESC) printShorkLine(0);
                if (mode == NORMAL)
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
                    char icon[10] = {CHAR_BULLET};
                    outputPos += writeOutput(output + outputPos,
                        OUTPUT_LEN - outputPos, " %s%s%s %s\n", colBullet,
                        icon, colReset, uptime);
                }
            }
            free(uptime);
        }
        else if (strcmp(fieldsProcessed[i], "pkgs") == 0)
        {
            char *pkgs = getPackages(os);
            if (pkgs && pkgs[0] != '\0')
            {
                if (NO_ESC) printShorkLine(0);
                if (mode == NORMAL)
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
                    char icon[10] = {CHAR_BULLET};
                    outputPos += writeOutput(output + outputPos,
                        OUTPUT_LEN - outputPos, " %s%s%s %s\n", colBullet,
                        icon, colReset, pkgs);
                }
            }
            free(pkgs);
        }
        else if (strcmp(fieldsProcessed[i], "loc") == 0)
        {
            LOCALES *locales = getLocales();
            if (locales && locales->locales)
            {
                if (NO_ESC) printShorkLine(0);
                if (mode == NORMAL)
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
                    char icon[10] = {CHAR_BULLET};
                    outputPos += writeOutput(output + outputPos,
                        OUTPUT_LEN - outputPos, " %s%s%s %s\n", colBullet,
                        icon, colReset, locales->locales);
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
                if (CHAR_BULLET != '\0')
                    snprintf(icon, sizeof(icon), " %s%c%s ", colBullet,
                        CHAR_BULLET, colReset);

                for (int j = 0; j < noScreens; j++)
                {
                    char *screen = interpretScreen(&screens[j]);

                    if (screen && screen[0] != '\0')
                    {
                        if (NO_ESC) printShorkLine(0);
                        if (mode == NORMAL)
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
                if (NO_ESC) printShorkLine(0);
                if (mode == NORMAL)
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
                    char icon[10] = {CHAR_BULLET};
                    outputPos += writeOutput(output + outputPos,
                        OUTPUT_LEN - outputPos, " %s%s%s %s\n", colBullet,
                        icon, colReset, de);
                }
            }
        }
        else if (strcmp(fieldsProcessed[i], "wm") == 0)
        {
            if (wm && wm[0] != '\0')
            {
                if (NO_ESC) printShorkLine(0);
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
                    char icon[10] = {CHAR_BULLET};
                    if (!COMPACT)
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, " %s%s%s %s%s\n",
                            colBullet, icon, colReset, wm, server);
                    else
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, " %s%s%s %s\n",
                            colBullet, icon, colReset, wm);
                }
            }
        }
        else if (strcmp(fieldsProcessed[i], "trm") == 0)
        {
            char *trm = getTerminal();
            if (trm)
            {
                if (NO_ESC) printShorkLine(0);
                if (mode == NORMAL)
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
                    char icon[10] = {CHAR_BULLET};
                    if (!COMPACT)
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, " %s%s%s %s\n",
                            colBullet, icon, colReset, trm);
                    else
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, " %s%s%s %s\n",
                            colBullet, icon, colReset, trm);
                }
            }
            // If we don't have a terminal name, we can at least still show
            // the console size
            else
            {
                if (NO_ESC) printShorkLine(0);
                if (mode == NORMAL)
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
                    char icon[10] = {CHAR_BULLET};
                    if (!COMPACT)
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos,
                            " %s%s%s %dx%d console\n", colBullet, icon,
                            colReset, TERM_SIZE.ws_col, TERM_SIZE.ws_row);
                    else
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, " %s%s%s %dx%dch\n",
                            colBullet, icon, colReset, TERM_SIZE.ws_col,
                            TERM_SIZE.ws_row);
                }
            }
            free(trm);
        }
        else if (strcmp(fieldsProcessed[i], "sh") == 0)
        {
            char *shell = getShell();
            if (shell && shell[0] != '\0')
            {
                if (NO_ESC) printShorkLine(0);
                if (mode == NORMAL)
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
                    char icon[10] = {CHAR_BULLET};
                    outputPos += writeOutput(output + outputPos,
                        OUTPUT_LEN - outputPos, " %s%s%s %s\n", colBullet,
                        icon, colReset, shell);
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
                    if (NO_ESC) printShorkLine(0);
                    if (mode == NORMAL)
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
                        char icon[10] = {CHAR_BULLET};
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, " %s%s%s %s\n",
                            colBullet, icon, colReset, cpuStr);
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
                if (CHAR_BULLET != '\0')
                    snprintf(icon, sizeof(icon), " %s%c%s ", colBullet,
                        CHAR_BULLET, colReset);

                for (int j = 0; j < noGPUs; j++)
                {
                    char *gpuStr = interpretGPU(&gpus[j], os);

                    if (gpuStr && gpuStr[0] != '\0')
                    {
                        if (NO_ESC) printShorkLine(0);
                        if (mode == NORMAL)
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
                if (NO_ESC) printShorkLine(0);
                if (mode == NORMAL)
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
                    char icon[10] = {CHAR_BULLET};
                    outputPos += writeOutput(output + outputPos,
                        OUTPUT_LEN - outputPos, " %s%s%s %s\n", colBullet,
                        icon, colReset, gpuFromCPU);
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
                if (NO_ESC) printShorkLine(0);
                if (mode == NORMAL)
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
                    char icon[10] = {CHAR_BULLET};
                    if (!COMPACT)
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, " %s%s%s %s RAM\n",
                            colBullet, icon, colReset, ram);
                    else
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, " %s%s%s %s (R)\n",
                            colBullet, icon, colReset, ram);
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
                if (NO_ESC) printShorkLine(0);
                if (mode == NORMAL)
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
                    char icon[10] = {CHAR_BULLET};
                    if (!COMPACT)
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, " %s%s%s %s swap\n",
                            colBullet, icon, colReset, swap);
                    else
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, " %s%s%s %s (S)\n",
                            colBullet, icon, colReset, swap);
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
                if (CHAR_BULLET != '\0')
                    snprintf(icon, sizeof(icon), " %s%c%s ", colBullet,
                        CHAR_BULLET, colReset);

                for (int i = 0; i < disks->count; i++)
                {
                    if (disks->disks[i][0] != '\0')
                    {
                        if (NO_ESC) printShorkLine(0);
                        if (mode == NORMAL)
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
                if (NO_ESC) printShorkLine(0);
                if (mode == NORMAL)
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
                    char icon[10] = {CHAR_BULLET};
                    if (!COMPACT)
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, " %s%s%s %s root\n",
                            colBullet, icon, colReset, root);
                    else
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, " %s%s%s %s (/)\n",
                            colBullet, icon, colReset, root);
                }
            }
            free(root);
        }
        else if (strcmp(fieldsProcessed[i], "lip") == 0 && !noIP)
        {
            char *localIP = getLocalIP();
            if (localIP)
            {
                if (NO_ESC) printShorkLine(0);
                if (mode == NORMAL)
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
                    char icon[10] = {CHAR_BULLET};
                    if (!COMPACT)
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, " %s%s%s %s local\n",
                            colBullet, icon, colReset, localIP);
                    else
                        outputPos += writeOutput(output + outputPos,
                            OUTPUT_LEN - outputPos, " %s%s%s %s (L)\n",
                            colBullet, icon, colReset, localIP);
                }
                free(localIP);
            }
        }
        else if (strcmp(fieldsProcessed[i], "clrs") == 0)
        {
            ColourPalette palette = getColourPalette(SHOW_SHORK);
            outputPos += writeOutput(output + outputPos,
                OUTPUT_LEN - outputPos, "%s\n", palette.baseCols);
            outputPos += writeOutput(output + outputPos,
                OUTPUT_LEN - outputPos, "%s\n", palette.brightCols);
        }
        else if (strcmp(fieldsProcessed[i], "clba") == 0)
        {
            ColourPalette palette = getColourPalette(SHOW_SHORK);
            outputPos += writeOutput(output + outputPos,
                OUTPUT_LEN - outputPos, "%s\n", palette.baseCols);
        }
        else if (strcmp(fieldsProcessed[i], "clbr") == 0)
        {
            ColourPalette palette = getColourPalette(SHOW_SHORK);
            outputPos += writeOutput(output + outputPos,
                OUTPUT_LEN - outputPos, "%s\n", palette.brightCols);
        }
    }

    // Print buffered output
    if (!NO_ESC)
    {
        int shorkWidth = SHORK_NORM_WIDTH;
        int shorkHeight = SHORK_NORM_HEIGHT;
        if (!SHOW_SHORK)
            shorkWidth = 0;
        else if (COMPACT)
        {
            shorkWidth = SHORK_COMP_WIDTH;
            shorkHeight = SHORK_COMP_HEIGHT;
        }

        WORD_WRAPPED *data = NULL;
        if (mode == BULLETS)
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
                    CHAR_BULLET, colReset);
                data = wordWrap(output, TERM_SIZE.ws_col - shorkWidth,
                    "          ", prefixStr, 1, 0);
                free(prefixStr);
            }
        }

        if (data)
        {
            if (SHOW_SHORK)
            {
                printf("\033[%dA", shorkHeight);
                printf("\033[%dC", shorkWidth);
            }

            for (int i = 0; i < data->len; i++)
            {
                putchar(data->str[i]);
                if (SHOW_SHORK)
                {
                    if (data->str[i] == '\n')
                        printf("\033[%dC", shorkWidth);
                }
            }

            if (SHOW_SHORK && data->lines < shorkHeight)
                printf("\033[%dB", shorkHeight - data->lines);

            free(data->str);
            free(data);

            if (SHOW_SHORK)
                printf("\r");
        }
        else
            printf("ERROR: could not process output string\n");
    }
    // If needed, finish off printing the SHORK
    else if (SHOW_SHORK)
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
        writeConf(CHAR_BULLET, COL_ACCENT, COL_BULLET, COL_PCT_HIGH,
            COL_PCT_LOW, COL_PCT_MED, COMPACT, fieldsOrig, MAX_UNIT, mode,
            NO_ESC, noIP, SHOW_SHORK);

    freeGlobals();
    free(colAccent);
    free(colBullet);
    free(colPctLow);
    free(colPctMed);
    free(colPctHigh);
    free(fieldsOrig);
    free(fields);
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
