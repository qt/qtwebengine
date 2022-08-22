# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_example(customdialogs)
qt_internal_add_example(customtouchhandle)
qt_internal_add_example(lifecycle)
qt_internal_add_example(minimal)
qt_internal_add_example(quicknanobrowser)
qt_internal_add_example(webengineaction)
if(TARGET Qt::QuickControls2)
    qt_internal_add_example(recipebrowser)
endif()
