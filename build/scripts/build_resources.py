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
import time

qtwebengine_src = os.path.abspath(os.path.join(os.path.dirname(__file__), "../.."))


chrome_src = subprocess.check_output("git config qtwebengine.chromiumsrcdir || true", shell=True).strip()
if chrome_src:
  chrome_src = os.path.join(qtwebengine_src, chrome_src)
if not chrome_src or not os.path.isdir(chrome_src):
  chrome_src = os.path.join(qtwebengine_src, '3rdparty/chromium')
  print 'CHROMIUM_SRC_DIR not set, falling back to ' + chrome_src

grit_tool = os.path.join(chrome_src, 'tools/grit/grit.py')
resources_subdir = os.path.join(qtwebengine_src, 'resources')

def checkNeedForRebuild(grd_file):
    grit_files = subprocess.check_output(['python', grit_tool, '-i', grd_file, 'buildinfo']).splitlines()

    dependencies = [grd_file]
    data_packages = []
    for line in grit_files:
        if line.startswith('input|'):
            dependencies.append(line.split('|')[1])
        if line.startswith('data_package|'):
            data_packages.append(line.split('|')[1])

    target_timestamp = 0
    for data_package in data_packages:
        data_package_file = os.path.join(resources_subdir, data_package)
        if not os.path.isfile(data_package_file):
            return True

        data_package_timestamp = os.path.getmtime(data_package_file)
        if data_package_timestamp < target_timestamp or target_timestamp == 0:
            target_timestamp = data_package_timestamp

    for dependency in dependencies:
        dependency_timestamp = os.path.getmtime(dependency)
        if (dependency_timestamp > target_timestamp):
            return True
    return False

def rebuildPakFile(grd_file):
    print 'Rebuilding resource file for:' + grd_file
    resource_ids_file = os.path.join(chrome_src, 'tools/gritsettings/resource_ids')
    subprocess.call(['python', grit_tool, '-i', grd_file, 'build', '-f', resource_ids_file, '-o', resources_subdir])

def rebuildIfNeeded(grd_file):
    grd_file = os.path.join(chrome_src, grd_file)
    if checkNeedForRebuild(grd_file):
        rebuildPakFile(grd_file)


# The grd_file is specified relative to the chromium source directory.
rebuildIfNeeded('net/base/net_resources.grd')
