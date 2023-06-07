# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

add_subdirectory(dialogs)
add_subdirectory(publicapi)
add_subdirectory(qquickwebenginedefaultsurfaceformat)
add_subdirectory(qtbug-70248)
# Re-enable if QTBUG-101744 and QTBUG-103354 have been fixed. 
if(NOT MACOS)
    add_subdirectory(uidelegates)
endif()
add_subdirectory(inspectorserver)
add_subdirectory(qmltests)
add_subdirectory(qquickwebengineview)
add_subdirectory(qquickwebengineviewgraphics)
