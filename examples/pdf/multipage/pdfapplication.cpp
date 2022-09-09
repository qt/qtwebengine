// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "pdfapplication.h"
#include <QFileOpenEvent>

PdfApplication::PdfApplication(int &argc, char **argv)
    : QGuiApplication(argc, argv) { }

bool PdfApplication::event(QEvent *e) {
    if (e->type() == QEvent::FileOpen) {
        QFileOpenEvent *foEvent = static_cast<QFileOpenEvent *>(e);
        m_fileOpener->setProperty("source", foEvent->url());
    }
    return QGuiApplication::event(e);
}
