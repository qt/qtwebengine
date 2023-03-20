// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

FilePickerForm {
    property QtObject request
    property string selectedFile
    signal closeForm()

    cancelButton.onClicked: {
        request.dialogReject();
        closeForm();
    }

    okButton.onClicked: {
        request.dialogAccept('/' + selectedFile);
        closeForm();
    }

    function createCallback(fileIndex) {
        return function() {
            for (var i = 0; i < files.children.length; i++) {
                var file = files.children[i];
                if (i === fileIndex) {
                    selectedFile = file.text;
                    file.selected = true;
                } else {
                    file.selected = false;
                }
            }
        }
    }

    Component.onCompleted: {
        selectedFile = request.defaultFileName;
        for (var i = 0; i < files.children.length; i++) {
            var file = files.children[i];
            file.clicked.connect(createCallback(i));
            if (file.text === selectedFile)
                file.selected = true;
        }
    }
}
