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

THIRDPARTY_UPSTREAM_CHROMIUM_DIR="$( cd "$( dirname "$0" )"/../src/3rdparty_upstream/chromium && pwd )"
SCRIPTS_DIR="$( cd "$( dirname "$0" )"/../tools/scripts && pwd )"
PATCH_DIR="$( cd "$( dirname "$0" )"/chromium && pwd )"

for MODULE in \
    / \
    /third_party/WebKit \
    /third_party/libjingle/source/talk \
    /third_party/libvpx \
    /tools/gyp \
    /tools/grit \
    /v8
do
    cd $THIRDPARTY_UPSTREAM_CHROMIUM_DIR$MODULE
    echo "Entering $PWD"
    git rev-parse first-parent > /dev/null
    if [ $? -ne 0 ]; then
        echo "ERROR: Can't find the 'first-parent' git tag pointing where to start, you might have to re-run init-repository.py and re-apply your modifications."
        exit 1
    fi

    rm $PATCH_DIR$MODULE/0*
    git format-patch --no-signature -k -o $PATCH_DIR$MODULE first-parent
    $SCRIPTS_DIR/clean_gitformatpatch_patchid.py $PATCH_DIR$MODULE/0*
    cd $PATCH_DIR
    git add --all $PATCH_DIR$MODULE
done

