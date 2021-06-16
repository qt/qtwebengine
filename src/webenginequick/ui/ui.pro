TARGETPATH = QtWebEngine/ControlsDelegates

QML_FILES += \
    # Authentication Dialog
    AuthenticationDialog.qml \
    # JS Dialogs
    AlertDialog.qml \
    ColorDialog.qml \
    ConfirmDialog.qml \
    FilePicker.qml \
    PromptDialog.qml \
    # Menus. Based on Qt Quick Controls
    Menu.qml \
    MenuItem.qml \
    MenuSeparator.qml \
    ToolTip.qml \
    information.png \
    question.png

load(qml_module)
