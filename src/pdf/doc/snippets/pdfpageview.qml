// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [0]
import QtQuick
import QtQuick.Pdf

PdfPageView {
    document: PdfDocument { source: "my.pdf" }
}
//! [0]

