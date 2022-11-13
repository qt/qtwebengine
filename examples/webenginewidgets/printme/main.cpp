// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "printhandler.h"
#include <QApplication>
#include <QShortcut>
#include <QWebEngineView>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWebEngineView view;
    view.setUrl(QUrl(QStringLiteral("qrc:/index.html")));
    view.resize(1024, 750);
    view.show();

    PrintHandler handler;
    handler.setView(&view);

    auto printPreviewShortCut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_P), &view);
    auto printShortCut = new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_P), &view);

    QObject::connect(printPreviewShortCut, &QShortcut::activated, &handler, &PrintHandler::printPreview);
    QObject::connect(printShortCut, &QShortcut::activated, &handler, &PrintHandler::print);

    return app.exec();
}
