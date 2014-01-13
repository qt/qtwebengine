#!/bin/sh

if [ $# -ne 1 ]; then
    echo "Usage: $0 release-name"
    echo "       example: $0 qtwebengine-opensource-src-0.1.0-tp1"
    exit 0
fi

RELEASE_NAME=$1
OUTDIR=`pwd`

git archive origin/master --format tar --prefix=$RELEASE_NAME/ -o $OUTDIR/$RELEASE_NAME.tar
cd src/3rdparty
git archive origin/master --format tar --prefix=$RELEASE_NAME/src/3rdparty/ -o $OUTDIR/$RELEASE_NAME.src.3rdparty.tar

tar --concatenate --file=$OUTDIR/$RELEASE_NAME.tar $OUTDIR/$RELEASE_NAME.src.3rdparty.tar
rm $OUTDIR/$RELEASE_NAME.src.3rdparty.tar

gzip $OUTDIR/$RELEASE_NAME.tar

