#!/bin/sh

set -e

SHORKCOMMON_URL="https://github.com/SharktasticA/shorkcommon.git"
SHORKCOMMON_COMMIT="22078d5e08a7e8a7b1fee5bee334918d703d7e6f"
CPUINFO_DS_URL="https://github.com/SharktasticA/cpuinfo-ds.git"
CPUINFO_DS_COMMIT="c45125f71ae38ab04af3e2338b591645f2d2b1d4"

sync_dir()
{
    DIR="$1"
    URL="$2"
    COMMIT="$3"

    if [ "$(git -C "$DIR" rev-parse HEAD 2>/dev/null)" = "$COMMIT" ]; then
        return
    fi

    rm -rf "$DIR"
    git clone -q "$URL" "$DIR"
    git -C "$DIR" checkout -q "$COMMIT"
}

if [ -e .git ]; then
    git submodule update --init shorkcommon cpuinfo-ds
else
    sync_dir shorkcommon "$SHORKCOMMON_URL" "$SHORKCOMMON_COMMIT"
    sync_dir cpuinfo-ds "$CPUINFO_DS_URL" "$CPUINFO_DS_COMMIT"
fi
