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
        if self.shasum != '':
            return
        line = subprocess.check_output(['git', 'submodule', 'status', self.path])
        line = line.lstrip(' -')
        self.shasum = line.split(' ')[0]

    def findGitDir(self):
        try:
            return subprocess.check_output(['git', 'rev-parse', '--git-dir']).strip()
        except subprocess.CalledProcessError, e:
            sys.exit("git dir could not be determined! - Initialization failed!" + e.output)

    def reset(self):
        currentDir = os.getcwd()
        os.chdir(self.path)
        gitdir = self.findGitDir()
        if os.path.isdir(os.path.join(gitdir, 'rebase-merge')):
            if os.path.isfile(os.path.join(gitdir, 'MERGE_HEAD')):
                print 'merge in progress... aborting merge.'
                subprocess.call(['git', 'merge', '--abort'])
            else:
                print 'rebase in progress... aborting merge.'
                subprocess.call(['git', 'rebase', '--abort'])
        if os.path.isdir(os.path.join(gitdir, 'rebase-apply')):
            print 'am in progress... aborting am.'
            subprocess.call(['git', 'am', '--abort'])
        subprocess.call(['git', 'reset', '--hard'])
        os.chdir(currentDir)

    def initialize(self):
        if self.matchesOS():
            self.reset()
            print '-- initializing ' + self.path + ' --'
            subprocess.call(['git', 'submodule', 'init', self.path])
            subprocess.call(['git', 'submodule', 'update', self.path])
            self.findSha()
            currentDir = os.getcwd()
            os.chdir(self.path)
            # Make sure we have checked out the right shasum.
            # In case there were other patches applied before.
            val = subprocess.call(['git', 'checkout', self.shasum])
            if val != 0:
                sys.exit("!!! initialization failed !!!")
            self.initSubmodules()
            os.chdir(currentDir)
        else:
            print '-- skipping ' + self.path + ' for this operating system. --'


    def readSubmodules(self):
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

    def initSubmodules(self):
        submodules = self.readSubmodules()
        for submodule in submodules:
            submodule.initialize()


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

    ninjaSubmodule = Submodule()
    ninjaSubmodule.path = '3rdparty_upstream/ninja'
    ninjaSubmodule.shasum = ninja_shasum
    ninjaSubmodule.url = ninja_url
    ninjaSubmodule.os = 'all'
    ninjaSubmodule.initialize()

    chromiumSubmodule = Submodule()
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

