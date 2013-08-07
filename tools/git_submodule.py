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
            initSubmodules()
            os.chdir(currentDir)
        else:
            print '-- skipping ' + self.path + ' for this operating system. --'

    def listFiles(self):
        if self.matchesOS():
            currentDir = os.getcwd()
            os.chdir(self.path)
            files = subprocess.check_output(['git', 'ls-files']).splitlines()
            os.chdir(currentDir)
            return files
        else:
            print '-- skipping ' + self.path + ' for this operating system. --'
            return []


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
