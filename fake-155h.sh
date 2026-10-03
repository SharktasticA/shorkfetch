#!/bin/bash

######################################################
## Creates a fakesys directory containing Intel     ##
## Core Ultra 7 155H sysfs data that SHORKFETCH can ##
## test with. Data provided by adyolo.              ##
######################################################
## Licence: GNU GENERAL PUBLIC LICENSE Version 3    ##
######################################################
## Kali (links.sharktastica.co.uk)                  ##
######################################################

FAKESYS="$(pwd)/fakesys"
rm -rf "$FAKESYS"

BASE_FREQS="1400000 1400000 1400000 1400000 1400000 1400000 1400000 1400000 1400000 1400000 1400000 1400000 900000 900000 900000 900000 900000 900000 900000 900000 700000 700000"
i=0
for V in $BASE_FREQS; do
    D="$FAKESYS/devices/system/cpu/cpufreq/policy$i"
    mkdir -p "$D"
    echo "$V" > "$D/base_frequency"
    i=$((i+1))
done

MAX_FREQS="4500000 4800000 4800000 4800000 4800000 4500000 4500000 4500000 4500000 4500000 4500000 4500000 3800000 3800000 3800000 3800000 3800000 3800000 3800000 3800000 2500000 2500000"
i=0
for V in $MAX_FREQS; do
    D="$FAKESYS/devices/system/cpu/cpu$i/cpufreq"
    mkdir -p "$D"
    echo "$V" > "$D/cpuinfo_max_freq"
    i=$((i+1))
done

SIBLINGS="0,5 1-2 1-2 3-4 3-4 0,5 6-7 6-7 8-9 8-9 10-11 10-11 12 13 14 15 16 17 18 19 20 21"
i=0
for V in $SIBLINGS; do
    D="$FAKESYS/devices/system/cpu/cpu$i/topology"
    mkdir -p "$D"
    echo "$V" > "$D/thread_siblings_list"
    i=$((i+1))
done

mkdir -p "$FAKESYS/devices/cpu_core" "$FAKESYS/devices/cpu_atom"
echo "0-11"  > "$FAKESYS/devices/cpu_core/cpus"
echo "12-21" > "$FAKESYS/devices/cpu_atom/cpus"
