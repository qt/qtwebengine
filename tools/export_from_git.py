#!/usr/bin/env python

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
        not file_path.startswith('net/test/')):
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
    if file_path.startswith('chrome/app/'):
        return True
    if file_path.startswith('chrome/installer'):
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

