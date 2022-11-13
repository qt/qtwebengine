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
snapshot_src_dir = os.path.abspath(os.path.join(qtwebengine_root, 'src/3rdparty'))
upstream_src_dir = os.path.abspath(snapshot_src_dir + '_upstream')

sys.path.append(os.path.join(qtwebengine_root, 'tools', 'scripts'))
import version_resolver as resolver
import git_submodule as GitSubmodule

target_dir = os.path.join(upstream_src_dir, 'chromium')
os.chdir(target_dir)
chromium = GitSubmodule.Submodule()
chromium.path = "chromium"
submodules = chromium.readSubmodules()
submodules.append(chromium)

patches = resolver.preparePatchesFromSnapshot()
errors = {}
magic_string = '+++ b/chromium/'
print('-- checking patch annotations --')
for module in submodules:
    for path in patches:
        patch_list = sorted(patches[path])
        for patch in patch_list:
            patch_file = os.path.basename(patch)
            # Check if the annotation points to a valid submodule dir.
            if path != 'chromium' and not os.path.isdir(path):
                sys.exit('\nERROR: ' + patch_file + ': specified annotation <' + path + '> is not a valid submodule directory')
            with open(patch, 'r') as pfile:
                line_no = 0
                for line in pfile:
                    line_no = line_no + 1
                    if not line.startswith(magic_string):
                        continue
                    line = line[len(magic_string):]
                    # Check if the annotation matches the files we patch.
                    if (line.startswith(module.path) and module.path != path
                        or
                        path != 'chromium' and module.path == path and not line.startswith(path)):
                            in_submodule = module.path

                            if not line.startswith(module.path):
                                in_submodule = 'chromium'
                            if patch_file not in errors:
                                errors[patch_file] = []

                            errors[patch_file].append('line ' + str(line_no)
                                                      + ': annotated with <' + path +'> but patching '
                                                      + line.strip() + ' in <' + in_submodule + '>')

if not errors:
    print('-- all good --')
else:
    print('-- errors found --\n')

for patch in errors:
    print(patch + ':')
    for error in errors[patch]:
        print('\t' + error)
