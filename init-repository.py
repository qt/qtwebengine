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

chrome_src = os.environ.get('CHROMIUM_SRC_DIR')
if chrome_src:
  chrome_src = os.path.abspath(chrome_src)
if not chrome_src or not os.path.isdir(chrome_src):
  chrome_src = os.path.join(qtwebengine_src, 'chromium')
  print 'CHROMIUM_SRC_DIR not set, falling back to ' + chrome_src

def which(tool_name):
    path = os.environ.get('PATH')
    for entry in path.split(os.pathsep):
        entry = os.path.join(entry, tool_name)
        if os.access(entry, os.X_OK):
            return entry
    return ''


class Submodule:
    def __init__(self):
        self.path = ''
        self.url = ''
        self.shasum = ''
        self.os = []

    def matchesOS(self):
        if not self.os:
            return True
        if 'all' in self.os:
            return True
        if sys.platform.startswith('win32') and 'win' in self.os:
            return True
        if sys.platform.startswith('linux') and 'unix' in self.os:
            return True
        if sys.platform.startswith('darwin') and ('unix' in self.os or 'mac' in self.os):
            return True
        return False

    def findSha(self):
        line = subprocess.check_output(['git', 'submodule', 'status', self.path])
        line = line.lstrip(' -')
        self.shasum = line.split(' ')[0]

    def initialize(self):
        if self.matchesOS():
            print '-- initializing ' + self.path + ' --'
            subprocess.call(['git', 'submodule', 'init', self.path])
            subprocess.call(['git', 'submodule', 'update', self.path])
            self.findSha()
            currentDir = os.getcwd()
            os.chdir(self.path)
            # Make sure we have checked out the right shasum.
            # In case there were other patches applied before.
            subprocess.call(['git', 'checkout', self.shasum])
            initSubmodules()
            os.chdir(currentDir)
        else:
            print '-- skipping ' + self.path + ' for this operating system. --'


def readSubmodules():
    currentDir = os.getcwd()
    if not os.path.isfile('.gitmodules'):
        return []
    gitmodules_file = open('.gitmodules')
    gitmodules_lines = gitmodules_file.readlines()
    gitmodules_file.close()

    submodules = []
    currentSubmodule = None
    for line in gitmodules_lines:
        if line.find('[submodule') == 0:
            if currentSubmodule:
                submodules.append(currentSubmodule)
            currentSubmodule = Submodule()
        tokens = line.split('=')
        if len(tokens) >= 2:
            key = tokens[0].strip()
            value = tokens[1].strip()
            if key == 'path':
                currentSubmodule.path = value
            elif key == 'url':
                currentSubmodule.url = value
            elif key == 'os':
                currentSubmodule.os = value.split(',')
    if currentSubmodule:
        submodules.append(currentSubmodule)
    return submodules

def initSubmodules():
    submodules = readSubmodules()
    for submodule in submodules:
        submodule.initialize()


def updateLastChange():
    currentDir = os.getcwd()
    os.chdir(chrome_src)
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
    ninja_src = os.path.join(qtwebengine_src, 'build/ninja')
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

os.chdir(qtwebengine_src)
addGerritRemote()
installGitHooks()
initSubmodules()
applyPatches()
updateLastChange()
buildNinja()

