# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

qt_commandline_option(build-qtpdf TYPE boolean NAME qtpdf-build)
qt_commandline_option(webengine-developer-build TYPE boolean)

qt_commandline_subconfig(src/core/api)
