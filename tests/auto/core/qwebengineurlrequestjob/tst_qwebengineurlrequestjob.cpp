// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>
#include <util.h>
#include <QtWebEngineCore/qwebengineloadinginfo.h>
#include <QtWebEngineCore/qwebengineurlschemehandler.h>
#include <QtWebEngineCore/qwebengineurlscheme.h>
#include <QtWebEngineCore/qwebengineurlrequestjob.h>
#include <QtWebEngineCore/qwebengineprofile.h>
#include <QtWebEngineCore/qwebenginepage.h>


class CustomPage : public QWebEnginePage
{
    Q_OBJECT

public:
    CustomPage(QWebEngineProfile *profile, QString compareStringPrefix, QString compareStringSuffix,
               QObject *parent = nullptr)
        : QWebEnginePage(profile, parent)
        , m_compareStringPrefix(compareStringPrefix)
        , m_compareStringSuffix(compareStringSuffix)
        , m_comparedMessageCount(0)
    {
    }

    int comparedMessageCount() const { return m_comparedMessageCount; }

signals:
    void receivedMessage();

protected:
    void javaScriptConsoleMessage(QWebEnginePage::JavaScriptConsoleMessageLevel level,
                                  const QString &message, int lineNumber,
                                  const QString &sourceID) override
    {
        Q_UNUSED(level);
        Q_UNUSED(lineNumber);
        Q_UNUSED(sourceID);

        auto splitMessage = message.split(";");
        if (splitMessage[0] == m_compareStringPrefix) {
            QCOMPARE(splitMessage[1], m_compareStringSuffix);
            m_comparedMessageCount++;
            emit receivedMessage();
        }
    }

private:
    QString m_compareStringPrefix;
    QString m_compareStringSuffix;
    int m_comparedMessageCount;
};

class AdditionalResponseHeadersHandler : public QWebEngineUrlSchemeHandler
{
    Q_OBJECT
public:
    AdditionalResponseHeadersHandler(bool addAdditionalResponseHeaders, QObject *parent = nullptr)
        : QWebEngineUrlSchemeHandler(parent)
        , m_addAdditionalResponseHeaders(addAdditionalResponseHeaders)
    {
    }

    void requestStarted(QWebEngineUrlRequestJob *job) override
    {
        QCOMPARE(job->requestUrl(), QUrl(schemeName + ":about"));

        QMultiMap<QByteArray, QByteArray> additionalResponseHeaders;
        if (m_addAdditionalResponseHeaders) {
            additionalResponseHeaders.insert(QByteArray::fromStdString("test1"),
                                             QByteArray::fromStdString("test1VALUE"));
            additionalResponseHeaders.insert(QByteArray::fromStdString("test2"),
                                             QByteArray::fromStdString("test2VALUE"));
        }
        job->setAdditionalResponseHeaders(additionalResponseHeaders);

        QFile *file = new QFile(QStringLiteral(":additionalResponseHeadersScript.html"), job);
        file->open(QIODevice::ReadOnly);

        job->reply(QByteArrayLiteral("text/html"), file);
    }

    static void registerUrlScheme()
    {
        QWebEngineUrlScheme webUiScheme(schemeName);
        webUiScheme.setFlags(QWebEngineUrlScheme::CorsEnabled);
        QWebEngineUrlScheme::registerScheme(webUiScheme);
    }

    const static inline QByteArray schemeName =
            QByteArrayLiteral("additionalresponseheadershandler");

private:
    bool m_addAdditionalResponseHeaders;
};

class RequestBodyHandler : public QWebEngineUrlSchemeHandler
{
    Q_OBJECT
public:
    void requestStarted(QWebEngineUrlRequestJob *job) override
    {
        QCOMPARE(job->requestUrl(), QUrl(schemeName + ":about"));
        QCOMPARE(job->requestMethod(), QByteArrayLiteral("POST"));

        QIODevice *requestBodyDevice = job->requestBody();
        requestBodyDevice->open(QIODevice::ReadOnly);
        QByteArray requestBody = requestBodyDevice->readAll();
        requestBodyDevice->close();

        QBuffer *buf = new QBuffer(job);
        buf->open(QBuffer::ReadWrite);
        buf->write(requestBody);
        job->reply(QByteArrayLiteral("text/plain"), buf);
        buf->close();
    }

    static void registerUrlScheme()
    {
        QWebEngineUrlScheme webUiScheme(schemeName);
        webUiScheme.setFlags(QWebEngineUrlScheme::CorsEnabled
                             | QWebEngineUrlScheme::FetchApiAllowed);
        QWebEngineUrlScheme::registerScheme(webUiScheme);
    }

    const static inline QByteArray schemeName = QByteArrayLiteral("requestbodyhandler");
};

class SuccessHandler : public QWebEngineUrlSchemeHandler
{
public:
    SuccessHandler() { }

    void requestStarted(QWebEngineUrlRequestJob *requestJob) override
    {
        if (silentSuccess)
            requestJob->reply("", nullptr);
        else {
            QBuffer *buffer = new QBuffer(requestJob);
            buffer->setData(requestJob->requestUrl().toString().toUtf8());
            requestJob->reply("text/plain;charset=utf-8", buffer);
        }
    }

    static void registerUrlScheme()
    {
        QWebEngineUrlScheme successScheme(schemeName);
        QWebEngineUrlScheme::registerScheme(successScheme);
    }

    bool silentSuccess = false;
    const static inline QByteArray schemeName = QByteArrayLiteral("success");
};

class tst_QWebEngineUrlRequestJob : public QObject
{
    Q_OBJECT

public:
    tst_QWebEngineUrlRequestJob() { }

private Q_SLOTS:
    void initTestCase()
    {
        AdditionalResponseHeadersHandler::registerUrlScheme();
        RequestBodyHandler::registerUrlScheme();
        SuccessHandler::registerUrlScheme();
    }

    void withAdditionalResponseHeaders_data()
    {
        QTest::addColumn<bool>("withHeaders");
        QTest::addColumn<QString>("expectedHeaders");
        QTest::newRow("headers enabled")
                << true << "content-type: text/html\r\ntest1: test1value\r\ntest2: test2value\r\n";
        QTest::newRow("headers disabled") << false << "content-type: text/html\r\n";
    }

    void withAdditionalResponseHeaders()
    {
        QFETCH(bool, withHeaders);
        QFETCH(QString, expectedHeaders);

        QWebEngineProfile profile;

        AdditionalResponseHeadersHandler handler(withHeaders);
        profile.installUrlSchemeHandler(AdditionalResponseHeadersHandler::schemeName, &handler);

        CustomPage page(&profile, "TST_ADDITIONALRESPONSEHEADERS", expectedHeaders);
        QSignalSpy spy(&page, SIGNAL(loadFinished(bool)));

        page.load(QUrl("qrc:///additionalResponseHeadersScript.html"));
        QVERIFY(spy.wait());
        QCOMPARE(page.comparedMessageCount(), 1);
    }

    void requestBody()
    {
        QWebEngineProfile profile;

        RequestBodyHandler handler;
        profile.installUrlSchemeHandler(RequestBodyHandler::schemeName, &handler);

        const QString expected = "reading request body successful";
        CustomPage page(&profile, "TST_REQUESTBODY", expected);
        QSignalSpy spy(&page, SIGNAL(receivedMessage()));

        page.load(QUrl("qrc:///requestBodyScript.html"));
        QVERIFY(spy.wait());
        QCOMPARE(page.comparedMessageCount(), 1);
    }

    void notifySuccess()
    {
        QWebEngineProfile profile;
        QWebEnginePage page(&profile);
        QSignalSpy loadFinishedSpy(&page, SIGNAL(loadFinished(bool)));

        SuccessHandler handler;
        page.profile()->installUrlSchemeHandler(SuccessHandler::schemeName, &handler);

        page.load(QUrl("success://one"));
        QTRY_COMPARE(loadFinishedSpy.size(), 1);
        QTRY_COMPARE(loadFinishedSpy.at(0).first().toBool(), true);
        QCOMPARE(toPlainTextSync(&page), "success://one");

        handler.silentSuccess = true;

        page.load(QUrl("success://two"));
        // Page load was successful
        QTRY_COMPARE(loadFinishedSpy.size(), 2);
        QTRY_COMPARE(loadFinishedSpy.at(1).first().toBool(), true);
        // The content of the page did not change
        QCOMPARE(toPlainTextSync(&page), "success://one");
    }
};

QTEST_MAIN(tst_QWebEngineUrlRequestJob)
#include "tst_qwebengineurlrequestjob.moc"
