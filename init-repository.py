#!/usr/bin/env python

#############################################################################
#
# Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
# Contact: http://www.qt-project.org/legal
#
# This file is part of the QtWebEngine module of the Qt Toolkit.
#
# $QT_BEGIN_LICENSE:LGPL$
# Commercial License Usage
# Licensees holding valid commercial Qt licenses may use this file in
# accordance with the commercial license agreement provided with the
# Software or, alternatively, in accordance with the terms contained in
# a written agreement between you and Digia.  For licensing terms and
# conditions see http://qt.digia.com/licensing.  For further information
# use the contact form at http://qt.digia.com/contact-us.
#
# GNU Lesser General Public License Usage
# Alternatively, this file may be used under the terms of the GNU Lesser
# General Public License version 2.1 as published by the Free Software
# Foundation and appearing in the file LICENSE.LGPL included in the
# packaging of this file.  Please review the following information to
# ensure the GNU Lesser General Public License version 2.1 requirements
# will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
#
# In addition, as a special exception, Digia gives you certain additional
# rights.  These rights are described in the Digia Qt LGPL Exception
# version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
#
# GNU General Public License Usage
# Alternatively, this file may be used under the terms of the GNU
# General Public License version 3.0 as published by the Free Software
# Foundation and appearing in the file LICENSE.GPL included in the
# packaging of this file.  Please review the following information to
# ensure the GNU General Public License version 3.0 requirements will be
# met: http://www.gnu.org/copyleft/gpl.html.
#
#
# $QT_END_LICENSE$
#
#############################################################################

import glob
import os
import subprocess
import sys
import string
import argparse

qtwebengine_src = os.path.abspath(os.path.join(os.path.dirname(__file__)))

sys.path.append(os.path.join(qtwebengine_src, 'tools'))
import git_submodule as GitSubmodule

chromium_src = os.environ.get('CHROMIUM_SRC_DIR')
ninja_src = os.path.join(qtwebengine_src, '3rdparty_upstream/ninja')
use_external_chromium = False

parser = argparse.ArgumentParser(description='Initialize QtWebEngine repository.')
group = parser.add_mutually_exclusive_group()
group.add_argument('-u', '--upstream', action='store_true', help='initialize using upstream Chromium submodule (default)')
group.add_argument('-s', '--snapshot', action='store_true', help='initialize using flat Chromium snapshot submodule')
group.add_argument('--android', action='store_true', help='initialize using upstream Chromium and also clone android tools')
args = parser.parse_args()

if chromium_src:
    chromium_src = os.path.abspath(chromium_src)
    use_external_chromium = True
if not chromium_src or not os.path.isdir(chromium_src):
    if args.snapshot:
        chromium_src = os.path.join(qtwebengine_src, '3rdparty/chromium')
        ninja_src = os.path.join(qtwebengine_src, '3rdparty/ninja')
    if args.upstream or not chromium_src:
        chromium_src = os.path.join(qtwebengine_src, '3rdparty_upstream/chromium')
        args.upstream = True
    print 'CHROMIUM_SRC_DIR not set, using Chromium in' + chromium_src

# Write our chromium sources directory into git config.
relative_chromium_src = os.path.relpath(chromium_src, qtwebengine_src)
subprocess.call(['git', 'config', 'qtwebengine.chromiumsrcdir', relative_chromium_src])


def which(tool_name):
    path = os.environ.get('PATH')
    for entry in path.split(os.pathsep):
        entry = os.path.join(entry, tool_name)
        if os.access(entry, os.X_OK):
            return entry
    return ''

def updateLastChange():
    if use_external_chromium:
        return
    currentDir = os.getcwd()
    os.chdir(chromium_src)
    print 'updating LASTCHANGE files'
    subprocess.call(['python', 'build/util/lastchange.py', '-o', 'build/util/LASTCHANGE'])
    subprocess.call(['python', 'build/util/lastchange.py', '-s', 'third_party/WebKit', '-o', 'build/util/LASTCHANGE.blink'])
    os.chdir(currentDir)

def addGerritRemote():
    os.chdir(qtwebengine_src)
    remotes = subprocess.check_output(['git', 'remote'])
    if not 'gerrit' in remotes:
        subprocess.call(['git', 'remote', 'add', 'gerrit', 'ssh://codereview.qt-project.org:29418/qt-labs/qtwebengine.git'])

def installGitHooks():
    os.chdir(qtwebengine_src)
    subprocess.call(['scp', '-p', '-P', '29418', 'codereview.qt-project.org:hooks/commit-msg', '.git/hooks'])

def applyPatches():
    if use_external_chromium:
        return
    os.chdir(qtwebengine_src)
    subprocess.call(['sh', './patches/patch-chromium.sh'])

def initUpstreamSubmodules():
    ninja_url = 'https://github.com/martine/ninja.git'
    chromium_url = 'https://chromium.googlesource.com/chromium/src.git'
    ninja_shasum = '40b51a0b986b8675e15b0cd1b10c272bf51fdb84'
    chromium_shasum = '29d2d710e0e7961dff032ad4ab73887cc33122bb'
    os.chdir(qtwebengine_src)

    current_submodules = subprocess.check_output(['git', 'submodule'])
    if not '3rdparty_upstream/ninja' in current_submodules:
        subprocess.call(['git', 'submodule', 'add', ninja_url, '3rdparty_upstream/ninja'])
    if not use_external_chromium and not '3rdparty_upstream/chromium' in current_submodules:
        subprocess.call(['git', 'submodule', 'add', chromium_url, '3rdparty_upstream/chromium'])

    ninjaSubmodule = GitSubmodule.Submodule()
    ninjaSubmodule.path = '3rdparty_upstream/ninja'
    ninjaSubmodule.shasum = ninja_shasum
    ninjaSubmodule.url = ninja_url
    ninjaSubmodule.os = 'all'
    ninjaSubmodule.initialize()

    if not use_external_chromium:
        chromiumSubmodule = GitSubmodule.Submodule()
        chromiumSubmodule.path = '3rdparty_upstream/chromium'
        chromiumSubmodule.shasum = chromium_shasum
        chromiumSubmodule.url = chromium_url
        chromiumSubmodule.os = 'all'
        if args.android:
            GitSubmodule.extra_os = ['android']
        chromiumSubmodule.initialize()

def initSnapshot():
    snapshot = GitSubmodule.Submodule()
    snapshot.path = '3rdparty'
    snapshot.os = 'all'
    snapshot.initialize()

os.chdir(qtwebengine_src)
addGerritRemote()
installGitHooks()

print 'Configuring git to ignore all submodules. Submodule changes will not show up in "git diff"!'
subprocess.call(['git', 'config', 'diff.ignoreSubmodules', 'all'])
subprocess.call(['git', 'update-index', '--assume-unchanged', '.gitmodules'])

if args.upstream:
    initUpstreamSubmodules()
    updateLastChange()
    applyPatches()
if args.snapshot:
    initSnapshot()
