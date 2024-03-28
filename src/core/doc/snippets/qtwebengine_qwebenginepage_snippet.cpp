// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

void wrapInFunction()
{

//! [0]
    m_view->page()->findText(QStringLiteral("Qt"), QWebEnginePage::FindFlags(), [this](const QWebEngineFindTextResult &result) {
        if (result.numberOfMatches() == 0) QMessageBox::information(m_view, QString(), QStringLiteral("No occurrences found"));
    });
//! [0]

}

