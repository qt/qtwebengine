#!/usr/bin/env python
# Copyright (C) 2016 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import sys
import json
import version_resolver as resolver

channels = resolver.readReleaseChannels()

if len(sys.argv) == 1 or sys.argv[1] == 'all':
    print(json.dumps(channels, sort_keys=True, indent=2))

for platform in sys.argv:
    if platform in channels:
        print('"' + platform + '": ' + json.dumps(channels[platform], sort_keys=True, indent=2))
