// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef PRINTHANDLER_H
#define PRINTHANDLER_H

#include <QEventLoop>
#include <QObject>
#include <QPrinter>

QT_BEGIN_NAMESPACE
class QPainter;
class QPrinter;
class QWebEngineView;
QT_END_NAMESPACE

class PrintHandler : public QObject
{
    Q_OBJECT
public:
    PrintHandler(QObject *parent = nullptr);
    void setView(QWebEngineView *view);

public slots:
    void print();
    void printPreview();
    void printDocument(QPrinter *printer);
    void printFinished(bool success);

private:
    QWebEngineView *m_view = nullptr;
    QPrinter m_printer;
    QEventLoop m_waitForResult;
    bool m_inPrintPreview = false;
};

#endif // PRINTHANDLER_H
