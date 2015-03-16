TARGETPATH = QtWebEngine/UIDelegates

QML_FILES += \
    # JS Dialogs
    AlertDialog.qml \
    ConfirmDialog.qml \
    FilePicker.qml \
    PromptDialog.qml \
    # Menus. Based on Qt Quick Controls
    Menu.qml \
    MenuItem.qml \
    MenuSeparator.qml \
    # Message Bubble
    MessageBubble.qml

load(qml_module)
