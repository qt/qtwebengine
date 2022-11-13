// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "testwindow.h"
#include "quickutil.h"

#include <QScopedPointer>
#include <QtQml/QQmlEngine>
#include <QtTest/QtTest>
#include <QtWebEngineQuick/private/qquickwebengineview_p.h>
#include <httpserver.h>
#include <QNetworkProxy>
#include <QObject>

class tst_UIDelegates : public QObject
{
    Q_OBJECT
public:
    tst_UIDelegates();

private Q_SLOTS:
    void init();
    void initTestCase();
    void cleanup();
    void javaScriptDialog();
    void javaScriptDialog_data();
    void fileDialog();
    void contextMenu();
    void tooltip();
    void colorDialog();
    void authenticationDialog_data();
    void authenticationDialog();

private:
    inline QQuickWebEngineView *newWebEngineView();
    inline QQuickWebEngineView *webEngineView() const;
    void runJavaScript(const QString &script);
    QScopedPointer<TestWindow> m_window;
    QScopedPointer<QQmlComponent> m_component;
};

tst_UIDelegates::tst_UIDelegates()
{
    QtWebEngineQuick::initialize();
    static QQmlEngine *engine = new QQmlEngine(this);
    m_component.reset(new QQmlComponent(engine, this));
    m_component->setData(QByteArrayLiteral("import QtQuick\n"
                                           "import QtWebEngine\n"
                                           "WebEngineView {}"),
                         QUrl());
}

QQuickWebEngineView *tst_UIDelegates::newWebEngineView()
{
    QObject *viewInstance = m_component->create();
    QQuickWebEngineView *webEngineView = qobject_cast<QQuickWebEngineView *>(viewInstance);
    return webEngineView;
}

void tst_UIDelegates::init()
{
    m_window.reset(new TestWindow(newWebEngineView()));
}

void tst_UIDelegates::initTestCase()
{
    QNetworkProxy proxy;
    proxy.setType(QNetworkProxy::HttpProxy);
    proxy.setHostName("localhost");
    proxy.setPort(5555);
    QNetworkProxy::setApplicationProxy(proxy);
}

void tst_UIDelegates::cleanup()
{
    m_window.reset();
}

inline QQuickWebEngineView *tst_UIDelegates::webEngineView() const
{
    return static_cast<QQuickWebEngineView *>(m_window->webEngineView.data());
}

void tst_UIDelegates::runJavaScript(const QString &script)
{
    webEngineView()->runJavaScript(script);
}

void tst_UIDelegates::javaScriptDialog_data()
{
    QTest::addColumn<QString>("javaScriptCode");
    QTest::addColumn<QString>("expectedObjectName");

    QTest::newRow("AlertDialog") << QString("alert('This is the Alert Dialog!');")
                                 << QString("alertDialog");
    QTest::newRow("ConfirmDialog") << QString("confirm('This is the Confirm Dialog.');")
                                   << QString("confirmDialog");
    QTest::newRow("PromptDialog") << QString("prompt('Is this the Prompt Dialog?', 'Yes');")
                                  << QString("promptDialog");
}

void tst_UIDelegates::javaScriptDialog()
{
    QFETCH(QString, javaScriptCode);
    QFETCH(QString, expectedObjectName);

    m_window->show();
    QTRY_VERIFY(qApp->focusObject());
    QQuickWebEngineView *view = webEngineView();

    view->loadHtml("<html><body>"
                   "</body></html>");
    QVERIFY(waitForLoadSucceeded(view));

    runJavaScript(javaScriptCode);
    QTRY_VERIFY(view->findChild<QObject *>(expectedObjectName));
}

void tst_UIDelegates::fileDialog()
{
    m_window->show();
    QTRY_VERIFY(qApp->focusObject());
    QQuickWebEngineView *view = webEngineView();

    view->loadHtml("<html><body>"
                   "<input type='file' id='filePicker'/>"
                   "</body></html>");
    QVERIFY(waitForLoadSucceeded(view));

    QPoint filePickerCenter = elementCenter(view, QStringLiteral("filePicker"));
    QTest::mouseClick(view->window(), Qt::LeftButton, {}, filePickerCenter);
    QTRY_VERIFY(view->findChild<QObject *>(QStringLiteral("fileDialog")));
}

void tst_UIDelegates::contextMenu()
{
    m_window->show();
    QTRY_VERIFY(qApp->focusObject());
    QQuickWebEngineView *view = webEngineView();

    view->loadHtml("<html><body>"
                   "</body></html>");
    QVERIFY(waitForLoadSucceeded(view));

    QTest::mouseClick(view->window(), Qt::RightButton);
    QTRY_VERIFY(view->findChild<QObject *>(QStringLiteral("menu")));
}

void tst_UIDelegates::tooltip()
{
    m_window->show();
    QTRY_VERIFY(qApp->focusObject());
    QQuickWebEngineView *view = webEngineView();

    view->loadHtml("<html><body>"
                   "<p id='toolTip' title='I'm a tooltip.'>Hover this text to display a tooltip</p>"
                   "</body></html>");
    QVERIFY(waitForLoadSucceeded(view));
    QString toolTipStr = QStringLiteral("toolTip");

    QPoint tooltipCenter = elementCenter(view, toolTipStr);
    QPoint windowCenter = QPoint(view->window()->width() / 2, view->window()->height() / 2);
    QVERIFY(tooltipCenter.x() == windowCenter.x());

    int distance = windowCenter.y() - tooltipCenter.y();
    for (int i = 3; i > 0; i--) {
        QTest::mouseMove(view->window(), QPoint(windowCenter.x(), windowCenter.y() - distance / i));
    }
    QTRY_VERIFY(view->findChild<QObject *>(toolTipStr));
}

void tst_UIDelegates::colorDialog()
{
    m_window->show();
    QTRY_VERIFY(qApp->focusObject());
    QQuickWebEngineView *view = webEngineView();

    view->loadHtml("<html><body>"
                   "<input type='color' id='colorPicker'>"
                   "</body></html>");
    QVERIFY(waitForLoadSucceeded(view));

    QPoint filePickerCenter = elementCenter(view, QStringLiteral("colorPicker"));
    QTest::mouseClick(view->window(), Qt::LeftButton, {}, filePickerCenter);
    QTRY_VERIFY(view->findChild<QObject *>(QStringLiteral("colorDialog")));
}

void tst_UIDelegates::authenticationDialog_data()
{
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<QByteArray>("response");

    QTest::newRow("Http Authentication Dialog")
            << QUrl("http://localhost:5555/")
            << QByteArrayLiteral("HTTP/1.1 401 Unauthorized\nWWW-Authenticate: "
                                 "Basic realm=\"Very Restricted Area\"\r\n\r\n");
    QTest::newRow("Proxy Authentication Dialog")
            << QUrl("http://qt.io/")
            << QByteArrayLiteral("HTTP/1.1 407 Proxy Auth Required\nProxy-Authenticate: "
                                 "Basic realm=\"Proxy requires authentication\"\r\n"
                                 "content-length: 0\r\n\r\n");
}

void tst_UIDelegates::authenticationDialog()
{
    QFETCH(QUrl, url);
    QFETCH(QByteArray, response);

    HttpServer server(QHostAddress::LocalHost, 5555);
    connect(&server, &HttpServer::newRequest,
            [url, response](HttpReqRep *rr) { rr->sendResponse(response); });
    QVERIFY(server.start());

    m_window->show();
    QTRY_VERIFY(qApp->focusObject());
    QQuickWebEngineView *view = webEngineView();
    view->loadHtml("<html><body>"
                   "</body></html>");
    QVERIFY(waitForLoadSucceeded(view));

    view->setUrl(url);
    QTRY_VERIFY(view->findChild<QObject *>(QStringLiteral("authenticationDialog")));
    QVERIFY(server.stop());
}

QTEST_MAIN(tst_UIDelegates)
#include "tst_uidelegates.moc"
#include "moc_quickutil.cpp"
