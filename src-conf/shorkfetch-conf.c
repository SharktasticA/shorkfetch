/*
    ######################################################
    ##          SHORK UTILITY - SHORKFETCH-CONF         ##
    ######################################################
    ## Main program logic                               ##
    ######################################################
    ## Licence: GNU GENERAL PUBLIC LICENSE Version 3    ##
    ######################################################
    ## Kali (links.sharktastica.co.uk)                  ##
    ######################################################
*/



#include "../shorkcommon/colours.h"
#include "../shorkcommon/shorkmenu.h"

#include "shorkfetch-conf.h"

#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



Config CONFIG = {
    '*',
    "bright_cyan",
    "cyan",
    "red",
    "green",
    "yellow",
    0,
    "head,---,os,krn,upt,pkgs,loc,scn,de,wm,trm,sh,cpu,gpu,ram,swap,dsk,root,lip, ,clrs, ",
    'p',
    NORMAL,
    0,
    0,
    0,
    0
};
char FIELDS[MAX_FIELDS][5];
int NO_FIELDS = 0;


/**
 * Shows a menu of possible field types to select.
 * @return Index of POSSIBLE_FIELDS that was selected; -1 if quit
 */
int addField(void)
{
    MenuItem menu[POSSIBLE_FIELDS_LEN];
    memset(menu, 0, sizeof(menu));
    for (int i = 0; i < POSSIBLE_FIELDS_LEN; i++)
    {
        snprintf(menu[i].id, MENU_ITEM_ID_LEN, "%s",
            POSSIBLE_FIELDS[i].id);
        snprintf(menu[i].name, MENU_ITEM_NAME_LEN, "%s",
            POSSIBLE_FIELDS[i].name);
        menu[i].isVisible = 1;
        menu[i].isStatic = 0;
    }

    int running = 1;
    int cursorX = 1;
    int cursorY = 1;
    int cursorXPrev = 1;
    int cursorYPrev = 0;
    int fullRedraw = 1;

    while (running)
    {
        if (fullRedraw)
        {
            clearScreen();
            printHeader("Select field to add");
            printMenu(menu, POSSIBLE_FIELDS_LEN, NULL, 1,
                TERM_SIZE.ws_col - 6, POSSIBLE_FIELDS_LEN, &cursorX,
                &cursorY, &cursorXPrev, &cursorYPrev);
            printFooter("[jk] Navigate [Enter] Select [q] Back");
        }
        else
        {
            if (COL_ENABLED)
                printf("\x1b[2;1H");
            else
                printf("\x1b[3;1H");
            printMenu(menu, POSSIBLE_FIELDS_LEN, NULL, 1,
                TERM_SIZE.ws_col - 6, POSSIBLE_FIELDS_LEN, &cursorX,
                &cursorY, &cursorXPrev, &cursorYPrev);
        }

        NavInput input = getNavInput();

        fullRedraw = 1;
        cursorYPrev = 0;
        switch (input)
        {
            case CURSOR_UP:
                cursorYPrev = cursorY;
                cursorY--;
                if (cursorY < 1) cursorY = POSSIBLE_FIELDS_LEN;
                fullRedraw = 0;
                break;

            case CURSOR_DOWN:
                cursorYPrev = cursorY;
                cursorY++;
                if (cursorY > POSSIBLE_FIELDS_LEN) cursorY = 1;
                fullRedraw = 0;
                break;

            case ENTER:
                return cursorY - 1;
                break;

            case QUIT:
                running = 0;
                break;

            default:
                fullRedraw = 0;
                break;
        }
    }

    return -1;
}

void asciiArtMenu(void)
{
    toggleOptionMenu("ASCII art",
        "Controls whether the ASCII art to the left of SHORKFETCH's output "
        "is displayed",
        &CONFIG.noArt, 1);
}

void colAccMenu(void)
{
    colMenu("Colour for accents",
        "Sets the general accent colour used for ASCII art and field names",
        CONFIG.colAccent);
}

void colBulMenu(void)
{
    colMenu("Colour for bullets",
        "Sets the bullet character colour when in bullet mode and for "
        "sublists",
        CONFIG.colBullet);
}

/**
 * Standardised menu for colour selecting options.
 * @param title Menu title
 * @param desc Menu description (can be NULL to skip)
 * @param val Option of char[CONF_COL_LEN] to change
 */
void colMenu(char *title, char *desc, char *val)
{
    MenuItem menu[] = {
        { "black", "Black", NULL, NULL, 1, 0 },
        { "blue", "Blue", NULL, NULL, 1, 0 },
        { "bright_blue", "Bright blue", NULL, NULL, 1, 0 },
        { "bright_cyan", "Bright cyan", NULL, NULL, 1, 0 },
        { "bright_green", "Bright green", NULL, NULL, 1, 0 },
        { "bright_magenta", "Bright magenta", NULL, NULL, 1, 0 },
        { "bright_red", "Bright red", NULL, NULL, 1, 0 },
        { "bright_white", "Bright white", NULL, NULL, 1, 0 },
        { "bright_yellow", "Bright yellow", NULL, NULL, 1, 0 },
        { "cyan", "Cyan", NULL, NULL, 1, 0 },
        { "green", "Green", NULL, NULL, 1, 0 },
        { "grey", "Grey", NULL, NULL, 1, 0 },
        { "magenta", "Magenta", NULL, NULL, 1, 0 },
        { "red", "Red", NULL, NULL, 1, 0 },
        { "white", "White", NULL, NULL, 1, 0 },
        { "yellow", "Yellow", NULL, NULL, 1, 0 },
        { "off", "Off (use terminal's foreground colour)", NULL, NULL, 1, 0 },
    };
    int menuSize = sizeof(menu) / sizeof(menu[0]);

    int running = 1;
    int cursorX = 1;
    int cursorY = 1;
    int cursorXPrev = 1;
    int cursorYPrev = 0;
    int fullRedraw = 1;

    while (running)
    {
        if (fullRedraw)
        {
            for (int i = 0; i < menuSize; i++)
            {
                if (!strcmp(menu[i].id, val))
                {
                    markEntry(menu[i].name, COL_FOR_GREEN, 0);
                    cursorY = i + 1;
                }
                else
                    unmarkEntry(menu[i].name);
            }

            clearScreen();
            printHeader(title);
            printMenu(menu, menuSize, desc, 1, TERM_SIZE.ws_col - 6,
                menuSize, &cursorX, &cursorY, &cursorXPrev, &cursorYPrev);
            printFooter("[jk] Navigate [Enter] Select [q] Back");
        }
        else
        {
            if (COL_ENABLED)
                printf("\x1b[2;1H");
            else
                printf("\x1b[3;1H");
            printMenu(menu, menuSize, desc, 1, TERM_SIZE.ws_col - 6,
                menuSize, &cursorX, &cursorY,  &cursorXPrev, &cursorYPrev);
        }

        NavInput input = getNavInput();

        fullRedraw = 1;
        cursorYPrev = 0;
        switch (input)
        {
            case CURSOR_UP:
                cursorYPrev = cursorY;
                cursorY--;
                if (cursorY < 1) cursorY = menuSize;
                fullRedraw = 0;
                break;

            case CURSOR_DOWN:
                cursorYPrev = cursorY;
                cursorY++;
                if (cursorY > menuSize) cursorY = 1;
                fullRedraw = 0;
                break;

            case ENTER:
                snprintf(val, CONF_COL_LEN, "%s", menu[cursorY-1].id);
                writeConf(CONFIG);
                break;

            case QUIT:
                running = 0;
                break;

            default:
                fullRedraw = 0;
                break;
        }
    }
}

void colPclMenu(void)
{
    colMenu("Colour for low \%",
        "Sets the colour for a percentage when it is lower than 50\%",
        CONFIG.colPctLow);
}

void colPcmMenu(void)
{
    colMenu("Colour for medium \%",
        "Sets the colour for a percentage when it is between 50% and 79\%",
        CONFIG.colPctMed);
}

void colPchMenu(void)
{
    colMenu("Colour for high \%",
        "Sets the colour for a percentage when it is 80% or higher",
        CONFIG.colPctHigh);
}

void compViewMenu(void)
{
    toggleOptionMenu("Compact view",
        "Compact mode limits field names to just 3 letters, uses "
        "appropriate shorthands and contractions, and removes any "
        "non-essential data that does not compromise the overall result",
        &CONFIG.compact, 0);
}

/**
 * Shows a menu to select what action (move, add, remove, etc.) to apply at
 * the selected field.
 * @param name Selected field's name
 * @param id Selected field's current index number
 * @return Selected field's index number after any changes or previous index
 *         number if removed
 */
int fieldActionsMenu(char *name, const int ind)
{
    int returnInd = ind;

    MenuItem rawMenu[] = {
        { "movB", "Move to bottom", NULL, NULL, ind < (NO_FIELDS - 1), 0 },
        { "movT", "Move to top", NULL, NULL, ind > 0, 0 },
        { "movU", "Move up", NULL, NULL, ind > 0, 0 },
        { "movD", "Move down", NULL, NULL, ind < (NO_FIELDS - 1), 0 },
        { "addA", "Add above", NULL, NULL, NO_FIELDS < MAX_FIELDS, 0 },
        { "addB", "Add below", NULL, NULL, NO_FIELDS < MAX_FIELDS, 0 },
        { "del", "Remove", NULL, NULL, 1, 0 }
    };
    int rawMenuSize = sizeof(rawMenu) / sizeof(rawMenu[0]);

    // Filter menu to just what should actually be visible
    MenuItem menu[rawMenuSize];
    int menuSize = 0;
    for (int i = 0; i < rawMenuSize; i++)
        if (rawMenu[i].isVisible)
            menu[menuSize++] = rawMenu[i];
    freeMenu(rawMenu, rawMenuSize);

    int running = 1;
    int cursorX = 1;
    int cursorY = 1;
    int cursorXPrev = 1;
    int cursorYPrev = 0;
    int fullRedraw = 1;

    char header[MENU_ITEM_NAME_LEN];
    snprintf(header, MENU_ITEM_NAME_LEN, "Actions for field %d (%s)",
        ind + 1, name);

    while (running)
    {
        if (fullRedraw)
        {
            clearScreen();
            printHeader(header);
            printMenu(menu, menuSize, NULL, 1, TERM_SIZE.ws_col - 6,
                menuSize, &cursorX, &cursorY, &cursorXPrev, &cursorYPrev);
            printFooter("[jk] Navigate [Enter] Select [q] Back");
        }
        else
        {
            if (COL_ENABLED)
                printf("\x1b[2;1H");
            else
                printf("\x1b[3;1H");
            printMenu(menu, menuSize, NULL, 1, TERM_SIZE.ws_col - 6,
                menuSize, &cursorX, &cursorY, &cursorXPrev, &cursorYPrev);
        }

        NavInput input = getNavInput();

        fullRedraw = 1;
        cursorYPrev = 0;
        switch (input)
        {
            case CURSOR_UP:
                cursorYPrev = cursorY;
                cursorY--;
                if (cursorY < 1) cursorY = menuSize;
                fullRedraw = 0;
                break;

            case CURSOR_DOWN:
                cursorYPrev = cursorY;
                cursorY++;
                if (cursorY > menuSize) cursorY = 1;
                fullRedraw = 0;
                break;

            case ENTER:
                returnInd = modifyField(menu[cursorY - 1], ind);
                running = 0;
                break;

            case QUIT:
                running = 0;
                break;

            default:
                fullRedraw = 0;
                break;
        }
    }

    return returnInd;
}

void fieldsMenu(void)
{
    MenuItem menu[MAX_FIELDS];
    int menuSize = 0;

    int running = 1;
    int cursorX = 1;
    int cursorY = 1;
    int cursorXPrev = 1;
    int cursorYPrev = 0;
    int fullRedraw = 1;

    char msg[200] = "The fields SHORKFETCH should try to display. You can "
        "select a field to move it up/down, insert field above/below it, "
        "or remove it. You may specify a total of 50 fields.";

    while (running)
    {
        if (fullRedraw)
        {
            if (populateFieldsMenu(menu, MAX_FIELDS) != 0)
            {
                EXIT_MSG = strdup("ERROR: could not parse field data");
                exit(1);
            }
            menuSize = NO_FIELDS;
            
            // If no fields, give *something* to use...
            if (NO_FIELDS == 0)
            {
                snprintf(menu[0].id, MENU_ITEM_ID_LEN, "add");
                snprintf(menu[0].name, MENU_ITEM_NAME_LEN, "Add field");
                menu[0].isVisible = 1;
                menuSize = 1;
            }

            clearScreen();
            printHeader("Fields");
            printMenu(menu, menuSize, msg, 1, TERM_SIZE.ws_col - 6,
                menuSize, &cursorX, &cursorY, &cursorXPrev, &cursorYPrev);
            printFooter("[jk] Navigate [Enter] Select [q] Back");
        }
        else
        {
            if (COL_ENABLED)
                printf("\x1b[2;1H");
            else
                printf("\x1b[3;1H");
            printMenu(menu, menuSize, msg, 1, TERM_SIZE.ws_col - 6,
                menuSize, &cursorX, &cursorY, &cursorXPrev, &cursorYPrev);
        }

        NavInput input = getNavInput();

        fullRedraw = 1;
        cursorYPrev = 0;
        switch (input)
        {
            case CURSOR_UP:
                cursorYPrev = cursorY;
                cursorY--;
                if (cursorY < 1) cursorY = menuSize;
                fullRedraw = 0;
                break;

            case CURSOR_DOWN:
                cursorYPrev = cursorY;
                cursorY++;
                if (cursorY > menuSize) cursorY = 1;
                fullRedraw = 0;
                break;

            case ENTER:
                if (NO_FIELDS == 0)
                {
                    int selected = addField();
                    if (selected != -1)
                    {
                        snprintf(FIELDS[0], sizeof(FIELDS[0]), "%s",
                            POSSIBLE_FIELDS[selected].id);
                        NO_FIELDS = 1;
                        saveFields();
                        cursorY = 1;
                    }
                }
                else
                    cursorY = fieldActionsMenu(menu[cursorY - 1].name,
                        cursorY - 1) + 1;
                break;

            case QUIT:
                running = 0;
                break;

            default:
                fullRedraw = 0;
                break;
        }
    }
}

void escCodesMenu(void)
{
    toggleOptionMenu("Escape codes",
        "Controls whether SHORKFETCH is allowed to use ANSI escape codes "
        "in its output. Disabling them can increase compatibility with "
        "programs that interpret and/or modify the output of another, but "
        "it will also disable SHORKFETCH's colour output, colour palette "
        "fields and word-wrapping for small terminals.",
        &CONFIG.noEsc, 1);
}

void ihaMenu(void)
{
    toggleOptionMenu("Intel Hybrid Architecture support",
        "Controls whether Intel Hybrid Architecture-specific core/thread "
            "counting is used, which distinguishes low-power efficient "
            "(L), efficient (E), and performance (P) cores from a combined "
            "core count for applicable CPUs. For example, the Intel Core "
            "Ultra 7 155H's core/thread count would be \"2L/8E/6P/22T\" "
            "instead of \"16C/22T\".",
        &CONFIG.noIHA, 1);
}

/**
 * Shows SHORKFETCH-CONF's main menu.
 * @returns 0 if exits normally
 */
int mainMenu(void)
{
    setupMenuSys(42, 12);
    readConf(&CONFIG);

    MenuItem menu[] = {
        { "art", "ASCII art", "", asciiArtMenu, 1, 0 },
        { "acc", "Colour for accents", "", colAccMenu, 1, 0 },
        { "bul", "Colour for bullets", "", colBulMenu, 1, 0 },
        { "pcl", "Colour for low \%", "", colPclMenu, 1, 0 },
        { "pcm", "Colour for medium \%", "", colPcmMenu, 1, 0 },
        { "pch", "Colour for high \%", "", colPchMenu, 1, 0 },
        { "com", "Compact view", "", compViewMenu, 1, 0 },
        { "esc", "Escape codes", "", escCodesMenu, 1, 0 },
        { "fld", "Fields", "", fieldsMenu, 1, 0 },
        { "iha", "Intel Hybrid Architecture support", "", ihaMenu, 1, 0 },
        { "unt", "Maximum data unit", "", maxUnitMenu, 1, 0 },
        { "mde", "View mode", "", viewModeMenu, 1, 0 }
    };
    int menuSize = sizeof(menu) / sizeof(menu[0]);

    int running = 1;
    int cursorX = 1;
    int cursorY = 1;
    int cursorXPrev = 1;
    int cursorYPrev = 0;
    int fullRedraw = 1;

    while (running)
    {
        if (fullRedraw)
        {
            clearScreen();
            printHeader("SHORKFETCH Configurator");
            printMenu(menu, menuSize, NULL, 1, TERM_SIZE.ws_col - 6,
                menuSize, &cursorX, &cursorY, &cursorXPrev, &cursorYPrev);
            printFooter("[jk] Navigate [Enter] Select [q] Quit");
        }
        else
        {
            if (COL_ENABLED)
                printf("\x1b[2;1H");
            else
                printf("\x1b[3;1H");
            printMenu(menu, menuSize, NULL, 1, TERM_SIZE.ws_col - 6,
                menuSize, &cursorX, &cursorY, &cursorXPrev, &cursorYPrev);
        }

        NavInput input = getNavInput();

        fullRedraw = 1;
        cursorYPrev = 0;
        switch (input)
        {
            case CURSOR_UP:
                cursorYPrev = cursorY;
                cursorY--;
                if (cursorY < 1) cursorY = menuSize;
                fullRedraw = 0;
                break;

            case CURSOR_DOWN:
                cursorYPrev = cursorY;
                cursorY++;
                if (cursorY > menuSize) cursorY = 1;
                fullRedraw = 0;
                break;

            case ENTER:
                menu[cursorY - 1].action();
                break;

            case QUIT:
                running = 0;
                break;

            default:
                fullRedraw = 0;
                break;
        }
    }

    clearScreen();
    return 0;
}

void maxUnitMenu(void)
{
    MenuItem menu[] = {
        { "b", "Bytes", NULL, NULL, 1, 0 },
        { "k", "Kibibytes", NULL, NULL, 1, 0 },
        { "m", "Mebibytes", NULL, NULL, 1, 0 },
        { "g", "Gibibytes", NULL, NULL, 1, 0 },
        { "t", "Tebibytes", NULL, NULL, 1, 0 },
        { "p", "Pebibytes", NULL, NULL, 1, 0 }
    };
    int menuSize = sizeof(menu) / sizeof(menu[0]);

    int running = 1;
    int cursorX = 1;
    int cursorY = 1;
    int cursorXPrev = 1;
    int cursorYPrev = 0;
    int fullRedraw = 1;

    char msg[200] = "The largest data unit SHORKFETCH will display on "
        "applicable fields. Any smaller units than selected will still be "
        "used if appropriate for the given data's magnitude.";

    while (running)
    {
        if (fullRedraw)
        {
            for (int i = 0; i < menuSize; i++)
            {
                if (menu[i].id[0] == CONFIG.maxUnit)
                {
                    markEntry(menu[i].name, COL_FOR_GREEN, 0);
                    cursorY = i + 1;
                }
                else
                    unmarkEntry(menu[i].name);
            }

            clearScreen();
            printHeader("Maximum data unit");
            printMenu(menu, menuSize, msg, 1, TERM_SIZE.ws_col - 6,
                menuSize, &cursorX, &cursorY, &cursorXPrev, &cursorYPrev);
            printFooter("[jk] Navigate [Enter] Select [q] Back");
        }
        else
        {
            if (COL_ENABLED)
                printf("\x1b[2;1H");
            else
                printf("\x1b[3;1H");
            printMenu(menu, menuSize, msg, 1, TERM_SIZE.ws_col - 6,
                menuSize, &cursorX, &cursorY, &cursorXPrev, &cursorYPrev);
        }

        NavInput input = getNavInput();

        fullRedraw = 1;
        cursorYPrev = 0;
        switch (input)
        {
            case CURSOR_UP:
                cursorYPrev = cursorY;
                cursorY--;
                if (cursorY < 1) cursorY = menuSize;
                fullRedraw = 0;
                break;

            case CURSOR_DOWN:
                cursorYPrev = cursorY;
                cursorY++;
                if (cursorY > menuSize) cursorY = 1;
                fullRedraw = 0;
                break;

            case ENTER:
                CONFIG.maxUnit = menu[cursorY - 1].id[0];
                writeConf(CONFIG);
                break;

            case QUIT:
                running = 0;
                break;

            default:
                fullRedraw = 0;
                break;
        }
    }
}

/**
 * Performs the desired action to FIELDS the selected field index. Should
 * only be called by fieldActionsMenu().
 * @param action Selected action's MenuItem from 
 * @param int Selected field's current index number
 * @return Selected field's index number after any changes or previous index
 *         number if deleted
 */
int modifyField(MenuItem action, const int ind)
{
    int returnInd = ind;

    if (!strncmp(action.id, "mov", 3))
    {
        // Use returnInd as our "to" and ind as our "from"
        if (!strcmp(action.id, "movT"))
            returnInd = 0;
        else if (!strcmp(action.id, "movB"))
            returnInd = NO_FIELDS - 1;
        else if (!strcmp(action.id, "movU"))
            returnInd = ind - 1;
        else if (!strcmp(action.id, "movD"))
            returnInd = ind + 1;

        char tmp[sizeof(FIELDS[0])];
        memcpy(tmp, FIELDS[ind], sizeof(tmp));

        if (ind < returnInd)
            memmove(FIELDS[ind], FIELDS[ind + 1],
                (returnInd - ind) * sizeof(FIELDS[0]));
        else
            memmove(FIELDS[returnInd + 1], FIELDS[returnInd],
                (ind - returnInd) * sizeof(FIELDS[0]));

        memcpy(FIELDS[returnInd], tmp, sizeof(tmp));
    }
    else if (!strncmp(action.id, "add", 3))
    {
        int selected = addField();
        // -1 means user selected "Back", so just exit
        if (selected == -1)
            return returnInd;

        int pos = !strcmp(action.id, "addA") ? ind : ind + 1;

        memmove(FIELDS[pos + 1], FIELDS[pos],
            (NO_FIELDS - pos) * sizeof(FIELDS[0]));

        snprintf(FIELDS[pos], sizeof(FIELDS[0]), "%s",
            POSSIBLE_FIELDS[selected].id);
        NO_FIELDS++;

        returnInd = pos;
    }
    else if (!strcmp(action.id, "del"))
    {
        // Remove the field at ind by shifting everything below it up one
        // index
        memmove(FIELDS[ind], FIELDS[ind + 1],
            (NO_FIELDS - ind - 1) * sizeof(FIELDS[0]));

        // Clear the now-unused last slot and shrink field count
        NO_FIELDS--;
        memset(FIELDS[NO_FIELDS], 0, sizeof(FIELDS[0]));

        if (ind == 0)
            returnInd = 0;
        else
            returnInd--;
    }

    saveFields();
    return returnInd;
}

/**
 * Fills a given MenuItem menu with SHORKFETCH fields loaded in CONFIG.
 * @param menu Menu to (re)populate
 * @return 0 on success; -1 on error
 */
int populateFieldsMenu(MenuItem *menu, const int menuSize)
{
    if (menu == NULL)
        return -1;

    // Zero out the menu
    memset(menu, 0, sizeof(MenuItem) * menuSize);

    // Validate CONFIG.fields and get an array of fields
    NO_FIELDS = validateFields(CONFIG.fields, FIELDS, 0);
    if (NO_FIELDS == -1)
        return -1;

    // Copy the fields into the menu
    for(int i = 0; i < NO_FIELDS; i++)
    {
        snprintf(menu[i].id, MENU_ITEM_ID_LEN, "%s", FIELDS[i]);
        for (int j = 0; j < POSSIBLE_FIELDS_LEN; j++)
        {
            if (!strcmp(FIELDS[i], POSSIBLE_FIELDS[j].id))
            {
                snprintf(menu[i].name, MENU_ITEM_NAME_LEN, "%s",
                    POSSIBLE_FIELDS[j].name);
                break;
            }
        }
        menu[i].isVisible = 1;
    }

    return 0;
}

/**
 * Saves the contents of the FIELDS array into the CONFIG.fields string.
 */
void saveFields(void)
{
    char buffer[CONF_FIELDS_LEN];
    buffer[0] = '\0';

    for (int i = 0; i < NO_FIELDS; i++)
    {
        if (i > 0)
            strncat(buffer, ",", CONF_FIELDS_LEN - strlen(buffer) - 1);
        strncat(buffer, FIELDS[i], CONF_FIELDS_LEN - strlen(buffer) - 1);
    }

    snprintf(CONFIG.fields, sizeof(CONFIG.fields), "%s", buffer);
    writeConf(CONFIG);
}

/**
 * Standardised menu for simple toggle (enable/disable) options.
 * @param title Menu title
 * @param desc Menu description (can be NULL to skip)
 * @param val Integer whose value to flip (passed by reference)
 * @param invert Flags if the toggled value should be inverted (for when
 *               dealing with a no* variable instead of a is*)
 */
void toggleOptionMenu(char *title, char *desc, int *val, int invert)
{
    MenuItem menu[] = {
        { "enb", "Enable", NULL, NULL, 1, 0 }
    };
    int menuSize = sizeof(menu) / sizeof(menu[0]);

    int running = 1;
    int cursorX = 1;
    int cursorY = 1;
    int cursorXPrev = 1;
    int cursorYPrev = 0;
    int fullRedraw = 1;

    while (running)
    {
        if (fullRedraw)
        {
            if (invert ? !*val : *val)
            {
                snprintf(menu[0].name, MENU_ITEM_NAME_LEN, "Disable");
                markEntry(menu[0].name, COL_FOR_RED, 1);
            }
            else
            {
                snprintf(menu[0].name, MENU_ITEM_NAME_LEN, "Enable");
                markEntry(menu[0].name, COL_FOR_GREEN, 1);
            }

            clearScreen();
            printHeader(title);
            printMenu(menu, menuSize, desc, 1, TERM_SIZE.ws_col - 6,
                menuSize, &cursorX, &cursorY, &cursorXPrev, &cursorYPrev);
            printFooter("[jk] Navigate [Enter] Toggle [q] Back");
        }
        else
        {
            if (COL_ENABLED)
                printf("\x1b[2;1H");
            else
                printf("\x1b[3;1H");
            printMenu(menu, menuSize, desc, 1, TERM_SIZE.ws_col - 6,
                menuSize, &cursorX, &cursorY,  &cursorXPrev, &cursorYPrev);
        }

        NavInput input = getNavInput();

        fullRedraw = 1;
        cursorYPrev = 0;
        switch (input)
        {
            case CURSOR_UP:
                cursorYPrev = cursorY;
                cursorY--;
                if (cursorY < 1) cursorY = menuSize;
                fullRedraw = 0;
                break;

            case CURSOR_DOWN:
                cursorYPrev = cursorY;
                cursorY++;
                if (cursorY > menuSize) cursorY = 1;
                fullRedraw = 0;
                break;

            case ENTER:
                *val = !*val;
                writeConf(CONFIG);
                break;

            case QUIT:
                running = 0;
                break;

            default:
                fullRedraw = 0;
                break;
        }
    }
}

void viewModeMenu(void)
{
    MenuItem menu[] = {
        { "bul", "Bulleted list", NULL, NULL, 1, 0 },
        { "nrm", "Normal (field names)", NULL, NULL, 1, 0 }
    };
    int menuSize = sizeof(menu) / sizeof(menu[0]);

    int running = 1;
    int cursorX = 1;
    int cursorY = 1;
    int cursorXPrev = 1;
    int cursorYPrev = 0;
    int fullRedraw = 1;

    char msg[200] = "The overall view mode SHORKFETCH should use";

    while (running)
    {
        if (fullRedraw)
        {
            for (int i = 0; i < menuSize; i++)
            {
                if (i == (int)CONFIG.mode)
                {
                    markEntry(menu[i].name, COL_FOR_GREEN, 0);
                    cursorY = i + 1;
                }
                else
                    unmarkEntry(menu[i].name);
            }

            clearScreen();
            printHeader("View mode");
            printMenu(menu, menuSize, msg, 1, TERM_SIZE.ws_col - 6,
                menuSize, &cursorX, &cursorY, &cursorXPrev, &cursorYPrev);
            printFooter("[jk] Navigate [Enter] Select [q] Back");
        }
        else
        {
            if (COL_ENABLED)
                printf("\x1b[2;1H");
            else
                printf("\x1b[3;1H");
            printMenu(menu, menuSize, msg, 1, TERM_SIZE.ws_col - 6,
                menuSize, &cursorX, &cursorY, &cursorXPrev, &cursorYPrev);
        }

        NavInput input = getNavInput();

        fullRedraw = 1;
        cursorYPrev = 0;
        switch (input)
        {
            case CURSOR_UP:
                cursorYPrev = cursorY;
                cursorY--;
                if (cursorY < 1) cursorY = menuSize;
                fullRedraw = 0;
                break;

            case CURSOR_DOWN:
                cursorYPrev = cursorY;
                cursorY++;
                if (cursorY > menuSize) cursorY = 1;
                fullRedraw = 0;
                break;

            case ENTER:
                CONFIG.mode = cursorY - 1;
                writeConf(CONFIG);
                break;

            case QUIT:
                running = 0;
                break;

            default:
                fullRedraw = 0;
                break;
        }
    }
}
