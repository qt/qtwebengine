# Copyright (C) 2016 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import re
import sys
import os

mocables = set()
includedMocs = set()
files = sys.argv[1:]

for f in filter(os.path.isfile, files):
    inBlockComment = False
    for line in open(f).readlines():
        # Block comments handling
        if "/*" in line:
            inBlockComment = True
        if inBlockComment and "*/" in line:
            inBlockComment = False
            if line.find("*/") != len(line) - 3:
                line = line[line.find("*/")+2:]
            else:
                continue
        if inBlockComment:
            continue
        #simple comments handling
        if "//" in line:
            line = line.partition("//")[0]
        if re.match(".*Q_OBJECT", line):
            mocables.add(f)
        im = re.search('#include "(moc_\w+.cpp)"', line)
        if im:
            includedMocs.add(im.group(1))

for mocable in includedMocs:
    print("Found included moc: " + mocable)

assert len(includedMocs) == 0 , "Included mocs are not supported !"

for mocable in mocables:
    print(mocable)
sys.exit(0)
