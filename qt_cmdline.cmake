# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_commandline_option(build-qtpdf TYPE boolean NAME qtpdf-build)
qt_commandline_option(webengine-developer-build TYPE boolean)

qt_commandline_subconfig(src/core/api)
