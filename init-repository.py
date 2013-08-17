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

qtwebengine_src = os.path.abspath(os.path.join(os.path.dirname(__file__)))

sys.path.append(os.path.join(qtwebengine_src, 'tools'))
import git_submodule as GitSubmodule

chromium_src = os.environ.get('CHROMIUM_SRC_DIR')
if chromium_src:
  chromium_src = os.path.abspath(chromium_src)
if not chromium_src or not os.path.isdir(chromium_src):
  chromium_src = os.path.join(qtwebengine_src, '3rdparty_upstream/chromium')
  print 'CHROMIUM_SRC_DIR not set, falling back to ' + chromium_src

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
    currentDir = os.getcwd()
    os.chdir(chromium_src)
    print 'updating LASTCHANGE files'
    subprocess.call(['python', 'build/util/lastchange.py', '-o', 'build/util/LASTCHANGE'])
    subprocess.call(['python', 'build/util/lastchange.py', '-s', 'third_party/WebKit', '-o', 'build/util/LASTCHANGE.blink'])
    os.chdir(currentDir)

def buildNinja():
    ninja_tool = which('ninja')
    if ninja_tool:
        print 'found ninja in: ' + ninja_tool + ' ...not building new ninja.'
        return
    currentDir = os.getcwd()
    ninja_src = os.path.join(qtwebengine_src, '3rdparty_upstream/ninja')
    os.chdir(ninja_src)
    print 'building ninja...'
    subprocess.call(['python', 'bootstrap.py'])
    os.chdir(currentDir)

def addGerritRemote():
    os.chdir(qtwebengine_src)
    remotes = subprocess.check_output(['git', 'remote'])
    if not 'gerrit' in remotes:
        subprocess.call(['git', 'remote', 'add', 'gerrit', 'ssh://codereview.qt-project.org:29418/qt-labs/qtwebengine.git'])

def installGitHooks():
    os.chdir(qtwebengine_src)
    subprocess.call(['scp', '-p', 'codereview.qt-project.org:hooks/commit-msg', '.git/hooks'])

def applyPatches():
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
        gitmodules_file = open('.gitmodules', 'a')
        gitmodules_file.writelines(['       ignore = all\n'])
        gitmodules_file.close()
    if not '3rdparty_upstream/chromium' in current_submodules:
        subprocess.call(['git', 'submodule', 'add', chromium_url, '3rdparty_upstream/chromium'])
        gitmodules_file = open('.gitmodules', 'a')
        gitmodules_file.writelines(['       ignore = all\n'])
        gitmodules_file.close()

    gitignore_file = open('.gitignore')
    gitignore_content = gitignore_file.readlines()
    gitignore_file.close()

    gitmodules_is_ignored = False
    for gitignore_line in gitignore_content:
        if '.gitmodules' in gitignore_line:
            gitmodules_is_ignored = True
    if not gitmodules_is_ignored:
        gitignore_file = open('.gitignore', 'a')
        gitignore_file.writelines(['.gitmodules\n'])
        gitignore_file.close()

    ninjaSubmodule = GitSubmodule.Submodule()
    ninjaSubmodule.path = '3rdparty_upstream/ninja'
    ninjaSubmodule.shasum = ninja_shasum
    ninjaSubmodule.url = ninja_url
    ninjaSubmodule.os = 'all'
    ninjaSubmodule.initialize()

    chromiumSubmodule = GitSubmodule.Submodule()
    chromiumSubmodule.path = '3rdparty_upstream/chromium'
    chromiumSubmodule.shasum = chromium_shasum
    chromiumSubmodule.url = chromium_url
    chromiumSubmodule.os = 'all'
    chromiumSubmodule.initialize()


os.chdir(qtwebengine_src)
addGerritRemote()
installGitHooks()
initUpstreamSubmodules()
updateLastChange()
applyPatches()
#buildNinja()

