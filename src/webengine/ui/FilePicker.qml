import QtQuick.Dialogs 1.1

FileDialog {

    signal filesSelected(var fileList);

    onAccepted: {
        filesSelected(fileUrls);
    }
}
