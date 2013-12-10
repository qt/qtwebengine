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
import version_resolver as resolver

extra_os = []

def subprocessCall(args):
    print args
    return subprocess.call(args)

def subprocessCheckOutput(args):
    print args
    return subprocess.check_output(args)

class DEPSParser:
    def __init__(self):
        self.global_scope = {
          'Var': self.Lookup,
          'deps_os': {},
        }
        self.local_scope = {}

    def Lookup(self, var_name):
        return self.local_scope["vars"][var_name]

    def createSubmodulesFromScope(self, scope, os):
        submodules = []
        for dep in scope:
            if (type(scope[dep]) == str):
                repo_rev = scope[dep].split('@')
                repo = repo_rev[0]
                rev = repo_rev[1]
                subdir = dep
                if subdir.startswith('src/'):
                    subdir = subdir[4:]
                if subdir.startswith('src'):
                    # Ignore the information about chromium since we get that from git.
                    continue
                shasum = ''
                if len(rev) == 40: # Length of a git shasum
                    shasum = rev
                submodule = Submodule(subdir, repo)
                submodule.os = os
                submodule.shasum = shasum
                if not submodule.shasum:
                    # We need to parse the svn branch and revision number.
                    ref = repo
                    # Try to find out the branch.
                    ref_path = repo.split('branches/')
                    if len(ref_path) > 1:
                        ref = ref_path[1]
                    if 'trunk' in ref or 'svn' in ref:
                        # Trunk is used, so we can use the remote master.
                        ref = 'master'
                    name_path = subdir.split('/')
                    if len(name_path) > 1:
                        # At this point some svn repository paths still include the repo name
                        # after the actual branch so we have to strip it together with the leading /
                        name = name_path[-1]
                        if ref.endswith(name):
                            branch = ref[:-(len(name) + 1)]
                            ref = 'refs/branch-heads/' + branch
                        if name in ref:
                            # At this point the last partition in the path
                            # is the branch name, so compose the git ref.
                            branch = ref.split('/')[-1]
                            ref = 'refs/branch-heads/' + branch
                    if 'master' not in ref and 'refs/branch-heads' not in ref:
                        # Finally compose the refs that did not mach previous rules.
                        ref = 'refs/branch-heads/' + ref
                    submodule.ref = ref
                    submodule.revision = int(rev)
                submodules.append(submodule)
        return submodules

    def sanityCheckModules(self, submodules):
        submodule_dict = {}
        for submodule in submodules:
            if submodule.path in submodule_dict:
                prev_module = submodule_dict[submodule.path]
                # We might have to create our own DEPS file if different platforms use different branches,
                # but for now it should be safe to select the latest revision from the requirements.
                if submodule.shasum or prev_module.revision >= submodule.revision:
                    continue
                if prev_module.ref != submodule.ref:
                    sys.exit('ERROR: branch mismatch for ' + submodule.path + '(' + prev_module.ref + ' vs ' + submodule.ref + ')')
                print('Duplicate submodule ' + submodule.path + '. Using latest revison ' + str(submodule.revision) + '.')
            submodule_dict[submodule.path] = submodule
        return list(submodule_dict.values())

    def parse(self, deps_content):
        exec(deps_content, self.global_scope, self.local_scope)

        submodules = []
        submodules.extend(self.createSubmodulesFromScope(self.local_scope['deps'], 'all'))
        for os_dep in self.local_scope['deps_os']:
            submodules.extend(self.createSubmodulesFromScope(self.local_scope['deps_os'][os_dep], os_dep))

        return self.sanityCheckModules(submodules)

    def parseFile(self, deps_file_name):
        currentDir = os.getcwd()
        if not os.path.isfile(deps_file_name):
            return []
        deps_file = open(deps_file_name)
        deps_content = deps_file.read().decode('utf-8')
        deps_file.close()
        return self.parse(deps_content)



class Submodule:
    def __init__(self, path='', url='', shasum='', os=[], ref=''):
        self.path = path
        self.url = url
        self.shasum = shasum
        self.os = os
        self.ref = ''
        self.revision = None

    def matchesOS(self):
        if not self.os:
            return True
        if 'all' in self.os:
            return True
        if (sys.platform.startswith('win32') or sys.platform.startswith('cygwin')) and 'win' in self.os:
            return True
        if sys.platform.startswith('linux') and 'unix' in self.os:
            return True
        if sys.platform.startswith('darwin') and ('unix' in self.os or 'mac' in self.os):
            return True
        for os in extra_os:
            if os in self.os:
                return True
        return False

    def findShaAndCheckout(self):
        oldCwd = os.getcwd()
        os.chdir(self.path)
        error = 0
        if self.ref:
            # Fetch the ref we parsed from the DEPS file.
            val = subprocessCall(['git', 'fetch', 'origin', self.ref])
            if val != 0:
                sys.exit("Could not fetch branch from upstream " + self.ref)
            error = subprocessCall(['git', 'checkout', 'FETCH_HEAD']);

            search_string = ''
            if self.path.endswith('/chromium'):
                search_string = resolver.currentVersion()
            elif self.revision:
                search_string = '@' + str(self.revision) + ' '
            if search_string:
                line = subprocessCheckOutput(['git', 'log', '-n1', '--pretty=oneline', '--grep=' + search_string])
                if line:
                    self.shasum = line.split(' ')[0]
            else: # No revision set, use the submodule shasum
                os.chdir(oldCwd)
                line = subprocessCheckOutput(['git', 'submodule', 'status', self.path])
                os.chdir(self.path)
                line = line.lstrip(' -')
                self.shasum = line.split(' ')[0]

        current_shasum = subprocessCheckOutput(['git', 'show', '-s', '--oneline']).split(' ')[0]
        if not self.shasum.startswith(current_shasum):
            # In case HEAD differs check out the actual shasum we require.
            error = subprocessCall(['git', 'checkout', self.shasum])
        os.chdir(oldCwd)
        return error

    def findGitDir(self):
        try:
            return subprocessCheckOutput(['git', 'rev-parse', '--git-dir']).strip()
        except subprocess.CalledProcessError, e:
            sys.exit("git dir could not be determined! - Initialization failed! " + e.output)

    def reset(self):
        currentDir = os.getcwd()
        os.chdir(self.path)
        gitdir = self.findGitDir()
        if os.path.isdir(os.path.join(gitdir, 'rebase-merge')):
            if os.path.isfile(os.path.join(gitdir, 'MERGE_HEAD')):
                print 'merge in progress... aborting merge.'
                subprocessCall(['git', 'merge', '--abort'])
            else:
                print 'rebase in progress... aborting merge.'
                subprocessCall(['git', 'rebase', '--abort'])
        if os.path.isdir(os.path.join(gitdir, 'rebase-apply')):
            print 'am in progress... aborting am.'
            subprocessCall(['git', 'am', '--abort'])
        subprocessCall(['git', 'reset', '--hard'])
        os.chdir(currentDir)

    def initialize(self):
        if self.matchesOS():
            print '-- initializing ' + self.path + ' --'
            if os.path.isdir(self.path):
                self.reset()

            if self.url:
                subprocessCall(['git', 'submodule', 'add', '-f', self.url, self.path])
            subprocessCall(['git', 'submodule', 'init', self.path])
            subprocessCall(['git', 'submodule', 'update', self.path])

            if self.findShaAndCheckout() != 0:
                sys.exit("!!! initialization failed !!!")
        else:
            print '-- skipping ' + self.path + ' for this operating system. --'

    def listFiles(self):
        if self.matchesOS() and os.path.isdir(self.path):
            currentDir = os.getcwd()
            os.chdir(self.path)
            files = subprocessCheckOutput(['git', 'ls-files']).splitlines()
            os.chdir(currentDir)
            return files
        else:
            print '-- skipping ' + self.path + ' for this operating system. --'
            return []


    def readSubmodules(self):
        submodules = []
        if self.ref:
            submodules = resolver.readSubmodules()
            print 'DEPS file provides the following submodules:'
            for submodule in submodules:
                submodule_ref = submodule.shasum
                if submodule.revision:
                    submodule_ref = submodule.ref + '@' + str(submodule.revision)
                print '{:<80}'.format(submodule.path) + '{:<120}'.format(submodule.url) + submodule_ref
        else: # Try .gitmodules since no ref has been specified
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
        oldCwd = os.getcwd()
        os.chdir(self.path)
        submodules = self.readSubmodules()
        for submodule in submodules:
            submodule.initialize()
        if self.ref:
            subprocessCall(['git', 'commit', '-a', '-m', 'initialize submodules'])
        os.chdir(oldCwd)
