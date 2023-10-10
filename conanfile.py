# Copyright (C) 2021 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

from conans import ConanFile
import re
from pathlib import Path
from typing import Dict, Any

_qtwebengine_features = [
    "qtpdf-build",
    "qtpdf-quick-build",
    "qtpdf-widgets-build",
    "qtwebengine-build",
    "qtwebengine-core-build",
    "qtwebengine-quick-build",
    "qtwebengine-widgets-build",
    "webengine-developer-build",
    "webengine-embedded-build",
    "webengine-extensions",
    "webengine-full-debug-info",
    "webengine-jumbo-build",
    "webengine-kerberos",
    "webengine-native-spellchecker",
    "webengine-pepper-plugins",
    "webengine-printing-and-pdf",
    "webengine-proprietary-codecs",
    "webengine-sanitizer",
    "webengine-spellchecker",
    "webengine-webchannel",
    "webengine-webrtc",
    "webengine-webrtc-pipewire",
    "system-webengine-ffmpeg",
    "system-webengine-icu",
]


def _parse_qt_version_by_key(key: str) -> str:
    with open(Path(__file__).parent.resolve() / ".cmake.conf") as f:
        m = re.search(fr'{key} .*"(.*)"', f.read())
    return m.group(1) if m else ""


def _get_qt_minor_version() -> str:
    return ".".join(_parse_qt_version_by_key("QT_REPO_MODULE_VERSION").split(".")[:2])


class QtWebEngine(ConanFile):
    name = "qtwebengine"
    license = "LGPL-3.0-only, Commercial Qt License Agreement"
    author = "The Qt Company <https://www.qt.io/contact-us>"
    url = "https://code.qt.io/cgit/qt/qtwebengine.git"
    description = (
        "Qt WebEngine integrates Chromium's fast moving web capabilities into Qt. "
        "The integration with Qt focuses on an API that is easy to use, yet extensible. "
        "We also make no compromise on the graphics integration, integrating the layer "
        "rendering of Chromium directly into the OpenGL scene graph of Qt Quick."
    )
    topics = "qt", "qt6", "qtwebengine", "WebEngine", "Chromium"
    settings = "os", "compiler", "arch", "build_type"
    # for referencing the version number and prerelease tag and dependencies info
    exports = ".cmake.conf", "dependencies.yaml"
    exports_sources = "*", "!conan*.*"
    python_requires = f"qt-conan-common/{_get_qt_minor_version()}@qt/everywhere"
    python_requires_extend = "qt-conan-common.QtLeafModule"

    def get_qt_leaf_module_options(self) -> Dict[str, Any]:
        """Implements abstractmethod from qt-conan-common.QtLeafModule"""
        return self._shared.convert_qt_features_to_conan_options(_qtwebengine_features)

    def get_qt_leaf_module_default_options(self) -> Dict[str, Any]:
        """Implements abstractmethod from qt-conan-common.QtLeafModule"""
        return self._shared.convert_qt_features_to_default_conan_options(_qtwebengine_features)

    def package_env_info(self) -> Dict[str, Any]:
        """Implements abstractmethod from qt-conan-common.QtLeafModule"""
        # this will be called only after a successful build
        _f = lambda p: True
        if tools.os_info.is_windows:
            ptrn = "**/QtWebEngineProcess.exe"
        elif tools.os_info.is_macos:
            ptrn = "**/QtWebEngineProcess.app/**/QtWebEngineProcess"
            _f = lambda p: not any(".dSYM" in item for item in p.parts)
        else:
            ptrn = "**/QtWebEngineProcess"
        ret = [str(p) for p in Path(self.package_folder).rglob(ptrn) if p.is_file() and _f(p)]
        if len(ret) != 1:
            print("Expected to find one 'QtWebEngineProcess'. Found: {0}".format(ret))
        return {"QTWEBENGINEPROCESS_PATH": ret.pop() if ret else ""}

