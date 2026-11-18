# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Runs qt_webengine_configure_check_coin() in script mode.
include("${HELPERS_FILE}")
qt_webengine_configure_check_coin("${WHITE_LIST_FILE}" "${WEBENGINE_CHECK}" "${PDF_CHECK}")
