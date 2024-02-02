// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QScopedPointer>
#include <QtQml/QQmlEngine>
#include <QtTest/QtTest>
#include <QQuickWebEngineProfile>
#include <QtWebEngineQuick/private/qquickwebengineview_p.h>
#include <QWebEnginePage>

#define INSPECTOR_SERVER_PORT "23654"
static const QUrl s_inspectorServerHttpBaseUrl("http://localhost:" INSPECTOR_SERVER_PORT);

class tst_InspectorServer : public QObject {
    Q_OBJECT
public:
    tst_InspectorServer();

private Q_SLOTS:
    void init();
    void cleanup();

    void testDevToolsId();
    void testPageList();
    void testRemoteDebuggingMessage();
    void openRemoteDebuggingSession();
private:
    void prepareWebViewComponent();
    inline QQuickWebEngineView* newWebView();
    inline QQuickWebEngineView* webView() const;
    QJsonArray fetchPageList() const;
    QScopedPointer<QQuickWebEngineView> m_webView;
    QScopedPointer<QQmlComponent> m_component;
};

tst_InspectorServer::tst_InspectorServer()
{
    qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--remote-allow-origins=*");
    qputenv("QTWEBENGINE_REMOTE_DEBUGGING", INSPECTOR_SERVER_PORT);
    QtWebEngineQuick::initialize();
    QQuickWebEngineProfile::defaultProfile()->setOffTheRecord(true);
    prepareWebViewComponent();
}

void tst_InspectorServer::prepareWebViewComponent()
{
    static QQmlEngine* engine = new QQmlEngine(this);
    m_component.reset(new QQmlComponent(engine, this));

    m_component->setData(QByteArrayLiteral("import QtQuick\n"
                                           "import QtWebEngine\n"
                                           "WebEngineView { }")
                        , QUrl());
}

QQuickWebEngineView* tst_InspectorServer::newWebView()
{
    QObject* viewInstance = m_component->create();

    return qobject_cast<QQuickWebEngineView*>(viewInstance);
}

void tst_InspectorServer::init()
{
    m_webView.reset(newWebView());
}

void tst_InspectorServer::cleanup()
{
    m_webView.reset();
}

inline QQuickWebEngineView* tst_InspectorServer::webView() const
{
    return m_webView.data();
}

QJsonArray tst_InspectorServer::fetchPageList() const
{
    QNetworkAccessManager qnam;
    QSignalSpy spy(&qnam, &QNetworkAccessManager::finished);;
    QNetworkRequest request(s_inspectorServerHttpBaseUrl.resolved(QUrl("json/list")));
    QScopedPointer<QNetworkReply> reply(qnam.get(request));
    spy.wait();
    // Work-around a network bug in Qt6:
    if (reply->error() == QNetworkReply::ContentNotFoundError) {
        reply.reset(qnam.get(request));
        spy.wait();
    }
    return QJsonDocument::fromJson(reply->readAll()).array();
}

void tst_InspectorServer::testDevToolsId()
{
    const QUrl testPageUrl = QUrl::fromLocalFile(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()
                                                 + QLatin1String("/html/basic_page.html"));
    QSignalSpy loadSpy(webView(), SIGNAL(loadingChanged(QWebEngineLoadingInfo)));
    webView()->setUrl(testPageUrl);
    QTRY_VERIFY_WITH_TIMEOUT(loadSpy.size() && !webView()->isLoading(), 10000);

    // Our page should be the only one in the list.
    QJsonArray pageList = fetchPageList();
    QCOMPARE(pageList.size(), 1);
    QCOMPARE(testPageUrl.toString(), pageList.at(0).toObject().value("url").toString());
    QCOMPARE(webView()->devToolsId(), pageList.at(0).toObject().value("id").toString());
}

void tst_InspectorServer::testPageList()
{
    const QUrl testPageUrl = QUrl::fromLocalFile(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()
                                                 + QLatin1String("/html/basic_page.html"));
    QSignalSpy loadSpy(webView(), SIGNAL(loadingChanged(QWebEngineLoadingInfo)));
    webView()->setUrl(testPageUrl);
    QTRY_VERIFY_WITH_TIMEOUT(loadSpy.size() && !webView()->isLoading(), 10000);

    // Our page has developerExtrasEnabled and should be the only one in the list.
    QJsonArray pageList = fetchPageList();
    QCOMPARE(pageList.size(), 1);
    QCOMPARE(testPageUrl.toString(), pageList.at(0).toObject().value("url").toString());
}

void tst_InspectorServer::testRemoteDebuggingMessage()
{
    const QUrl testPageUrl = QUrl::fromLocalFile(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()
                                                 + QLatin1String("/html/basic_page.html"));
    QSignalSpy loadSpy(webView(), SIGNAL(loadingChanged(QWebEngineLoadingInfo)));
    webView()->setUrl(testPageUrl);
    QTRY_VERIFY_WITH_TIMEOUT(loadSpy.size() && !webView()->isLoading(), 10000);

    QJsonArray pageList = fetchPageList();
    QCOMPARE(pageList.size(), 1);
    QVERIFY(pageList.at(0).toObject().contains("webSocketDebuggerUrl"));

    // Test sending a raw remote debugging message through our web socket server.
    // For this specific message see: http://code.google.com/chrome/devtools/docs/protocol/tot/runtime.html#command-evaluate
    QLatin1String jsExpression("2 + 2");
    QLatin1String jsExpressionResult("4");
    QScopedPointer<QQuickWebEngineView> webSocketQueryWebView(newWebView());
    webSocketQueryWebView->loadHtml(QString(
        "<script type=\"text/javascript\">\n"
        "var socket = new WebSocket('%1');\n"
        "socket.onmessage = function(message) {\n"
            "var response = JSON.parse(message.data);\n"
            "if (response.id === 1)\n"
                "document.title = response.result.result.value;\n"
        "}\n"
        "socket.onopen = function() {\n"
            "socket.send('{\"id\": 1, \"method\": \"Runtime.evaluate\", \"params\": {\"expression\": \"%2\" } }');\n"
        "}\n"
        "</script>")
        .arg(pageList.at(0).toObject().value("webSocketDebuggerUrl").toString())
        .arg(jsExpression));

    QTRY_COMPARE_WITH_TIMEOUT(webSocketQueryWebView->title(), jsExpressionResult, 10000);
}

void tst_InspectorServer::openRemoteDebuggingSession()
{
    const QUrl testPageUrl = QUrl::fromLocalFile(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()
                                                 + QLatin1String("/html/basic_page.html"));
    QSignalSpy loadSpy(webView(), SIGNAL(loadingChanged(QWebEngineLoadingInfo)));
    webView()->setUrl(testPageUrl);
    QTRY_VERIFY_WITH_TIMEOUT(loadSpy.size() && !webView()->isLoading(), 10000);

    QJsonArray pageList = fetchPageList();
    QCOMPARE(pageList.size(), 1);
    QVERIFY(pageList.at(0).toObject().contains("devtoolsFrontendUrl"));

    QScopedPointer<QQuickWebEngineView> inspectorWebView(newWebView());
    inspectorWebView->setUrl(s_inspectorServerHttpBaseUrl.resolved(QUrl(pageList.at(0).toObject().value("devtoolsFrontendUrl").toString())));

    // To test the whole pipeline this exploits a behavior of the inspector front-end which won't provide any title unless the
    // debugging session was established correctly through web socket.
    // So this test case will fail if:
    // - The page list didn't return a valid inspector URL
    // - Or the front-end couldn't be loaded through the inspector HTTP server
    // - Or the web socket connection couldn't be established between the front-end and the page through the inspector server
    QTRY_VERIFY_WITH_TIMEOUT(inspectorWebView->title().startsWith("DevTools -"), 60000);
}

QTEST_MAIN(tst_InspectorServer)

#include "tst_inspectorserver.moc"
