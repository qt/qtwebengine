#!/bin/bash
# Copyright (C) 2016 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

if [ $# -ne 3 ]; then
    echo "Usage: $0 git-ref release-name version"
    echo "       example: $0 origin/master qtwebengine-opensource-src 0.1.0-tp1"
    exit 0
fi

hash syncqt.pl &>/dev/null
if [ $? -eq 1 ]; then
    echo "Cannot find syncqt.pl in PATH."
    exit 1
fi

set -e
QTWEBENGINE_REF=$1
RELEASE_NAME=$2
VERSION=$3
OUTDIR=`pwd`

RELEASE_NAME=$RELEASE_NAME-$VERSION

THIRD_PARTY_REF=`git log -p $QTWEBENGINE_REF --ignore-submodules=none -n 1 -- src/3rdparty | grep "+Subproject commit" | cut -f 3 -d ' '`

git archive $QTWEBENGINE_REF --format tar --prefix=$RELEASE_NAME/ -o $OUTDIR/$RELEASE_NAME.tar
cd src/3rdparty
git archive $THIRD_PARTY_REF --format tar --prefix=$RELEASE_NAME/src/3rdparty/ -o $OUTDIR/$RELEASE_NAME.src.3rdparty.tar

tar --concatenate --file=$OUTDIR/$RELEASE_NAME.tar $OUTDIR/$RELEASE_NAME.src.3rdparty.tar
rm $OUTDIR/$RELEASE_NAME.src.3rdparty.tar

mkdir $RELEASE_NAME
trap "{ rm -rf $RELEASE_NAME ; exit 255}" EXIT
echo `git rev-parse $QTWEBENGINE_REF` > $RELEASE_NAME/.tag
tar -r --file=$OUTDIR/$RELEASE_NAME.tar $RELEASE_NAME/.tag
trap - EXIT
rm -r $RELEASE_NAME

TMP_SYNCQT_PACKAGE_DIR=/tmp/qtwebengine-syncqt-fix
mkdir $TMP_SYNCQT_PACKAGE_DIR
trap "{ rm -rf $TMP_SYNCQT_PACKAGE_DIR ; exit 255; } " EXIT
(cd $TMP_SYNCQT_PACKAGE_DIR \
    && tar xf $OUTDIR/$RELEASE_NAME.tar \
    && (cd $RELEASE_NAME && syncqt.pl -version `echo $VERSION | cut -d- -f1`) \
    && tar cf syncqt-output.tar $RELEASE_NAME/include
)
tar --concatenate --file=$OUTDIR/$RELEASE_NAME.tar $TMP_SYNCQT_PACKAGE_DIR/syncqt-output.tar
trap - EXIT
rm -rf $TMP_SYNCQT_PACKAGE_DIR

gzip $OUTDIR/$RELEASE_NAME.tar

