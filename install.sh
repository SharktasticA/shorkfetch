#!/bin/bash

######################################################
##            SHORK UTILITY - SHORKFETCH            ##
######################################################
## shorkfetch install script                        ##
######################################################
## Licence: GNU GENERAL PUBLIC LICENSE Version 3    ##
######################################################
## Kali (links.sharktastica.co.uk)                  ##
######################################################



set -e



if ! gcc --version >/dev/null 2>&1; then
    echo "ERROR: GCC is required for compiling shorkfetch"
    exit 1
fi

if ! make --version >/dev/null 2>&1; then
    echo "ERROR: make is required for compiling shorkfetch"
    exit 1
fi



if git --version >/dev/null 2>&1; then
    git clone https://github.com/SharktasticA/shorkfetch
    cd shorkfetch
    sudo make install

    cd ..
    rm -rf shorkfetch
else
    wget https://github.com/SharktasticA/shorkfetch/archive/refs/heads/main.zip
    unzip main.zip
    cd shorkfetch-main
    sudo make install

    cd ..
    rm -rf shorkfetch-main main.zip
fi

shorkfetch
echo "All done! :)"
