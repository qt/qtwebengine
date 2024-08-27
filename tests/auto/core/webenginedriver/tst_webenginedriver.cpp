// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QtCore/qjsondocument.h>
#include <QtCore/qlibraryinfo.h>
#include <QtCore/qobject.h>
#include <QtCore/qprocess.h>
#include <QtGui/qimage.h>
#include <QtNetwork/qnetworkaccessmanager.h>
#include <QtNetwork/qnetworkreply.h>
#include <QtNetwork/qnetworkrequest.h>
#include <QtWebEngineCore/qtwebenginecoreglobal.h>
#include <QtWebEngineCore/qwebenginepage.h>
#include <QtWebEngineCore/qwebengineprofile.h>
#include <QtWebEngineWidgets/qwebengineview.h>

#include <widgetutil.h>

#define REMOTE_DEBUGGING_PORT 12345
#define WEBENGINEDRIVER_PORT 9515

class DriverServer : public QObject
{
    Q_OBJECT

public:
    DriverServer(QProcessEnvironment processEnvironment = {}, QStringList processArguments = {})
        : m_serverURL(QUrl(
                QLatin1String("http://localhost:%1").arg(QString::number(WEBENGINEDRIVER_PORT))))
    {
        QString driverPath = QLibraryInfo::path(QLibraryInfo::LibraryExecutablesPath)
                + QLatin1String("/webenginedriver");
#if defined(Q_OS_WIN)
        driverPath += QLatin1String(".exe");
#endif
        m_process.setProgram(driverPath);

        if (processArguments.isEmpty())
            processArguments
                    << QLatin1String("--port=%1").arg(QString::number(WEBENGINEDRIVER_PORT));
        m_process.setArguments(processArguments);

        if (!processEnvironment.isEmpty())
            m_process.setProcessEnvironment(processEnvironment);

        connect(&m_process, &QProcess::errorOccurred, [this](QProcess::ProcessError error) {
            qWarning() << "WebEngineDriver error occurred:" << error;
            dumpConsoleMessages();
        });

        connect(&m_process, &QProcess::readyReadStandardError, [this]() {
            QProcess::ProcessChannel tmp = m_process.readChannel();
            m_process.setReadChannel(QProcess::StandardError);
            while (m_process.canReadLine()) {
                QString line = QString::fromUtf8(m_process.readLine());
                if (line.endsWith(QLatin1Char('\n')))
                    line.truncate(line.size() - 1);
                m_stderr << line;
            }
            m_process.setReadChannel(tmp);
        });

        connect(&m_process, &QProcess::readyReadStandardOutput, [this]() {
            QProcess::ProcessChannel tmp = m_process.readChannel();
            m_process.setReadChannel(QProcess::StandardOutput);
            while (m_process.canReadLine()) {
                QString line = QString::fromUtf8(m_process.readLine());
                if (line.endsWith(QLatin1Char('\n')))
                    line.truncate(line.size() - 1);
                m_stdout << line;
            }
            m_process.setReadChannel(tmp);
        });
    }

    ~DriverServer()
    {
        if (m_process.state() != QProcess::Running)
            return;

        if (!m_sessionId.isEmpty())
            deleteSession();

        shutdown();
    }

    bool start()
    {
        if (!QFileInfo::exists(m_process.program())) {
            qWarning() << "WebEngineDriver executable not found:" << m_process.program();
            return false;
        }

        connect(&m_process, &QProcess::finished,
                [this](int exitCode, QProcess::ExitStatus exitStatus) {
                    qWarning().nospace()
                            << "WebEngineDriver exited unexpectedly (exitCode: " << exitCode
                            << " exitStatus: " << exitStatus << "):";
                    dumpConsoleMessages();
                });

        m_process.start();

        bool started = m_process.waitForStarted();
        if (!started) {
            qWarning() << "Failed to start WebEngineDriver:" << m_process.errorString();
            return false;
        }

        bool ready = QTest::qWaitFor(
                [this]() {
                    if (m_process.state() != QProcess::Running)
                        return true;

                    for (QString line : m_stdout) {
                        if (line.contains(
                                    QLatin1String("WebEngineDriver was started successfully.")))
                            return true;
                    }

                    return false;
                },
                10000);

        if (ready && m_process.state() != QProcess::Running) {
            // Warning is already reported by handler of QProcess::finished() signal.
            return false;
        }

        if (!ready) {
            if (m_stdout.empty())
                qWarning("Starting WebEngineDriver timed out.");
            else
                qWarning("Something went wrong while starting WebEngineDriver:");

            dumpConsoleMessages();
            return false;
        }

        return true;
    }

    bool shutdown()
    {
        // Do not warn about unexpected exit.
        disconnect(&m_process, &QProcess::finished, nullptr, nullptr);

        bool sent = sendCommand(QLatin1String("/shutdown"));

        bool finished = (m_process.state() == QProcess::NotRunning) || m_process.waitForFinished();
        if (!finished || !sent)
            qWarning() << "Failed to properly shutdown WebEngineDriver:" << m_process.errorString();

        return finished;
    }

    bool sendCommand(QString command, const QJsonDocument &params = {},
                     QJsonDocument *result = nullptr)
    {
        if (command.contains(QLatin1String(":sessionId"))) {
            if (m_sessionId.isEmpty()) {
                qWarning("Unable to execute session command without session.");
                return false;
            }

            QStringList commandList = command.split(QLatin1Char('/'));
            for (int i = 0; i < commandList.size(); ++i) {
                if (commandList[i] == QLatin1String(":sessionId")) {
                    commandList[i] = m_sessionId;
                    break;
                }
            }

            command = commandList.join(QLatin1Char('/'));
        }

        QNetworkReply::NetworkError replyError = QNetworkReply::NoError;
        QString replyString;

        connect(
                &m_qnam, &QNetworkAccessManager::finished, this,
                [&replyError, &replyString](QNetworkReply *reply) {
                    replyError = reply->error();
                    replyString = QString::fromUtf8(reply->readAll());
                },
                static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));

        QNetworkRequest request;
        QUrl requestURL = m_serverURL;

        requestURL.setPath(command);
        request.setUrl(requestURL);

        if (params.isEmpty()) {
            m_qnam.get(request);
        } else {
            request.setHeader(QNetworkRequest::ContentTypeHeader,
                              QVariant::fromValue(QStringLiteral("application/json")));
            m_qnam.post(request, params.toJson(QJsonDocument::Compact));
        }

        bool ready = QTest::qWaitFor(
                [&replyError, &replyString]() {
                    return replyError != QNetworkReply::NoError || !replyString.isEmpty();
                },
                10000);

        if (!ready) {
            qWarning() << "Command" << command << "timed out.";
            dumpConsoleMessages();
            return false;
        }

        if (replyError != QNetworkReply::NoError) {
            qWarning() << "Network error:" << replyError;
            if (!replyString.isEmpty()) {
                QJsonDocument errorReply = QJsonDocument::fromJson(replyString.toLatin1());
                if (!errorReply.isNull()) {
                    QString error = errorReply["value"]["error"].toString();
                    QString message = errorReply["value"]["message"].toString();
                    if (!error.isEmpty() || message.isEmpty()) {
                        qWarning() << "error:" << error;
                        qWarning() << "message:" << message;
                        return false;
                    }
                }

                qWarning() << replyString;
                return false;
            }

            dumpConsoleMessages();
            return false;
        }

        if (result) {
            if (replyString.isEmpty()) {
                qWarning("Network reply is empty.");
                return false;
            }

            QJsonParseError jsonError;
            *result = QJsonDocument::fromJson(replyString.toLatin1(), &jsonError);

            if (jsonError.error != QJsonParseError::NoError) {
                qWarning() << "Unable to parse reply:" << jsonError.errorString();
                return false;
            }
        }

        return true;
    }

    bool createSession(QJsonObject chromeOptions = {}, QString *sessionId = nullptr)
    {
        if (!m_sessionId.isEmpty()) {
            qWarning("A session already exists.");
            return false;
        }

        QJsonObject root;

        if (chromeOptions.isEmpty()) {
            // Connect to the test by default.
            chromeOptions.insert(
                    QLatin1String("debuggerAddress"),
                    QLatin1String("localhost:%1").arg(QString::number(REMOTE_DEBUGGING_PORT)));
            chromeOptions.insert(QLatin1String("w3c"), true);
        }

        QJsonObject alwaysMatch;
        alwaysMatch.insert(QLatin1String("goog:chromeOptions"), chromeOptions);

        QJsonObject seOptions;
        seOptions.insert(QLatin1String("loggingPrefs"), QJsonObject());
        alwaysMatch.insert(QLatin1String("se:options"), seOptions);

        QJsonObject capabilities;
        capabilities.insert(QLatin1String("alwaysMatch"), alwaysMatch);
        root.insert(QLatin1String("capabilities"), capabilities);

        QJsonDocument params;
        params.setObject(root);

        QJsonDocument sessionReply;
        bool sent = sendCommand(QLatin1String("/session"), params, &sessionReply);
        if (sent) {
            m_sessionId = sessionReply["value"]["sessionId"].toString();
            if (sessionId)
                *sessionId = m_sessionId;
        }

        return sent;
    }

    bool deleteSession()
    {
        if (m_sessionId.isEmpty()) {
            qWarning("There is no active session.");
            return false;
        }

        QNetworkReply::NetworkError replyError = QNetworkReply::NoError;
        QString replyString;

        connect(
                &m_qnam, &QNetworkAccessManager::finished, this,
                [&replyError, &replyString](QNetworkReply *reply) {
                    replyError = reply->error();
                    replyString = QString::fromUtf8(reply->readAll());
                },
                static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));

        QNetworkRequest request;
        QUrl requestURL = m_serverURL;
        requestURL.setPath(QLatin1String("/session/%1").arg(m_sessionId));
        request.setUrl(requestURL);
        m_qnam.deleteResource(request);

        bool ready = QTest::qWaitFor(
                [&replyError, &replyString]() {
                    return replyError != QNetworkReply::NoError || !replyString.isEmpty();
                },
                10000);

        if (!ready) {
            qWarning("Deleting session timed out.");
            return false;
        }

        if (replyError != QNetworkReply::NoError) {
            qWarning() << "Network error:" << replyError;
            return false;
        }

        return true;
    }

    QStringList stderrLines() const { return m_stderr; }

    bool waitForMessageOnStderr(const QRegularExpression &re, QString *message = nullptr) const
    {
        return QTest::qWaitFor(
                [this, &re, message]() {
                    for (QString line : m_stderr) {
                        if (line.contains(re)) {
                            if (message)
                                *message = line;
                            return true;
                        }
                    }

                    return false;
                },
                10000);
    }

private:
    void dumpConsoleMessages()
    {
        auto dumpLines = [](QStringList *lines) {
            if (lines->empty())
                return;

            for (QString line : *lines)
                qWarning() << qPrintable(line);

            lines->clear();
        };

        dumpLines(&m_stdout);
        dumpLines(&m_stderr);
    }

    QProcess m_process;
    QStringList m_stdout;
    QStringList m_stderr;

    QUrl m_serverURL;
    QString m_sessionId;

    QNetworkAccessManager m_qnam;
};

class tst_WebEngineDriver : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void status();
    void startBrowser();
    void navigate();
    void typeElement();
    void executeScript();
    void screenshot();
};

void tst_WebEngineDriver::status()
{
    DriverServer driverServer;
    QVERIFY(driverServer.start());

    QJsonDocument statusReply;
    QVERIFY(driverServer.sendCommand(QLatin1String("/status"), {}, &statusReply));

    QString versionString = statusReply["value"]["build"]["version"].toString();
    QCOMPARE(qWebEngineChromiumVersion(), versionString.split(QLatin1Char(' '))[0]);

    bool ready = statusReply["value"]["ready"].toBool();
    QVERIFY(ready);
}

void tst_WebEngineDriver::startBrowser()
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert(QLatin1String("QTWEBENGINE_REMOTE_DEBUGGING"),
               QString::number(REMOTE_DEBUGGING_PORT));

    QStringList args;
    args << QLatin1String("--port=%1").arg(QString::number(WEBENGINEDRIVER_PORT));
    args << QLatin1String("--log-level=ALL");

    DriverServer driverServer(env, args);
    QVERIFY(driverServer.start());

    QString testBrowserPath =
            QCoreApplication::applicationDirPath() + QLatin1String("/testbrowser");
#if defined(Q_OS_WIN)
    testBrowserPath += QLatin1String(".exe");
#endif

    QJsonArray chromeArgs;
    chromeArgs.append(QLatin1String("enable-logging=stderr"));
    chromeArgs.append(QLatin1String("v=1"));
    // To force graceful shutdown.
    chromeArgs.append(QLatin1String("log-net-log"));

    QJsonObject chromeOptions;
    chromeOptions.insert(QLatin1String("binary"), testBrowserPath);
    chromeOptions.insert(QLatin1String("args"), chromeArgs);
    chromeOptions.insert(QLatin1String("w3c"), true);
    QString sessionId;
    QVERIFY(driverServer.createSession(chromeOptions, &sessionId));

    // Test if the browser is started.
    QVERIFY(driverServer.waitForMessageOnStderr(QRegularExpression(
            QLatin1String("^DevTools listening on ws://127.0.0.1:%1/devtools/browser/")
                    .arg(QString::number(REMOTE_DEBUGGING_PORT)))));
    QVERIFY(driverServer.waitForMessageOnStderr(QRegularExpression(
            QLatin1String("^Remote debugging server started successfully. "
                          "Try pointing a Chromium-based browser to http://127.0.0.1:%1")
                    .arg(QString::number(REMOTE_DEBUGGING_PORT)))));

    // Check custom chromeArgs via logging.
    QVERIFY(driverServer.waitForMessageOnStderr(QRegularExpression(QLatin1String("VERBOSE1"))));

    QJsonDocument statusReply;
    QVERIFY(driverServer.sendCommand(QLatin1String("/status"), {}, &statusReply));
    bool ready = statusReply["value"]["ready"].toBool();
    QVERIFY(ready);

    QVERIFY(driverServer.deleteSession());

    // Test if the browser is stopped.
    QVERIFY(driverServer.waitForMessageOnStderr(
            QRegularExpression(QLatin1String("Test browser is about to quit."))));
    QVERIFY(driverServer.waitForMessageOnStderr(
            QRegularExpression(QLatin1String("\\[%1\\] RESPONSE Quit").arg(sessionId))));
}

void tst_WebEngineDriver::navigate()
{
    DriverServer driverServer;
    QVERIFY(driverServer.start());

    QWebEnginePage page;
    QSignalSpy loadSpy(&page, &QWebEnginePage::loadFinished);

    page.load(QUrl(QLatin1String("about:blank")));
    QTRY_COMPARE(loadSpy.size(), 1);

    QVERIFY(driverServer.createSession());

    QUrl url = QUrl(QLatin1String("qrc:///resources/input.html"));
    QString paramsString = QLatin1String("{\"url\": \"%1\"}").arg(url.toString());
    QJsonDocument params = QJsonDocument::fromJson(paramsString.toUtf8());
    QVERIFY(driverServer.sendCommand(QLatin1String("/session/:sessionId/url"), params));
    QTRY_COMPARE(loadSpy.size(), 2);
    QVERIFY(loadSpy.at(1).at(0).toBool());
    QCOMPARE(url, page.url());

    QVERIFY(driverServer.deleteSession());
}

void tst_WebEngineDriver::typeElement()
{
    DriverServer driverServer;
    QVERIFY(driverServer.start());

    QWebEngineView view;
    view.resize(300, 100);
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    QUrl url = QUrl(QLatin1String("qrc:///resources/input.html"));
    QSignalSpy loadSpy(view.page(), &QWebEnginePage::loadFinished);
    view.load(url);
    QTRY_COMPARE(loadSpy.size(), 1);

    QVERIFY(driverServer.createSession());

    // Find <input id="text_input"> element and extract its id from the reply.
    QString textInputId;
    {
        QString paramsString = QLatin1String(
                "{\"using\": \"css selector\", \"value\": \"[id=\\\"text_input\\\"]\"}");
        QJsonDocument params = QJsonDocument::fromJson(paramsString.toUtf8());
        QJsonDocument elementReply;
        QVERIFY(driverServer.sendCommand(QLatin1String("/session/:sessionId/element"), params,
                                         &elementReply));

        QVERIFY(!elementReply.isEmpty());
        QJsonObject value = elementReply["value"].toObject();
        QVERIFY(!value.isEmpty());
        QStringList keys = value.keys();
        QCOMPARE(keys.size(), 1);
        textInputId = value[keys[0]].toString();
        QVERIFY(!textInputId.isEmpty());
    }

    // Type text into the input field.
    QString inputText = QLatin1String("WebEngineDriver");
    {
        QString command = QLatin1String("/session/:sessionId/element/%1/value").arg(textInputId);

        QJsonObject root;
        root.insert(QLatin1String("text"), inputText);
        QJsonArray value;
        for (QChar ch : inputText) {
            value.append(QJsonValue(ch));
        }
        root.insert(QLatin1String("value"), value);
        root.insert(QLatin1String("id"), textInputId);
        QJsonDocument params;
        params.setObject(root);

        QVERIFY(driverServer.sendCommand(command, params));

        QTRY_COMPARE(
                evaluateJavaScriptSync(view.page(), "document.getElementById('text_input').value")
                        .toString(),
                inputText);
    }

    QVERIFY(driverServer.deleteSession());
}

void tst_WebEngineDriver::executeScript()
{
    DriverServer driverServer;
    QVERIFY(driverServer.start());

    QUrl url = QUrl(QLatin1String("qrc:///resources/input.html"));
    QWebEnginePage page;
    QSignalSpy loadSpy(&page, &QWebEnginePage::loadFinished);

    page.load(url);
    QTRY_COMPARE(loadSpy.size(), 1);
    QCOMPARE(page.title(), url.toString());
    QSignalSpy titleSpy(&page, &QWebEnginePage::titleChanged);

    QString newTitle = QLatin1String("WebEngineDriver Test Page");
    QString script = QLatin1String("document.title = '%1';"
                                   "return document.title;")
                             .arg(newTitle);

    QVERIFY(driverServer.createSession());
    QString paramsString = QLatin1String("{\"script\": \"%1\", \"args\": []}").arg(script);
    QJsonDocument params = QJsonDocument::fromJson(paramsString.toUtf8());
    QJsonDocument executeReply;
    QVERIFY(driverServer.sendCommand(QLatin1String("/session/:sessionId/execute/sync"), params,
                                     &executeReply));

    QTRY_COMPARE(titleSpy.size(), 1);
    QCOMPARE(executeReply["value"].toString(), newTitle);
    QCOMPARE(page.title(), newTitle);

    QVERIFY(driverServer.deleteSession());
}

void tst_WebEngineDriver::screenshot()
{
    DriverServer driverServer;
    QVERIFY(driverServer.start());

    QWebEngineView view;
    view.resize(300, 100);
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    QSignalSpy loadSpy(view.page(), &QWebEnginePage::loadFinished);

    view.setHtml(QLatin1String("<html><head><style>"
                               "html {background-color:red;}"
                               "</style></head><body></body></html>"));
    QTRY_COMPARE(loadSpy.size(), 1);

    QVERIFY(driverServer.createSession());
    QJsonDocument screenshotReply;
    QVERIFY(driverServer.sendCommand(QLatin1String("/session/:sessionId/screenshot"), {},
                                     &screenshotReply));

    QByteArray base64 = screenshotReply["value"].toString().toLocal8Bit();
    QVERIFY(!base64.isEmpty());
    QImage screenshot;
    screenshot.loadFromData(QByteArray::fromBase64(base64));
    QVERIFY(!screenshot.isNull());
    QCOMPARE(screenshot.pixel(screenshot.width() / 2, screenshot.height() / 2), 0xFFFF0000);

    QVERIFY(driverServer.deleteSession());
}

#define STRINGIFY_LITERAL(x) #x
#define STRINGIFY_EXPANDED(x) STRINGIFY_LITERAL(x)
static QByteArrayList params = QByteArrayList()
        << "--remote-debugging-port=" STRINGIFY_EXPANDED(REMOTE_DEBUGGING_PORT);
W_QTEST_MAIN(tst_WebEngineDriver, params)

#include "tst_webenginedriver.moc"
