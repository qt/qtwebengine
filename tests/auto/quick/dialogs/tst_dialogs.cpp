/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "testhandler.h"
#include "server.h"
#include "util.h"

#include <QtWebEngine/private/qquickwebenginedialogrequests_p.h>
#include <QtWebEngine/private/qquickwebenginecontextmenurequest_p.h>
#include <QQuickWebEngineProfile>

#include <QNetworkProxy>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QSignalSpy>
#include <QTest>

class tst_Dialogs : public QObject
{
    Q_OBJECT
public:
    tst_Dialogs(){}

private slots:
    void initTestCase();
    void init();
    void contextMenuRequested();
    void javaScriptDialogRequested();
    void javaScriptDialogRequested_data();
    void colorDialogRequested();
    void fileDialogRequested();
    void authenticationDialogRequested_data();
    void authenticationDialogRequested();

private:
    void createDialog(const QLatin1String &dialog, bool &ok);
private:
    QScopedPointer<QQmlApplicationEngine> m_engine;
    QQuickWindow *m_window;
    TestHandler *m_listener;
};

void tst_Dialogs::initTestCase()
{
    QQuickWebEngineProfile::defaultProfile()->setOffTheRecord(true);
    qmlRegisterType<TestHandler>("io.qt.tester", 1, 0, "TestHandler");
    m_engine.reset(new QQmlApplicationEngine());
    m_engine->load(QUrl(QStringLiteral("qrc:/WebView.qml")));
    m_window = qobject_cast<QQuickWindow*>(m_engine->rootObjects().first());
    Q_ASSERT(m_window);
    m_listener = m_window->findChild<TestHandler*>(QStringLiteral("TestListener"));
    Q_ASSERT(m_listener);

    QNetworkProxy proxy;
    proxy.setType(QNetworkProxy::HttpProxy);
    proxy.setHostName("localhost");
    proxy.setPort(5555);
    QNetworkProxy::setApplicationProxy(proxy);
}

void tst_Dialogs::init()
{
     m_listener->setRequest(nullptr);
     m_listener->setReady(false);
}

void tst_Dialogs::createDialog(const QLatin1String &dialog, bool &ok)
{
    QString trigger = QStringLiteral("document.getElementById('buttonOne').onclick = function() {document.getElementById('%1').click()}");
    QSignalSpy dialogSpy(m_listener, &TestHandler::requestChanged);
    m_listener->runJavaScript(trigger.arg(dialog));
    QTRY_VERIFY(m_listener->ready());
    QTest::mouseClick(m_window, Qt::LeftButton);
    QTRY_COMPARE(dialogSpy.count(), 1);
    ok = true;
}

void tst_Dialogs::colorDialogRequested()
{
    m_listener->load(QUrl("qrc:/index.html"));
    QTRY_VERIFY(m_listener->ready());
    bool ok = false;
    createDialog(QLatin1String("colorpicker"), ok);
    if (ok) {
        auto *dialog = qobject_cast<QQuickWebEngineColorDialogRequest*>(m_listener->request());
        QVERIFY2(dialog, "Incorrect dialog requested");
        dialog->dialogReject();
        QVERIFY2(dialog->isAccepted(), "Dialog is not accepted");
        QCOMPARE(dialog->color(), QColor("#ff0000"));
    }
}

void tst_Dialogs::contextMenuRequested()
{
    m_listener->load(QUrl("qrc:/index.html"));
    QTRY_COMPARE_WITH_TIMEOUT(m_listener->ready(), true, 20000);
    QSignalSpy dialogSpy(m_listener, &TestHandler::requestChanged);
    QTest::mouseClick(m_window, Qt::RightButton);
    QTRY_COMPARE(dialogSpy.count(), 1);
    auto dialog = qobject_cast<QQuickWebEngineContextMenuRequest*>(m_listener->request());
    QVERIFY2(dialog, "Incorrect dialog requested");
}

void tst_Dialogs::fileDialogRequested()
{
    m_listener->load(QUrl("qrc:/index.html"));
    QTRY_VERIFY(m_listener->ready());
    bool ok = false;
    createDialog(QLatin1String("filepicker"), ok);
    if (ok) {
        auto dialog = qobject_cast<QQuickWebEngineFileDialogRequest*>(m_listener->request());
        QVERIFY2(dialog, "Incorrect dialog requested");
        dialog->dialogReject();
        QVERIFY2(dialog->isAccepted(), "Dialog is not accepted");
        QStringList mimeTypes {".cpp", ".html", ".h" , ".png", ".qdoc", ".qml"};
        QCOMPARE(dialog->acceptedMimeTypes(), mimeTypes);
    }
}

void tst_Dialogs::authenticationDialogRequested_data()
{
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<QQuickWebEngineAuthenticationDialogRequest::AuthenticationType>("type");
    QTest::addColumn<QString>("realm");
    QTest::addColumn<QByteArray>("reply");
    QTest::newRow("Http Authentication Dialog") << QUrl("http://localhost:5555/")
                                                << QQuickWebEngineAuthenticationDialogRequest::AuthenticationTypeHTTP
                                                << QStringLiteral("Very Restricted Area")
                                                << QByteArrayLiteral("HTTP/1.1 401 Unauthorized\nWWW-Authenticate: "
                                                                     "Basic realm=\"Very Restricted Area\"\r\n\r\n");
    QTest::newRow("Proxy Authentication Dialog")<< QUrl("http://qt.io/")
                                                << QQuickWebEngineAuthenticationDialogRequest::AuthenticationTypeProxy
                                                << QStringLiteral("Proxy requires authentication")
                                                << QByteArrayLiteral("HTTP/1.1 407 Proxy Auth Required\nProxy-Authenticate: "
                                                                "Basic realm=\"Proxy requires authentication\"\r\n"
                                                                "content-length: 0\r\n\r\n");

}

void tst_Dialogs::authenticationDialogRequested()
{
    QFETCH(QUrl, url);
    QFETCH(QQuickWebEngineAuthenticationDialogRequest::AuthenticationType, type);
    QFETCH(QString, realm);

    QFETCH(QByteArray, reply);
    Server server;
    server.setReply(reply);
    server.run();
    QTRY_VERIFY2(server.isListening(), "Could not setup authentication server");

    QSignalSpy dialogSpy(m_listener, &TestHandler::requestChanged);
    m_listener->load(url);

    QTRY_COMPARE(dialogSpy.count(), 1);
    auto *dialog = qobject_cast<QQuickWebEngineAuthenticationDialogRequest*>(m_listener->request());
    QVERIFY2(dialog, "Incorrect dialog requested");
    dialog->dialogReject();
    QVERIFY2(dialog->isAccepted(), "Dialog is not accepted");
    QCOMPARE(dialog->type(), type);
    QCOMPARE(dialog->realm(),realm);
    QCOMPARE(dialog->url(), url);
    QCOMPARE(dialog->proxyHost(), QStringLiteral("localhost"));
}

void tst_Dialogs::javaScriptDialogRequested_data()
{
    QTest::addColumn<QString>("script");
    QTest::addColumn<QQuickWebEngineJavaScriptDialogRequest::DialogType>("type");
    QTest::addColumn<QString>("message");
    QTest::addColumn<QString>("defaultText");
    QTest::newRow("AlertDialog") << QStringLiteral("alert('This is the Alert Dialog !')")
                                 << QQuickWebEngineJavaScriptDialogRequest::DialogTypeAlert
                                 << QStringLiteral("This is the Alert Dialog !")
                                 << QString();
    QTest::newRow("PromptDialog")<< QStringLiteral("prompt('Is this the Prompt Dialog ?', 'Yes')")
                                 << QQuickWebEngineJavaScriptDialogRequest::DialogTypePrompt
                                 << QStringLiteral("Is this the Prompt Dialog ?")
                                 << QStringLiteral("Yes");
    QTest::newRow("ConfirmDialog")<< QStringLiteral("confirm('This is the Confirm Dialog.')")
                                 << QQuickWebEngineJavaScriptDialogRequest::DialogTypeConfirm
                                 << QStringLiteral("This is the Confirm Dialog.")
                                 << QString();
}

void tst_Dialogs::javaScriptDialogRequested()
{
    QFETCH(QString, script);
    QFETCH(QQuickWebEngineJavaScriptDialogRequest::DialogType, type);
    QFETCH(QString, message);
    QFETCH(QString, defaultText);

    m_listener->load(QUrl("qrc:/index.html"));
    QTRY_VERIFY(m_listener->ready());

    QSignalSpy dialogSpy(m_listener, &TestHandler::requestChanged);
    m_listener->runJavaScript(script);
    QTRY_COMPARE(dialogSpy.count(), 1);
    auto *dialog = qobject_cast<QQuickWebEngineJavaScriptDialogRequest*>(m_listener->request());
    QVERIFY2(dialog, "Incorrect dialog requested");
    dialog->dialogReject();
    QVERIFY2(dialog->isAccepted(), "Dialog is not accepted");
    QCOMPARE(dialog->type(), type);
    QCOMPARE(dialog->message(), message);
    QCOMPARE(dialog->defaultText(), defaultText);
    QTRY_VERIFY(m_listener->ready()); // make sure javascript executes no longer
}

static QByteArrayList params;
W_QTEST_MAIN(tst_Dialogs, params)
#include "tst_dialogs.moc"

