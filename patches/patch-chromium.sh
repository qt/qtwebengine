#!/bin/sh
# Script used temporarily to invoke gclient and apply our patches

if [ -z $CHROMIUM_SRC_DIR -o ! -d $CHROMIUM_SRC_DIR ]; then
    echo "CHROMIUM_SRC_DIR not set or pointing to a non existing directory."
    exit 1;
fi

PATCH_DIR=$PWD
DEPOT_TOOLS=$CHROMIUM_SRC_DIR/../depot_tools

cd $CHROMIUM_SRC_DIR
echo "Entering $PWD"

$DEPOT_TOOLS/gclient revert

if [ "$2" = "--update" ]; then
    $DEPOT_TOOLS/gclient fetch
    $DEPOT_TOOLS/gclient sync
fi

echo "Applying patches..."
git am $PATCH_DIR/0001-My-local-fixes.patch

cd tools/gyp
echo "Entering $PWD"

git am $PATCH_DIR/0001-GYP-Fix-build-with-toplevel-dir.patch
