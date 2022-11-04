// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [0]
import QtQuick
import QtQuick.Pdf

PdfMultiPageView {
    document: PdfDocument { source: "my.pdf" }
}
//! [0]
