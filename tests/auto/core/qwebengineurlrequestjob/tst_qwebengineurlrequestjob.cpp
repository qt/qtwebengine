// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <QtTest/QtTest>
#include <QtWebEngineCore/qwebengineurlschemehandler.h>
#include <QtWebEngineCore/qwebengineurlscheme.h>
#include <QtWebEngineCore/qwebengineurlrequestjob.h>
#include <QtWebEngineCore/qwebengineprofile.h>
#include <QtWebEngineCore/qwebenginepage.h>

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

class AdditionalResponseHeadersPage : public QWebEnginePage
{
    Q_OBJECT

public:
    AdditionalResponseHeadersPage(QWebEngineProfile *profile, QString compareString,
                                  QObject *parent = nullptr)
        : QWebEnginePage(profile, parent), m_compareString(compareString)
    {
    }

protected:
    void javaScriptConsoleMessage(QWebEnginePage::JavaScriptConsoleMessageLevel level,
                                  const QString &message, int lineNumber,
                                  const QString &sourceID) override
    {
        Q_UNUSED(level);
        Q_UNUSED(lineNumber);
        Q_UNUSED(sourceID);

        auto splitMessage = message.split(";");
        if (splitMessage[0] == QString("TST_ADDITIONALRESPONSEHEADERS"))
            QCOMPARE(splitMessage[1], m_compareString);
    }

private:
    QString m_compareString;
};

class tst_QWebEngineUrlRequestJob : public QObject
{
    Q_OBJECT

public:
    tst_QWebEngineUrlRequestJob() { }

private Q_SLOTS:
    void initTestCase() { AdditionalResponseHeadersHandler::registerUrlScheme(); }

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

        AdditionalResponseHeadersPage page(&profile, expectedHeaders);
        QSignalSpy spy(&page, SIGNAL(loadFinished(bool)));

        page.load(QUrl("qrc:///additionalResponseHeadersScript.html"));
        QVERIFY(spy.wait());
    }
};

QTEST_MAIN(tst_QWebEngineUrlRequestJob)
#include "tst_qwebengineurlrequestjob.moc"
