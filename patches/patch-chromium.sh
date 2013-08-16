#!/bin/sh
#############################################################################
##
## Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
## Contact: http://www.qt-project.org/legal
##
## This file is part of the QtWebEngine of the Qt Toolkit.
##
## $QT_BEGIN_LICENSE:LGPL$
## Commercial License Usage
## Licensees holding valid commercial Qt licenses may use this file in
## accordance with the commercial license agreement provided with the
## Software or, alternatively, in accordance with the terms contained in
## a written agreement between you and Digia.  For licensing terms and
## conditions see http://qt.digia.com/licensing.  For further information
## use the contact form at http://qt.digia.com/contact-us.
##
## GNU Lesser General Public License Usage
## Alternatively, this file may be used under the terms of the GNU Lesser
## General Public License version 2.1 as published by the Free Software
## Foundation and appearing in the file LICENSE.LGPL included in the
## packaging of this file.  Please review the following information to
## ensure the GNU Lesser General Public License version 2.1 requirements
## will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
##
## In addition, as a special exception, Digia gives you certain additional
## rights.  These rights are described in the Digia Qt LGPL Exception
## version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
##
## GNU General Public License Usage
## Alternatively, this file may be used under the terms of the GNU
## General Public License version 3.0 as published by the Free Software
## Foundation and appearing in the file LICENSE.GPL included in the
## packaging of this file.  Please review the following information to
## ensure the GNU General Public License version 3.0 requirements will be
## met: http://www.gnu.org/copyleft/gpl.html.
##
##
## $QT_END_LICENSE$
##
#############################################################################

# Script used temporarily to invoke gclient and apply our patches

PATCH_DIR="$( cd "$( dirname "$0" )" && pwd )"
QTWEBENGINE_SRC_DIR="$( cd $PATCH_DIR/../ && pwd )"

if [ -z "$CHROMIUM_SRC_DIR" ]; then
    CHROMIUM_SRC_DIR="$( cd $PATCH_DIR/../3rdparty_upstream/chromium && pwd )"
fi

if [ ! -d "$CHROMIUM_SRC_DIR" ]; then
    echo "CHROMIUM_SRC_DIR pointing to a non existing directory. $CHROMIUM_SRC_DIR"
    exit 1;
fi

cd $CHROMIUM_SRC_DIR
echo "Entering $PWD"

git am $PATCH_DIR/0001-Export-ContentMainRunner.patch
git am $PATCH_DIR/0002-Add-WebEngineContext-to-RunLoop-s-friends.patch
git am $PATCH_DIR/0001-Mac-Use-libc-instead-of-stdlibc.patch
git am $PATCH_DIR/0002-Clang-libc-does-not-support-incomplete-types-in-temp.patch
git am $PATCH_DIR/0001-Mac-Do-not-modify-the-child-path.patch
git am $PATCH_DIR/0001-Do-not-warn-for-header-hygiene.patch
git am $PATCH_DIR/0001-Build-files-necessary-for-touch-and-gestures.patch
git am $PATCH_DIR/0001-remove-Wno-deprecated-register-from-common.gypi.patch

cd $CHROMIUM_SRC_DIR/third_party/WebKit
echo "Entering $PWD"

git am $PATCH_DIR/0001-Remove-leftovers-from-WebKitSystemInterface.patch
git am $PATCH_DIR/0001-Do-not-include-Assertions.h-within-namespace-WebKit.patch

cd $CHROMIUM_SRC_DIR/tools/gyp
echo "Entering $PWD"

git am $PATCH_DIR/0001-GYP-Fix-build-with-toplevel-dir.patch
git am $PATCH_DIR/0001-Add-support-for-libc-to-xcode_emulation.py.patch

cd $CHROMIUM_SRC_DIR/tools/grit
git am $PATCH_DIR/0001-GRIT-Allow-grd-files-outside-of-chromium-source-dir.patch

