#!/usr/bin/env python3
# Copyright (C) 2016 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import os
import subprocess
import sys

def getChromiumSrcDir():

  saved_cwd = os.getcwd()
  qtwebengine_root = os.path.abspath(os.path.join(os.path.dirname(__file__), "../.."))

  os.chdir(qtwebengine_root)
  try:
    chrome_src = subprocess.check_output("git config qtwebengine.chromiumsrcdir", shell=True).strip()
  except subprocess.CalledProcessError:
    chrome_src = None
  os.chdir(saved_cwd)

  if chrome_src:
    chrome_src = os.path.join(qtwebengine_root, chrome_src)
    print('Using external chromium sources specified in git config qtwebengine.chromiumsrcdir: ' + chrome_src)
  if not chrome_src or not os.path.isdir(chrome_src):
    chrome_src = os.path.normpath(os.path.join(qtwebengine_root, 'src/3rdparty/chromium'))
  return os.path.normcase(chrome_src)

