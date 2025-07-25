# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

set(qml_files
    "AlertDialog.qml"
    "ConfirmDialog.qml"
    "DirectoryPicker.qml"
    "FilePicker.qml"
    "Menu.qml"
    "MenuItem.qml"
    "PromptDialog.qml"
    "MenuParams.qml"
    "JSDialogParams.qml"
    "FilePickerParams.qml"
    "MenuSeparator.qml"
)

set_source_files_properties(MenuParams.qml FilePickerParams.qml JSDialogParams.qml PROPERTIES
    QT_QML_SINGLETON_TYPE TRUE
)

# test can be also configured with qt-cmake-standalone-test
# make sure we did not define qml module already in case of module build with tests

if(NOT TARGET Qt::WebEngineQuickTestMockDelegatesQml)
    qt_internal_add_qml_module(WebEngineQuickTestMockDelegatesQml
        URI "QtWebEngine.TestMockDelegates"
        VERSION "${PROJECT_VERSION}"
        QML_FILES ${qml_files}
        PAST_MAJOR_VERSIONS 1
        NO_SYNC_QT
        NO_PLUGIN
        NO_PRIVATE_MODULE
        NO_GENERATE_CPP_EXPORTS
        STATIC
    )
endif()

