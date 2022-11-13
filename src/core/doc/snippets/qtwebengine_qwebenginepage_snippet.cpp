// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GFDL-1.3-no-invariants-only

void wrapInFunction()
{

//! [0]
    m_view->page()->findText(QStringLiteral("Qt"), QWebEnginePage::FindFlags(), [this](bool found) {
        if (!found) QMessageBox::information(m_view, QString(), QStringLiteral("No occurrences found"));
    });
//! [0]

}

