#!/usr/bin/env python
# Copyright (C) 2016 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import glob
import os
import re
import shutil
import subprocess
import sys

qtwebengine_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
sys.path.append(os.path.join(qtwebengine_root, 'tools', 'scripts'))

import version_resolver as resolver

os.chdir(os.path.join(resolver.upstream_src_dir, 'chromium'))

if len(sys.argv) != 1:
    if "--reset" in sys.argv:
        resolver.resetUpstream()
        sys.exit()
    else:
        sys.exit('unknown option, try --reset or w/o options\n')

if not len(resolver.findSnapshotBaselineSha1()):
    sys.exit('\n-- missing chromium snapshot patches, try running init-repository.py w/o options first --')

patches = resolver.preparePatchesFromSnapshot()

for path in patches:
    leading = path.count('/') + 2
    target_dir = ""

    if path.startswith('chromium'):
        target_dir = os.path.join(resolver.upstream_src_dir, path)
    else:
        target_dir = os.path.join(resolver.upstream_src_dir, 'chromium', path)
        leading += 1

    if not os.path.isdir(target_dir):
        # Skip applying patches for non-existing submodules
        print('\n-- missing '+ target_dir + ', skipping --')
        continue

    os.chdir(target_dir)
    print('\n-- entering '+ os.getcwd() + ' --')

    # Sort the patches to be able to apply them in order
    patch_list = sorted(patches[path])
    for patch in patch_list:
        error = subprocess.call(['git', 'am', '-p' + str(leading), patch])
        if error != 0:
            sys.exit('-- git am ' + patch + ' failed in ' + os.getcwd() + ' --')

print('\n-- done --')

