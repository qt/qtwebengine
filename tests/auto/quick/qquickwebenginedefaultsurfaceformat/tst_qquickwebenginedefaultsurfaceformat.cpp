// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "testwindow.h"
#include "quickutil.h"

#include <QGuiApplication>
#include <QtQml/QQmlEngine>
#include <QScopedPointer>
#include <QSurfaceFormat>
#include <QtTest/QtTest>
#include <QTimer>
#include <private/qquickwebengineview_p.h>
#include <QQuickWebEngineProfile>

class tst_QQuickWebEngineDefaultSurfaceFormat : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void customDefaultSurfaceFormat();

private:
    inline void initEngineAndViewComponent();
    inline void initWindow();
    inline void deleteWindow();

    inline QQuickWebEngineView *newWebEngineView();
    inline QQuickWebEngineView *webEngineView() const;
    QUrl urlFromTestPath(const char *localFilePath);

    QQmlEngine *m_engine;
    TestWindow *m_window;
    QScopedPointer<QQmlComponent> m_component;
};

void tst_QQuickWebEngineDefaultSurfaceFormat::initEngineAndViewComponent() {
    m_engine = new QQmlEngine(this);
    QQuickWebEngineProfile::defaultProfile()->setOffTheRecord(true);
    m_component.reset(new QQmlComponent(m_engine, this));
    m_component->setData(QByteArrayLiteral("import QtQuick\n"
                                           "import QtWebEngine\n"
                                           "WebEngineView {}")
                         , QUrl());
}

void tst_QQuickWebEngineDefaultSurfaceFormat::initWindow()
{
    m_window = new TestWindow(newWebEngineView());
}

void tst_QQuickWebEngineDefaultSurfaceFormat::deleteWindow()
{
    delete m_window;
}

QQuickWebEngineView *tst_QQuickWebEngineDefaultSurfaceFormat::newWebEngineView()
{
    QObject *viewInstance = m_component->create();
    QQuickWebEngineView *webEngineView = qobject_cast<QQuickWebEngineView*>(viewInstance);
    return webEngineView;
}

inline QQuickWebEngineView *tst_QQuickWebEngineDefaultSurfaceFormat::webEngineView() const
{
    return static_cast<QQuickWebEngineView*>(m_window->webEngineView.data());
}

QUrl tst_QQuickWebEngineDefaultSurfaceFormat::urlFromTestPath(const char *localFilePath)
{
    QString testSourceDirPath = QDir(QT_TESTCASE_SOURCEDIR).canonicalPath();
    if (!testSourceDirPath.endsWith(QLatin1Char('/')))
        testSourceDirPath.append(QLatin1Char('/'));

    return QUrl::fromLocalFile(testSourceDirPath + QString::fromUtf8(localFilePath));
}

void tst_QQuickWebEngineDefaultSurfaceFormat::customDefaultSurfaceFormat()
{
#if !defined(Q_OS_MACOS)
    QSKIP("OpenGL Core Profile is currently only supported on macOS.");
#endif
    // Setting a new default QSurfaceFormat with a core OpenGL profile, before
    // app instantiation should succeed, without abort() being called.
    int argc = 1;
    char *argv[] = { const_cast<char*>("tst_QQuickWebEngineDefaultSurfaceFormat") };

    QSurfaceFormat format;
    format.setVersion( 3, 3 );
    format.setProfile( QSurfaceFormat::CoreProfile );
    QSurfaceFormat::setDefaultFormat( format );

    QGuiApplication app(argc, argv);
    QtWebEngineQuick::initialize();

    initEngineAndViewComponent();
    initWindow();
    QQuickWebEngineView* view = webEngineView();
    view->setUrl(urlFromTestPath("html/basic_page.html"));
    m_window->show();

    QObject::connect(
        view,
        &QQuickWebEngineView::loadingChanged, [](const QWebEngineLoadingInfo &info)
        {
            if (info.status() == QWebEngineLoadingInfo::LoadSucceededStatus
               || info.status() == QWebEngineLoadingInfo::LoadFailedStatus)
                QTimer::singleShot(100, qApp, &QCoreApplication::quit);
        }
    );

    QObject::connect(qApp, &QCoreApplication::aboutToQuit, [this]() {
        this->deleteWindow();
    });

    QCOMPARE(app.exec(), 0);
}

QTEST_APPLESS_MAIN(tst_QQuickWebEngineDefaultSurfaceFormat)
#include "tst_qquickwebenginedefaultsurfaceformat.moc"
