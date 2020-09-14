include(../tests.pri)
include(../../shared/http.pri)

QT += qmltest

IMPORTPATH += $$PWD/data

OTHER_FILES += \
    $$PWD/data/TestWebEngineView.qml \
    $$PWD/data/append-document-title.js \
    $$PWD/data/big-user-script.js \
    $$PWD/data/change-document-title.js \
    $$PWD/data/download.zip \
    $$PWD/data/directoryupload.html \
    $$PWD/data/favicon.html \
    $$PWD/data/forms.html \
    $$PWD/data/geolocation.html \
    $$PWD/data/javascript.html \
    $$PWD/data/link.html \
    $$PWD/data/localStorage.html \
    $$PWD/data/multifileupload.html \
    $$PWD/data/redirect.html \
    $$PWD/data/script-with-metadata.js \
    $$PWD/data/singlefileupload.html \
    $$PWD/data/test1.html \
    $$PWD/data/test2.html \
    $$PWD/data/test3.html \
    $$PWD/data/test4.html \
    $$PWD/data/test-iframe.html \
    $$PWD/data/keyboardModifierMapping.html \
    $$PWD/data/keyboardEvents.html \
    $$PWD/data/titleupdate.js \
    $$PWD/data/tst_action.qml \
    $$PWD/data/tst_activeFocusOnPress.qml \
    $$PWD/data/tst_audioMuted.qml \
    $$PWD/data/tst_contextMenu.qml \
    $$PWD/data/tst_desktopBehaviorLoadHtml.qml \
    $$PWD/data/tst_download.qml \
    $$PWD/data/tst_filePicker.qml \
    $$PWD/data/tst_findText.qml \
    $$PWD/data/tst_focusOnNavigation.qml \
    $$PWD/data/tst_geopermission.qml \
    $$PWD/data/tst_getUserMedia.qml \
    $$PWD/data/tst_loadHtml.qml \
    $$PWD/data/tst_loadProgress.qml \
    $$PWD/data/tst_loadRecursionCrash.qml \
    $$PWD/data/tst_loadUrl.qml \
    $$PWD/data/tst_mouseMove.qml \
    $$PWD/data/tst_navigationHistory.qml \
    $$PWD/data/tst_navigationRequested.qml \
    $$PWD/data/tst_newViewRequest.qml \
    $$PWD/data/tst_notification.qml \
    $$PWD/data/tst_profile.qml \
    $$PWD/data/tst_properties.qml \
    $$PWD/data/tst_runJavaScript.qml \
    $$PWD/data/tst_scrollPosition.qml \
    $$PWD/data/tst_titleChanged.qml \
    $$PWD/data/tst_unhandledKeyEventPropagation.qml \
    $$PWD/data/tst_userScripts.qml \
    $$PWD/data/tst_viewSource.qml \
    $$PWD/data/tst_webchannel.qml \
    $$PWD/data/tst_settings.qml \
    $$PWD/data/tst_keyboardModifierMapping.qml \
    $$PWD/data/tst_keyboardEvents.qml \
    $$PWD/data/webchannel-test.html \
    $$PWD/data/icons/favicon.png \

load(qt_build_paths)
DEFINES += QUICK_TEST_SOURCE_DIR=\\\"$$re_escape($$PWD$${QMAKE_DIR_SEP}data)\\\"
