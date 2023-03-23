// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef UTIL_H
#define UTIL_H

#include <QEventLoop>
#include <QQmlEngine>
#include <QSignalSpy>
#include <QTimer>
#include <QtTest/QtTest>
#include <QtWebEngineCore/QWebEngineLoadingInfo>
#include <QtWebEngineQuick/private/qquickwebengineview_p.h>
#include <QGuiApplication>

#if !defined(TESTS_SOURCE_DIR)
#define TESTS_SOURCE_DIR ""
#endif

class LoadSpy : public QEventLoop {
    Q_OBJECT

public:
    LoadSpy(QQuickWebEngineView *webEngineView)
    {
        connect(webEngineView, &QQuickWebEngineView::loadingChanged, this, &LoadSpy::onLoadingChanged);
    }

    ~LoadSpy() { }

Q_SIGNALS:
    void loadSucceeded();
    void loadFailed();

private Q_SLOTS:
    void onLoadingChanged(const QWebEngineLoadingInfo &info)
    {
        if (info.status() == QWebEngineLoadingInfo::LoadSucceededStatus)
            emit loadSucceeded();
        else if (info.status() == QWebEngineLoadingInfo::LoadFailedStatus)
            emit loadFailed();
    }
};

class LoadStartedCatcher : public QObject {
    Q_OBJECT

public:
    LoadStartedCatcher(QQuickWebEngineView *webEngineView)
        : m_webEngineView(webEngineView)
    {
        connect(m_webEngineView, &QQuickWebEngineView::loadingChanged, this, &LoadStartedCatcher::onLoadingChanged);
    }

    virtual ~LoadStartedCatcher() { }

public Q_SLOTS:
    void onLoadingChanged(const QWebEngineLoadingInfo &info)
    {
        if (info.status() == QWebEngineLoadingInfo::LoadStartedStatus)
            QMetaObject::invokeMethod(this, "finished", Qt::QueuedConnection);
    }

Q_SIGNALS:
    void finished();

private:
    QQuickWebEngineView *m_webEngineView;
};

inline bool waitForLoadSucceeded(QQuickWebEngineView *webEngineView, int timeout = 10000)
{
    LoadSpy loadSpy(webEngineView);
    QSignalSpy spy(&loadSpy, &LoadSpy::loadSucceeded);
    return spy.wait(timeout);
}

inline bool waitForLoadFailed(QQuickWebEngineView *webEngineView, int timeout = 20000)
{
    LoadSpy loadSpy(webEngineView);
    QSignalSpy spy(&loadSpy, &LoadSpy::loadFailed);
    return spy.wait(timeout);
}

inline QVariant evaluateJavaScriptSync(QQuickWebEngineView *view, const QString &script)
{
    QQmlEngine *engine = qmlEngine(view);
    engine->globalObject().setProperty("called", false);
    engine->globalObject().setProperty("result", QJSValue());
    QJSValue callback = engine->evaluate(
            "(function callback(r) {"
            "   called = true;"
            "   result = r;"
            "})"
            );
    view->runJavaScript(script, callback);
    QTRY_LOOP_IMPL(engine->globalObject().property("called").toBool(), 5000, 50);
    if (!engine->globalObject().property("called").toBool()) {
        qWarning("JavaScript wasn't evaluated");
        return QVariant();
    }

    return engine->globalObject().property("result").toVariant();
}

inline QPoint elementCenter(QQuickWebEngineView *view, const QString &id)
{
    const QString jsCode(
            "(function(){"
            "   var elem = document.getElementById('" + id + "');"
            "   var rect = elem.getBoundingClientRect();"
            "   return [(rect.left + rect.right) / 2, (rect.top + rect.bottom) / 2];"
            "})()");
    QVariantList rectList = evaluateJavaScriptSync(view, jsCode).toList();

    if (rectList.size() != 2) {
        qWarning("elementCenter failed.");
        return QPoint();
    }

    return QPoint(rectList.at(0).toInt(), rectList.at(1).toInt());
}

inline QString activeElementId(QQuickWebEngineView *webEngineView)
{
    qRegisterMetaType<QQuickWebEngineView::JavaScriptConsoleMessageLevel>("JavaScriptConsoleMessageLevel");
    QSignalSpy consoleMessageSpy(webEngineView, &QQuickWebEngineView::javaScriptConsoleMessage);

    webEngineView->runJavaScript(
                "if (document.activeElement == null)"
                "   console.log('');"
                "else"
                "   console.log(document.activeElement.id);"
    );

    if (!consoleMessageSpy.wait())
        return QString();

    QList<QVariant> arguments = consoleMessageSpy.takeFirst();
    if (static_cast<QQuickWebEngineView::JavaScriptConsoleMessageLevel>(arguments.at(0).toInt()) != QQuickWebEngineView::InfoMessageLevel)
        return QString();

    return arguments.at(1).toString();
}

#define W_QTEST_MAIN(TestObject, params) \
int main(int argc, char *argv[]) \
{ \
    QtWebEngineQuick::initialize(); \
    QList<const char *> w_argv(argc); \
    QLatin1String arg("--webEngineArgs"); \
    for (int i = 0; i < argc; ++i) \
        w_argv[i] = argv[i]; \
    w_argv.append(arg.data()); \
    for (int i = 0; i < params.size(); ++i) \
        w_argv.append(params[i].data()); \
    int w_argc = w_argv.size(); \
    \
    QGuiApplication app(w_argc, const_cast<char **>(w_argv.data())); \
    app.setAttribute(Qt::AA_Use96Dpi, true); \
    TestObject tc; \
    QTEST_SET_MAIN_SOURCE_PATH \
    return QTest::qExec(&tc, argc, argv); \
}
#endif /* UTIL_H */

