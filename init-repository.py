#!/usr/bin/env python

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
        if (sys.platform.startswith('linux') or sys.platform.startswith('darwin')) and 'unix' in self.os:
            return True
        return False

    def findSha(self):
        line = subprocess.check_output(['git', 'submodule', 'status', self.path]);
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
            #subprocess.call(['git', 'fetch'])
            #subprocess.call(['git', 'checkout', self.shasum, '-q']) # -q is to silence detached head warnings.
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

os.chdir(qtwebengine_src)
initSubmodules()
#initSubmodules('HEAD')
updateLastChange()
buildNinja()

