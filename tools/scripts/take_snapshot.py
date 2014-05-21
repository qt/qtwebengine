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
import shutil

import git_submodule as GitSubmodule

qtwebengine_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
os.chdir(qtwebengine_root)

def isInGitBlacklist(file_path):
    # We do need all the gyp files.
    if file_path.endswith('.gyp') or file_path.endswith('.gypi'):
        False
    if ( '.gitignore' in file_path
        or '.gitmodules' in file_path
        or '.DEPS' in file_path ):
        return True

def isInChromiumBlacklist(file_path):
    # Filter out empty submodule directories.
    if (os.path.isdir(file_path)):
        return True
    # We do need all the gyp files.
    if file_path.endswith('.gyp') or file_path.endswith('.gypi'):
        return False
    if ( '_jni' in file_path
        or 'jni_' in file_path
        or 'testdata/' in file_path
        or (file_path.startswith('third_party/android_tools') and
            not 'android/cpufeatures' in file_path)
        or '/tests/' in file_path
        or ('/test/' in file_path and
            not file_path.endswith('isolate') and
            not '/webrtc/test/testsupport/' in file_path and
            not file_path.startswith('net/test/') and
            not file_path.endswith('mock_chrome_application_mac.h') and
            not file_path.endswith('perftimer.h') and
            not 'ozone' in file_path)
        or file_path.endswith('.java')
        or file_path.startswith('android_webview')
        or file_path.startswith('apps/')
        or file_path.startswith('base/android/java')
        or file_path.startswith('breakpad')
        or file_path.startswith('build/android/')
        or (file_path.startswith('chrome/') and
            not file_path.startswith('chrome/VERSION') and
            not '/app/theme/' in file_path and
            not '/app/resources/' in file_path and
            not '/browser/resources/' in file_path and
            not '/renderer/resources/' in file_path and
            not 'repack_locales' in file_path and
            not 'third_party/chromevox' in file_path and
            not 'media/desktop_media_list.h' in file_path and
            not 'media/desktop_streams_registry.cc' in file_path and
            not 'media/desktop_streams_registry.h' in file_path and
            not file_path.endswith('.isolate') and
            not file_path.endswith('cf_resources.rc') and
            not file_path.endswith('version.py') and
            not file_path.endswith('.grd') and
            not file_path.endswith('.grdp') and
            not file_path.endswith('.json'))
        or file_path.startswith('chrome_frame')
        or file_path.startswith('chromeos')
        or file_path.startswith('cloud_print')
        or file_path.startswith('content/public/android/java')
        or file_path.startswith('content/shell/android/java')
        or file_path.startswith('courgette')
        or file_path.startswith('extensions')
        or file_path.startswith('google_update')
        or file_path.startswith('ios')
        or file_path.startswith('media/base/android/java')
        or file_path.startswith('mojo')
        or file_path.startswith('native_client')
        or file_path.startswith('net/android/java')
        or file_path.startswith('remoting')
        or file_path.startswith('rlz')
        or file_path.startswith('sync/android')
        or file_path.startswith('testing/android')
        or file_path.startswith('third_party/accessibility-developer-tools')
        or file_path.startswith('third_party/GTM')
        or file_path.startswith('third_party/WebKit/LayoutTests')
        or file_path.startswith('third_party/WebKit/ManualTests')
        or file_path.startswith('third_party/WebKit/PerformanceTests')
        or file_path.startswith('third_party/active_doc')
        or file_path.startswith('third_party/android_platform')
        or file_path.startswith('third_party/android_testrunner')
        or file_path.startswith('third_party/aosp')
        or file_path.startswith('third_party/apache-mime4j')
        or file_path.startswith('third_party/apple_sample_code')
        or file_path.startswith('third_party/bison')
        or file_path.startswith('third_party/brotli')
        or file_path.startswith('third_party/chromite')
        or file_path.startswith('third_party/cld_2')
        or file_path.startswith('third_party/codesighs')
        or file_path.startswith('third_party/cros_dbus_cplusplus')
        or file_path.startswith('third_party/cros_system_api')
        or file_path.startswith('third_party/eyesfree')
        or file_path.startswith('third_party/findbugs')
        or file_path.startswith('third_party/gperf')
        or file_path.startswith('third_party/gnu_binutils')
        or file_path.startswith('third_party/gtk+')
        or file_path.startswith('third_party/guava/src')
        or file_path.startswith('third_party/httpcomponents-client')
        or file_path.startswith('third_party/httpcomponents-core')
        or file_path.startswith('third_party/hunspell')
        or file_path.startswith('third_party/hunspell_dictionaries')
        or file_path.startswith('third_party/jarjar')
        or file_path.startswith('third_party/jsr-305/src')
        or file_path.startswith('third_party/libphonenumber')
        or file_path.startswith('third_party/libaddressinput')
        or file_path.startswith('third_party/liblouis')
        or file_path.startswith('third_party/lighttpd')
        or file_path.startswith('third_party/mingw-w64')
        or file_path.startswith('third_party/nacl_sdk_binaries')
        or file_path.startswith('third_party/pdfsqueeze')
        or file_path.startswith('third_party/pefile')
        or file_path.startswith('third_party/perl')
        or file_path.startswith('third_party/psyco_win32')
        or file_path.startswith('third_party/python_26')
        or file_path.startswith('third_party/scons-2.0.1')
        or file_path.startswith('third_party/syzygy')
        or file_path.startswith('third_party/swig')
        or file_path.startswith('third_party/webgl')
        or (file_path.startswith('third_party/trace-viewer') and
            not file_path.endswith('.template') and
            not file_path.endswith('.html') and
            not file_path.endswith('.js') and
            not file_path.endswith('.css') and
            not file_path.endswith('.png') and
            not '/build/' in file_path)
        or file_path.startswith('third_party/xulrunner-sdk')
        or file_path.startswith('tools/android')
        or file_path.startswith('tools/deep_memory_profiler')
        or file_path.startswith('tools/gn')
        or file_path.startswith('tools/page_cycler')
        or file_path.startswith('tools/perf')
        or file_path.startswith('tools/swarming_client')
        or file_path.startswith('tools/swarm_client')
        or file_path.startswith('tools/telemetry')
        or (file_path.startswith('tools/win') and
           not 'toolchain' in file_path)
        or file_path.startswith('ui/android/java')

        ):
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

third_party_upstream = os.path.join(qtwebengine_root, 'src/3rdparty_upstream')
third_party = os.path.join(qtwebengine_root, 'src/3rdparty')

def clearDirectory(directory):
    currentDir = os.getcwd()
    os.chdir(directory)
    print 'clearing the directory:' + directory
    for direntry in os.listdir(directory):
        if not direntry == '.git':
            print 'clearing:' + direntry
            shutil.rmtree(direntry)
    os.chdir(currentDir)

def listFilesInCurrentRepository():
    currentRepo = GitSubmodule.Submodule(os.getcwd())
    files = subprocess.check_output(['git', 'ls-files']).splitlines()
    submodules = currentRepo.readSubmodules()
    for submodule in submodules:
        submodule_files = submodule.listFiles()
        for submodule_file in submodule_files:
            files.append(os.path.join(submodule.path, submodule_file))
    return files

def exportNinja():
    third_party_upstream_ninja = os.path.join(third_party_upstream, 'ninja')
    third_party_ninja = os.path.join(third_party, 'ninja')
    os.makedirs(third_party_ninja);
    print 'exporting contents of:' + third_party_upstream_ninja
    os.chdir(third_party_upstream_ninja)
    files = listFilesInCurrentRepository()
    print 'creating hardlinks in ' + third_party_ninja
    for f in files:
        if not isInGitBlacklist(f):
            createHardLinkForFile(f, os.path.join(third_party_ninja, f))

def exportChromium():
    third_party_upstream_chromium = os.path.join(third_party_upstream, 'chromium')
    third_party_chromium = os.path.join(third_party, 'chromium')
    os.makedirs(third_party_chromium);
    print 'exporting contents of:' + third_party_upstream_chromium
    os.chdir(third_party_upstream_chromium)
    files = listFilesInCurrentRepository()
    # Add LASTCHANGE files which are not tracked by git.
    files.append('build/util/LASTCHANGE')
    files.append('build/util/LASTCHANGE.blink')
    print 'creating hardlinks in ' + third_party_chromium
    for f in files:
        if not isInChromiumBlacklist(f) and not isInGitBlacklist(f):
            createHardLinkForFile(f, os.path.join(third_party_chromium, f))

clearDirectory(third_party)

exportNinja()
exportChromium()

print 'done.'

