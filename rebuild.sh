#!/usr/bin/env bash
set -e

if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <target> <sdkconfig_file>"
    echo "Example: $0 esp32 sdkconfig.debug"
    exit 1
fi

TARGET="$1"
SDKCONFIG_FILE="$2"

if [ ! -f "$SDKCONFIG_FILE" ]; then
    echo "Error: sdkconfig file '$SDKCONFIG_FILE' not found."
    exit 1
fi

#idf.py fullclean
[ -f sdkconfig ] && rm sdkconfig
[ -d build ] && rm -r build
[ -d .config ] && rm -r .config
[ -f sdkconfig.old ] && rm sdkconfig.old


idf.py -D SDKCONFIG_DEFAULTS="$SDKCONFIG_FILE" set-target "$TARGET"
#idf.py build > /dev/null
