// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef PDFAPPLICATION_H
#define PDFAPPLICATION_H

#include <QGuiApplication>
#include <QObject>

class PdfApplication : public QGuiApplication
{
public:
    PdfApplication(int &argc, char **argv);
    void setFileOpener(QObject *opener) {
        m_fileOpener = opener;
    }

protected:
    bool event(QEvent *e) override;

    QObject *m_fileOpener;
};

#endif // PDFAPPLICATION_H
