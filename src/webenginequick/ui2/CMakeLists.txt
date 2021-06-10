set(qml_files
    "AlertDialog.qml"
    "AuthenticationDialog.qml"
    "ConfirmDialog.qml"
    "Menu.qml"
    "MenuItem.qml"
    "MenuSeparator.qml"
    "PromptDialog.qml"
    "ToolTip.qml"
)

set(resource_files
    "information.png"
    "question.png"
)

qt_internal_add_qml_module(WebEngineQuickDelegatesQml
    URI "QtWebEngine.Controls2Delegates"
    VERSION "${PROJECT_VERSION}"
    QML_FILES ${qml_files}
    PAST_MAJOR_VERSIONS 1
    NO_SYNC_QT
    PLUGIN_TARGET qtwebenginequickdelegatesplugin
    DEPENDENCIES QtQuickControls2
)

qt_internal_add_resource(qtwebenginequickdelegatesplugin "qtwebenginequickdelegatesplugin"
    PREFIX
        "/qt-project.org/imports/QtWebEngine/Controls2Delegates"
    FILES
        ${resource_files}
)

make_install_only(WebEngineQuickDelegatesQml)
make_install_only(qtwebenginequickdelegatesplugin)

