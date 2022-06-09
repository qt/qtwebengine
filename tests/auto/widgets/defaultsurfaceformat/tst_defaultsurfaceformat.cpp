// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <qtest.h>
#include <util.h>

#include <QSurfaceFormat>
#include <QTimer>
#include <qwebengineview.h>

class tst_QWebEngineDefaultSurfaceFormat : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void customDefaultSurfaceFormat();
};

void tst_QWebEngineDefaultSurfaceFormat::customDefaultSurfaceFormat()
{
#if !defined(Q_OS_MACOS)
    QSKIP("OpenGL Core Profile is currently only supported on macOS.");
#endif
    // Setting a new default QSurfaceFormat with a core OpenGL profile before
    // app instantiation should succeed, without abort() being called.
    int argc = 1;
    char *argv[] = { const_cast<char*>("tst_QWebEngineDefaultSurfaceFormat") };

    QSurfaceFormat format;
    format.setVersion( 3, 3 );
    format.setProfile( QSurfaceFormat::CoreProfile );
    QSurfaceFormat::setDefaultFormat( format );

    QApplication app(argc, argv);

    QWebEngineView view;
    view.load(QUrl("qrc:///resources/index.html"));
    view.show();
    QObject::connect(&view, &QWebEngineView::loadFinished, []() {
        QTimer::singleShot(100, qApp, &QCoreApplication::quit);
    });

    QCOMPARE(app.exec(), 0);
}

QTEST_APPLESS_MAIN(tst_QWebEngineDefaultSurfaceFormat)
#include "tst_defaultsurfaceformat.moc"
