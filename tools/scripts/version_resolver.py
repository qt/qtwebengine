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

import os
import sys
import json
import urllib2
import git_submodule as GitSubmodule

chromium_version = '31.0.1650.63'
chromium_branch = '1650'

json_url = 'http://omahaproxy.appspot.com/all.json'
git_deps_url = 'http://src.chromium.org/chrome/trunk/src/.DEPS.git'
base_deps_url = 'http://src.chromium.org/chrome/releases/'

def currentVersion():
    return chromium_version

def currentBranch():
    return chromium_branch

def readReleaseChannels():
    response = urllib2.urlopen(json_url)
    raw_json = response.read().strip()
    data = json.loads(raw_json)
    channels = {}

    for obj in data:
        os = obj['os']
        channels[os] = []
        for ver in obj['versions']:
            channels[os].append({ 'channel': ver['channel'], 'version': ver['version'], 'branch': ver['true_branch'] })
    return channels

def readSubmodules():
    response = urllib2.urlopen(base_deps_url + chromium_version + '/DEPS')
    svn_deps = response.read().strip()

    response = urllib2.urlopen(git_deps_url)
    git_deps = response.read().strip()

    parser = GitSubmodule.DEPSParser()
    svn_submodules = parser.parse(svn_deps)
    git_submodules = parser.parse(git_deps)

    submodule_dict = {}
    git_dict = {}

    for sub in git_submodules:
        git_dict[sub.path] = sub

    for sub in svn_submodules:
        if 'reference_build' not in sub.path and (sub.revision or sub.shasum) and sub.path in git_dict:
            submodule_dict[sub.path] = sub

    for git in git_submodules:
        if git.path in submodule_dict:
            # We'll use the git repository instead of svn.
            module = submodule_dict[git.path]
            module.url = git.url
            if not module.shasum:
                # We use the git shasum as fallback.
                module.shasum = git.shasum

    return list(submodule_dict.values())
