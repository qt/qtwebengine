#!/bin/sh
# Script used temporarily to invoke gclient and apply our patches

if [ -z $CHROMIUM_SRC_DIR -o ! -d $CHROMIUM_SRC_DIR ]; then
    echo "CHROMIUM_SRC_DIR not set or pointing to a non existing directory."
    exit 1;
fi

PATCH_DIR="$( cd "$( dirname "$0" )" && pwd )"

cd $CHROMIUM_SRC_DIR
echo "Entering $PWD"

GCLIENT=`which gclient 2>/dev/null`
if [ -z $GCLIENT ]; then
    # Try to find it in the most likely location
    GCLIENT=$CHROMIUM_SRC_DIR/../depot_tools/gclient
    if [ ! -e $GCLIENT ]; then
        echo "Can't find gclient"
        exit 2;
    fi
fi

$GCLIENT revert

if [ "$2" = "--update" ]; then
    $GCLIENT fetch
    $GCLIENT sync
fi

echo "Applying patches..."
git am $PATCH_DIR/0001-My-local-fixes.patch $PATCH_DIR/0002-Add-WebEngineContext-to-RunLoop-s-friends.patch

cd tools/gyp
echo "Entering $PWD"

git am $PATCH_DIR/0001-GYP-Fix-build-with-toplevel-dir.patch
