#!/bin/bash

######################################################
##            SHORK UTILITY - SHORKFETCH            ##
######################################################
## SHORKFETCH quick download & install script       ##
######################################################
## Licence: GNU GENERAL PUBLIC LICENSE Version 3    ##
######################################################
## Kali (links.sharktastica.co.uk)                  ##
######################################################



set -e



START_DIR="$(pwd)"
BLUE='\033[0;34m'
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
RESET='\033[0m'



if ! gcc --version >/dev/null 2>&1; then
    echo -e "${RED}ERROR: GCC is required for compiling SHORKFETCH${RESET}"
    exit 1
fi
if ! git --version >/dev/null 2>&1; then
    echo -e "${RED}ERROR: Git is required for compiling SHORKFETCH${RESET}"
    exit 1
fi

if ! make --version >/dev/null 2>&1; then
    echo -e "${RED}ERROR: make is required for compiling SHORKFETCH${RESET}"
    exit 1
fi



cleanup()
{
    echo -e "${YELLOW}Cleaning up...${RESET}"
    cd "$START_DIR"
    rm -rf shorkfetch 2>/dev/null || true;
}
trap cleanup EXIT



if git --version >/dev/null 2>&1; then
    echo -e "${YELLOW}Cloning SHORKFETCH repo...${RESET}"
    git clone https://github.com/SharktasticA/shorkfetch
    cd shorkfetch

    echo -e "${YELLOW}Installing SHORKFETCH (you may be asked for sudo)...${RESET}"
    sudo make install
fi

shorkfetch
echo -e "${GREEN}Installed! :) Run: shorkfetch${RESET}"
