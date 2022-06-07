# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if (Qt6Quick_VERSION VERSION_GREATER_EQUAL "6.4.0")
set(colorDialog "ColorDialog.qml")
else()
set(colorDialog "custom/ColorDialog.qml")
endif()

set(qml_files
    "AlertDialog.qml"
    "AuthenticationDialog.qml"
    "AutofillPopup.qml"
    "ConfirmDialog.qml"
    "DirectoryPicker.qml"
    "FilePicker.qml"
    "Menu.qml"
    "MenuItem.qml"
    "MenuSeparator.qml"
    "PromptDialog.qml"
    "ToolTip.qml"
    "TouchHandle.qml"
    "TouchSelectionMenu.qml"
    ${colorDialog}
)

set(resource_files
    "information.png"
    "question.png"
)

qt_internal_add_qml_module(WebEngineQuickDelegatesQml
    URI "QtWebEngine.ControlsDelegates"
    VERSION "${PROJECT_VERSION}"
    QML_FILES ${qml_files}
    PAST_MAJOR_VERSIONS 1
    NO_SYNC_QT
    PLUGIN_TARGET qtwebenginequickdelegatesplugin
    DEPENDENCIES QtQuickControls2
)

qt_internal_add_resource(qtwebenginequickdelegatesplugin "qtwebenginequickdelegatesplugin"
    PREFIX
        "/qt-project.org/imports/QtWebEngine/ControlsDelegates"
    FILES
        ${resource_files}
)


