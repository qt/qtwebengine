include(../../httpserver/httpserver.cmake)
include(../../util/util.cmake)

qt_internal_add_test(tst_qmltests
    SOURCES
        tst_qmltests.cpp
    LIBRARIES
        Qt::WebEngineQuick
        Qt::QuickTest
        Test::HttpServer
        Test::Util
)

set(testList
    tst_action.qml
    tst_activeFocusOnPress.qml
    tst_audioMuted.qml
    tst_desktopBehaviorLoadHtml.qml
    tst_findText.qml
    tst_focusOnNavigation.qml
    tst_fullScreenRequest.qml
    tst_geopermission.qml
    tst_getUserMedia.qml
    tst_keyboardEvents.qml
    tst_keyboardModifierMapping.qml
    tst_loadHtml.qml
    tst_loadProgress.qml
    tst_loadRecursionCrash.qml
    tst_loadUrl.qml
    tst_mouseMove.qml
    tst_navigationRequested.qml
    tst_newViewRequest.qml
    tst_notification.qml
    tst_profile.qml
    tst_properties.qml
    tst_runJavaScript.qml
    tst_scrollPosition.qml
    tst_settings.qml
    tst_titleChanged.qml
    tst_unhandledKeyEventPropagation.qml
    tst_userScripts.qml
    tst_viewSource.qml
)

if(QT_FEATURE_webengine_webchannel)
    list(APPEND testList tst_webchannel.qml)
endif()

if(QT_FEATURE_ssl)
    list(APPEND testList tst_certificateError.qml)
endif()

if(QT_FEATURE_webengine_testsupport)
    list(APPEND testList
        tst_favicon.qml
        tst_faviconDownload.qml
        tst_inputMethod.qml
        tst_linkHovered.qml
        tst_loadFail.qml
        tst_mouseClick.qml
        tst_viewSoure.qml
    )
#    if(TARGET Qt::QuickControls) #FIXME
        list(APPEND testList tst_javaScriptDialogs.qml)
#    else()
#        message("\n!!! QuickControls target is missing, some tests are not executed !!! FIXME \n")
#    endif()
endif()

if(TARGET Qt::QuickControls) #FIXME
   list(APPEND testList
        tst_contextMenu.qml
        tst_download.qml
        tst_filePicker.qml
   )
else()
   message("\n!!!! QuickControls target is missing, some tests are not executed !!! FIXME \n")
endif()

if(TARGET Qt::QuickControls2)
   list(APPEND testList tst_navigationHistory.qml)
else()
   message("\n!!!! QuickControls2 target is missing, some tests are not executed !!! FIXME \n")
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
