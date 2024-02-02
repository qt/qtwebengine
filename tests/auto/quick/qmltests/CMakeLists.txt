# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

include(../../httpserver/httpserver.cmake)
include(../../util/util.cmake)

qt_internal_add_test(tst_qmltests
    SOURCES
        tst_qmltests.cpp
    LIBRARIES
        Qt::GuiPrivate
        Qt::QuickTest
        Qt::TestPrivate
        Qt::WebEngineQuick
        Test::HttpServer
        Test::Util
)

set(testList
    tst_action.qml
    tst_activeFocusOnPress.qml
    tst_audioMuted.qml
    tst_contextMenu.qml
    tst_basicProfiles.qml
    tst_datalist.qml
    tst_desktopBehaviorLoadHtml.qml
    tst_download.qml
    tst_dragHandlerUnderView.qml
    tst_favicon.qml
    tst_faviconDatabase.qml
    tst_filePicker.qml
    tst_filesystem.qml
    tst_findText.qml
    tst_focusOnNavigation.qml
    tst_fullScreenRequest.qml
    tst_getUserMedia.qml
    tst_inputMethod.qml
    tst_inputTextDirection.qml
    tst_javaScriptDialogs.qml
    tst_keyboardEvents.qml
    tst_keyboardModifierMapping.qml
    tst_linkHovered.qml
    tst_loadFail.qml
    tst_loadHtml.qml
    tst_loadProgress.qml
    tst_loadRecursionCrash.qml
    tst_loadUrl.qml
    tst_mouseClick.qml
    tst_mouseMove.qml
    tst_navigationHistory.qml
    tst_navigationRequested.qml
    tst_newViewRequest.qml
    tst_notification.qml
    tst_properties.qml
    tst_runJavaScript.qml
    tst_scrollPosition.qml
    tst_settings.qml
    tst_titleChanged.qml
    tst_unhandledKeyEventPropagation.qml
    tst_userScripts.qml
    tst_userScriptCollection.qml
    tst_viewSource.qml
    tst_save.qml
)

if(QT_FEATURE_webengine_webchannel)
    list(APPEND testList tst_webchannel.qml)
endif()

if(QT_FEATURE_ssl)
    list(APPEND testList tst_certificateError.qml)
endif()

if (NOT APPLE)
   list(APPEND testList tst_geopermission.qml)
endif()

set(content "")
foreach(test ${testList})
   set(contents "${contents}${CMAKE_CURRENT_LIST_DIR}/data/${test}\n")
endforeach()
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/webengine.qmltests ${contents})

set(tst_qmltests_resource_files
    "resources/server.pem"
    "resources/server.key"
)

qt_internal_add_resource(tst_qmltests "tst_qmltests"
    PREFIX
       "/"
    FILES
       ${tst_qmltests_resource_files}
)
