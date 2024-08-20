// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
import QtQuick

Image {
    id: image
    source: "test.pdf"
    fillMode: Image.PreserveAspectFit
    Shortcut {
        sequence: StandardKey.MoveToNextPage
        enabled: image.currentFrame < image.frameCount - 1
        onActivated: image.currentFrame++
    }
    Shortcut {
        sequence: StandardKey.MoveToPreviousPage
        enabled: image.currentFrame > 0
        onActivated: image.currentFrame--
    }
}
