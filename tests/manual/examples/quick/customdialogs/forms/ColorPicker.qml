// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

ColorPickerForm {
    property QtObject request
    signal closeForm()

    okButton.onClicked: {
        request.dialogAccept(colorPicker.color);
        closeForm();
    }

    cancelButton.onClicked: {
        request.dialogReject();
        closeForm();
    }

    function createCallback(color) {
        return function() { colorPicker.color = color };
    }

    Component.onCompleted:{
        for (var i = 0; i < grid.children.length; i++) {
            var cell = grid.children[i];
            cell.clicked.connect(createCallback(cell.color));
        }
        colorPicker.color = request.color;
    }
}
