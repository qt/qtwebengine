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
import imp
import errno

import git_submodule as GitSubmodule

def isBlacklisted(file_path):
    # Filter out empty submodule directories.
    if (os.path.isdir(file_path)):
        return True
    # We do need all the gyp files.
    if file_path.endswith('.gyp') or file_path.endswith('.gypi'):
        return False
    if '/tests/' in file_path:
        return True
    if ('/test/' in file_path and
        not '/webrtc/test/testsupport/' in file_path and
        not file_path.startswith('net/test/') and
        not file_path.endswith('mock_chrome_application_mac.h')):
        return True
    if file_path.startswith('third_party/WebKit/LayoutTests'):
        return True
    if file_path.startswith('third_party/WebKit/PerformanceTests'):
        return True
    if file_path.startswith('third_party/WebKit/ManualTests'):
        return True
    if file_path.startswith('android_webview'):
        return True
    if file_path.startswith('apps/'):
        return True
    if file_path.startswith('build/android/'):
        return True
    if (file_path.startswith('chrome/') and
        not 'repack_locales' in file_path and
        not file_path.endswith('version.py')):
        return True
    if file_path.startswith('chrome_frame'):
        return True
    if file_path.startswith('chromeos'):
        return True
    if file_path.startswith('breakpad'):
        return True
    if file_path.startswith('third_party/GTM'):
        return True
    if file_path.startswith('third_party/chromite'):
        return True
    if file_path.startswith('third_party/webgl_conformance'):
        return True
    if file_path.startswith('third_party/hunspell_dictionaries'):
        return True
    if file_path.startswith('cloud_print'):
        return True
    if file_path.startswith('ios'):
        return True
    if file_path.startswith('google_update'):
        return True
    if file_path.startswith('courgette'):
        return True
    if file_path.startswith('native_client'):
        return True
    if (file_path.startswith('third_party/trace-viewer') and
        not file_path.endswith('.template') and
        not file_path.endswith('.html') and
        not file_path.endswith('.js') and
        not file_path.endswith('.css') and
        not file_path.endswith('.png') and
        not '/build/' in file_path):
        return True
    if file_path.startswith('remoting'):
        return True
    if file_path.startswith('win8'):
        return True
    if '.gitignore' in file_path:
        return True
    if '.gitmodules' in file_path:
        return True
    if '.DEPS' in file_path:
        return True
    return False


def createHardLinkForFile(src, dst):
    src = os.path.abspath(src)
    dst = os.path.abspath(dst)
    dst_dir = os.path.dirname(dst)

    if not os.path.isdir(dst_dir):
        os.makedirs(dst_dir)

    if os.path.exists(dst):
        os.remove(dst)

    try:
        os.link(src, dst)
    except OSError as exception:
        if exception.errno == errno.ENOENT:
            print 'file does not exist:' + src
        else:
            raise


if len(sys.argv) != 2:
    print 'usage: ' + sys.argv[0] + ' [output directory]'
    exit(1)


output_directory = sys.argv[1]
files = subprocess.check_output(['git', 'ls-files']).splitlines()
submodules = GitSubmodule.readSubmodules()
for submodule in submodules:
    submodule_files = submodule.listFiles()
    for submodule_file in submodule_files:
        files.append(os.path.join(submodule.path, submodule_file))

# Add LASTCHANGE files which are not tracked by git.
files.append('build/util/LASTCHANGE')
files.append('build/util/LASTCHANGE.blink')

print 'creating hardlinks...'
file_list = open('/tmp/file_list', 'w')
for f in files:
    if not isBlacklisted(f):
        createHardLinkForFile(f, os.path.join(output_directory, f))
        file_list.write(f + '\n');

file_list.close()

print 'done.'

