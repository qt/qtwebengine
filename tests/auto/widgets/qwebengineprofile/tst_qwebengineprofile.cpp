// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <util.h>
#include <QtCore/qbuffer.h>
#include <QtCore/qmimedatabase.h>
#include <QtTest/QtTest>
#include <QtWebEngineCore/qwebengineurlrequestinterceptor.h>
#include <QtWebEngineCore/qwebengineurlrequestjob.h>
#include <QtWebEngineCore/qwebenginecookiestore.h>
#include <QtWebEngineCore/qwebengineurlscheme.h>
#include <QtWebEngineCore/qwebengineurlschemehandler.h>
#include <QtWebEngineCore/qwebenginesettings.h>
#include <QtWebEngineCore/qwebengineprofile.h>
#include <QtWebEngineCore/qwebenginepage.h>
#include <QtWebEngineCore/qwebenginedownloadrequest.h>
#include <QtWebEngineWidgets/qwebengineview.h>

#if QT_CONFIG(webengine_webchannel)
#include <QWebChannel>
#endif

#include <httpserver.h>
#include <httpreqrep.h>

#include <map>
#include <mutex>

class tst_QWebEngineProfile : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void defaultProfile_data();
    void defaultProfile();
    void userDefinedProfile();
    void clearDataFromCache();
    void disableCache();
    void urlSchemeHandlers();
    void urlSchemeHandlerFailRequest();
    void urlSchemeHandlerFailOnRead();
    void urlSchemeHandlerStreaming();
    void urlSchemeHandlerStreaming2();
    void urlSchemeHandlerRequestHeaders();
    void urlSchemeHandlerInstallation();
    void urlSchemeHandlerXhrStatus();
    void urlSchemeHandlerScriptModule();
    void urlSchemeHandlerLongReply();
    void customUserAgent();
    void httpAcceptLanguage();
    void downloadItem();
    void changePersistentPath();
    void changeHttpUserAgent();
    void changeHttpAcceptLanguage();
    void changePersistentCookiesPolicy();
    void initiator();
    void badDeleteOrder();
    void qtbug_71895(); // this should be the last test
};

void tst_QWebEngineProfile::initTestCase()
{
    QWebEngineUrlScheme foo("foo");
    QWebEngineUrlScheme stream("stream");
    QWebEngineUrlScheme letterto("letterto");
    QWebEngineUrlScheme aviancarrier("aviancarrier");
    QWebEngineUrlScheme myscheme("myscheme");
    foo.setSyntax(QWebEngineUrlScheme::Syntax::Host);
    stream.setSyntax(QWebEngineUrlScheme::Syntax::HostAndPort);
    stream.setDefaultPort(8080);
    letterto.setSyntax(QWebEngineUrlScheme::Syntax::Path);
    aviancarrier.setSyntax(QWebEngineUrlScheme::Syntax::Path);
    aviancarrier.setFlags(QWebEngineUrlScheme::CorsEnabled);
    QWebEngineUrlScheme::registerScheme(foo);
    QWebEngineUrlScheme::registerScheme(stream);
    QWebEngineUrlScheme::registerScheme(letterto);
    QWebEngineUrlScheme::registerScheme(aviancarrier);
    QWebEngineUrlScheme::registerScheme(myscheme);
}

static QString StandardCacheLocation() { static auto p = QStandardPaths::writableLocation(QStandardPaths::CacheLocation); return p; }
static QString StandardAppDataLocation() { static auto p = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation); return p; }

void tst_QWebEngineProfile::defaultProfile_data()
{
    QTest::addColumn<bool>("userCreated");
    QTest::addRow("global") << false;
    QTest::addRow("user") << true;
}

void tst_QWebEngineProfile::defaultProfile()
{
    QFETCH(bool, userCreated);
    QScopedPointer<QWebEngineProfile> p(userCreated ? new QWebEngineProfile : nullptr);
    QWebEngineProfile *profile = userCreated ? p.get() : QWebEngineProfile::defaultProfile();
    QVERIFY(profile);
    QVERIFY(profile->isOffTheRecord());
    QCOMPARE(profile->storageName(), QString());
    QCOMPARE(profile->httpCacheType(), QWebEngineProfile::MemoryHttpCache);
    QCOMPARE(profile->persistentCookiesPolicy(), QWebEngineProfile::NoPersistentCookies);
    QCOMPARE(profile->cachePath(), QString());
    QCOMPARE(profile->persistentStoragePath(), StandardAppDataLocation() + QStringLiteral("/QtWebEngine/OffTheRecord"));
    // TBD: setters do not really work
    profile->setCachePath(QStringLiteral("/home/foo/bar"));
    QCOMPARE(profile->cachePath(), QString());
    profile->setPersistentStoragePath(QStringLiteral("/home/foo/bar"));
    QCOMPARE(profile->persistentStoragePath(), QStringLiteral("/home/foo/bar"));
    profile->setHttpCacheType(QWebEngineProfile::DiskHttpCache);
    QCOMPARE(profile->httpCacheType(), QWebEngineProfile::MemoryHttpCache);
    profile->setPersistentCookiesPolicy(QWebEngineProfile::ForcePersistentCookies);
    QCOMPARE(profile->persistentCookiesPolicy(), QWebEngineProfile::NoPersistentCookies);
}

void tst_QWebEngineProfile::userDefinedProfile()
{
    QWebEngineProfile profile(QStringLiteral("Test"));
    QVERIFY(!profile.isOffTheRecord());
    QCOMPARE(profile.storageName(), QStringLiteral("Test"));
    QCOMPARE(profile.httpCacheType(), QWebEngineProfile::DiskHttpCache);
    QCOMPARE(profile.persistentCookiesPolicy(), QWebEngineProfile::AllowPersistentCookies);
    QCOMPARE(profile.cachePath(), StandardCacheLocation() + QStringLiteral("/QtWebEngine/Test"));
    QCOMPARE(profile.persistentStoragePath(), StandardAppDataLocation() + QStringLiteral("/QtWebEngine/Test"));
}

class AutoDir : public QDir
{
public:
    AutoDir(const QString &p) : QDir(p)
    {
        makeAbsolute();
        removeRecursively();
    }

    ~AutoDir() { removeRecursively(); }
};
qint64 totalSize(QDir dir)
{
    qint64 sum = 0;
    const QDir::Filters filters{QDir::Dirs, QDir::Files, QDir::NoSymLinks, QDir::NoDotAndDotDot};
    for (const QFileInfo &entry : dir.entryInfoList(filters)) {
        if (entry.isFile())
            sum += entry.size();
        else if (entry.isDir())
            sum += totalSize(entry.filePath());
    }
    return sum;
}

class TestServer : public HttpServer
{
public:
    TestServer()
    {
        connect(this, &HttpServer::newRequest, this, &TestServer::onNewRequest);
    }

private:
    void onNewRequest(HttpReqRep *rr)
    {
        const QDir resourceDir(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath() + "/resources");
        QString path = rr->requestPath();
        path.remove(0, 1);

        if (rr->requestMethod() != "GET" || !resourceDir.exists(path)) {
            rr->sendResponse(404);
            return;
        }

        QFile file(resourceDir.filePath(path));
        file.open(QIODevice::ReadOnly);
        QByteArray data = file.readAll();
        rr->setResponseBody(data);
        QMimeDatabase db;
        QMimeType mime = db.mimeTypeForFileNameAndData(file.fileName(), data);
        rr->setResponseHeader(QByteArrayLiteral("content-type"), mime.name().toUtf8());
        if (!mime.inherits("text/html"))
            rr->setResponseHeader(QByteArrayLiteral("cache-control"),
                                  QByteArrayLiteral("public, max-age=31536000"));
        rr->sendResponse();
    }
};

void tst_QWebEngineProfile::clearDataFromCache()
{
    TestServer server;
    QSignalSpy serverSpy(&server, &HttpServer::newRequest);
    QVERIFY(server.start());

    AutoDir cacheDir("./tst_QWebEngineProfile_clearDataFromCache");

    QWebEngineProfile profile(QStringLiteral("clearDataFromCache"));
    profile.setCachePath(cacheDir.path());
    profile.setHttpCacheType(QWebEngineProfile::DiskHttpCache);

    QWebEnginePage page(&profile);
    QVERIFY(loadSync(&page, server.url("/hedgehog.html")));
    // Wait for GET /favicon.ico
    QTRY_COMPARE(serverSpy.size(), 3);

    QVERIFY(cacheDir.exists("Cache"));
    qint64 sizeBeforeClear = totalSize(cacheDir);
    profile.clearHttpCache();
    // Wait for cache to be cleared.
    QTest::qWait(1000);
    QVERIFY(sizeBeforeClear > totalSize(cacheDir));

    (void)server.stop();
}

void tst_QWebEngineProfile::disableCache()
{
    TestServer server;
    QVERIFY(server.start());

    AutoDir cacheDir("./tst_QWebEngineProfile_disableCache");

    QWebEngineProfile profile("disableCache");
    QWebEnginePage page(&profile);
    profile.setCachePath(cacheDir.path());
    QVERIFY(!cacheDir.exists("Cache"));

    profile.setHttpCacheType(QWebEngineProfile::NoCache);
    // Wait for cache to be cleared.
    QTest::qWait(1000);
    QVERIFY(loadSync(&page, server.url("/hedgehog.html")));
    QVERIFY(!cacheDir.exists("Cache"));

    profile.setHttpCacheType(QWebEngineProfile::DiskHttpCache);
    QVERIFY(loadSync(&page, server.url("/hedgehog.html")));
    QVERIFY(cacheDir.exists("Cache"));

    (void)server.stop();
}

class RedirectingUrlSchemeHandler : public QWebEngineUrlSchemeHandler
{
public:
    void requestStarted(QWebEngineUrlRequestJob *job) override
    {
        job->redirect(QUrl(QStringLiteral("data:text/plain;charset=utf-8,")
                           + job->requestUrl().fileName()));
    }
};

class ReplyingUrlSchemeHandler : public QWebEngineUrlSchemeHandler
{
public:
    ReplyingUrlSchemeHandler(QObject *parent = nullptr)
        : QWebEngineUrlSchemeHandler(parent)
    {
    }
    ~ReplyingUrlSchemeHandler()
    {
    }

    void requestStarted(QWebEngineUrlRequestJob *job) override
    {
        QBuffer *buffer = new QBuffer(job);
        buffer->setData(job->requestUrl().toString().toUtf8());
        m_buffers.append(buffer);
        job->reply("text/plain;charset=utf-8", buffer);
    }

    QList<QPointer<QBuffer>> m_buffers;
};

// an evil version constantly claiming to be at end, similar to QNetworkReply
class StreamingIODeviceBasic : public QIODevice
{
    Q_OBJECT
public:
    StreamingIODeviceBasic(QObject *parent) : QIODevice(parent), m_bytesRead(0), m_bytesAvailable(0)
    {
        setOpenMode(QIODevice::ReadOnly);
        m_timer.start(100, this);
    }
    bool isSequential() const override { return true; }
    qint64 bytesAvailable() const override
    {
        const std::lock_guard<QRecursiveMutex> lock(m_mutex);
        return m_bytesAvailable;
    }
protected:
    bool internalAtEnd() const
    {
        return (m_data.size() >= 1000 && m_bytesRead >= 1000);
    }
    void timerEvent(QTimerEvent *) override
    {
        const std::lock_guard<QRecursiveMutex> lock(m_mutex);
        m_bytesAvailable += 200;
        m_data.append(200, 'c');
        emit readyRead();
        if (m_data.size() >= 1000) {
            m_timer.stop();
            emit readChannelFinished();
        }
    }

    qint64 readData(char *data, qint64 maxlen) override
    {
        const std::lock_guard<QRecursiveMutex> lock(m_mutex);
        qint64 len = qMin(qint64(m_bytesAvailable), maxlen);
        if (len) {
            memcpy(data, m_data.constData() + m_bytesRead, len);
            m_bytesAvailable -= len;
            m_bytesRead += len;
        } else if (internalAtEnd())
            return -1;

        return len;
    }
    qint64 writeData(const char *, qint64) override
    {
        return 0;
    }

    mutable QRecursiveMutex m_mutex;
private:
    QByteArray m_data;
    QBasicTimer m_timer;
    int m_bytesRead;
    int m_bytesAvailable;
};

// A nicer version implementing atEnd
class StreamingIODevice : public StreamingIODeviceBasic
{
public:
    StreamingIODevice(QObject *parent) : StreamingIODeviceBasic(parent) {}
    bool atEnd() const override
    {
        const std::lock_guard<QRecursiveMutex> lock(m_mutex);
        return internalAtEnd();
    }
};

class StreamingUrlSchemeHandler : public QWebEngineUrlSchemeHandler
{
public:
    StreamingUrlSchemeHandler(QObject *parent = nullptr)
        : QWebEngineUrlSchemeHandler(parent)
    {
    }

    void requestStarted(QWebEngineUrlRequestJob *job) override
    {
        job->reply("text/plain;charset=utf-8", new StreamingIODevice(job));
    }
};

class StreamingUrlSchemeHandler2 : public QWebEngineUrlSchemeHandler
{
public:
    StreamingUrlSchemeHandler2(QObject *parent = nullptr)
        : QWebEngineUrlSchemeHandler(parent)
    {
    }

    void requestStarted(QWebEngineUrlRequestJob *job) override
    {
        job->reply("text/plain;charset=utf-8", new StreamingIODeviceBasic(job));
    }
};

void tst_QWebEngineProfile::urlSchemeHandlers()
{
    RedirectingUrlSchemeHandler lettertoHandler;
    QWebEngineProfile profile(QStringLiteral("urlSchemeHandlers"));
    profile.installUrlSchemeHandler("letterto", &lettertoHandler);
    QWebEngineView view;
    view.setPage(new QWebEnginePage(&profile, &view));
    view.settings()->setAttribute(QWebEngineSettings::ErrorPageEnabled, false);
    QString emailAddress = QStringLiteral("egon@olsen-banden.dk");
    QVERIFY(loadSync(view.page(), QUrl(QStringLiteral("letterto:") + emailAddress)));
    QCOMPARE(toPlainTextSync(view.page()), emailAddress);

    // Install a gopher handler after the view has been fully initialized.
    ReplyingUrlSchemeHandler gopherHandler;
    profile.installUrlSchemeHandler("gopher", &gopherHandler);
    QUrl url = QUrl(QStringLiteral("gopher://olsen-banden.dk/benny"));
    QVERIFY(loadSync(view.page(), url));
    QCOMPARE(toPlainTextSync(view.page()), url.toString());

    // Remove the letterto scheme, and check whether it is not handled anymore.
    profile.removeUrlScheme("letterto");
    emailAddress = QStringLiteral("kjeld@olsen-banden.dk");
    QVERIFY(loadSync(view.page(), QUrl(QStringLiteral("letterto:") + emailAddress), false));
    QVERIFY(toPlainTextSync(view.page()) != emailAddress);

    // Check if gopher is still working after removing letterto.
    url = QUrl(QStringLiteral("gopher://olsen-banden.dk/yvonne"));
    QVERIFY(loadSync(view.page(), url));
    QCOMPARE(toPlainTextSync(view.page()), url.toString());

    // Does removeAll work?
    profile.removeAllUrlSchemeHandlers();
    url = QUrl(QStringLiteral("gopher://olsen-banden.dk/harry"));
    QVERIFY(loadSync(view.page(), url, false));
    QVERIFY(toPlainTextSync(view.page()) != url.toString());

    // Install a handler that is owned by the view. Make sure this doesn't crash on shutdown.
    profile.installUrlSchemeHandler("aviancarrier", new ReplyingUrlSchemeHandler(&view));
    url = QUrl(QStringLiteral("aviancarrier:inspector.mortensen@politistyrke.dk"));
    QVERIFY(loadSync(view.page(), url));
    QCOMPARE(toPlainTextSync(view.page()), url.toString());

    // Check that all buffers got deleted
    QCOMPARE(gopherHandler.m_buffers.size(), 2);
    for (int i = 0; i < gopherHandler.m_buffers.size(); ++i)
        QVERIFY(gopherHandler.m_buffers.at(i).isNull());
}

class FailingUrlSchemeHandler : public QWebEngineUrlSchemeHandler
{
public:
    void requestStarted(QWebEngineUrlRequestJob *job) override
    {
        job->fail(QWebEngineUrlRequestJob::UrlInvalid);
    }
};

class FailingIODevice : public QIODevice
{
public:
    FailingIODevice(QWebEngineUrlRequestJob *job) : m_job(job)
    {
    }

    qint64 readData(char *, qint64) override
    {
        m_job->fail(QWebEngineUrlRequestJob::RequestFailed);
        return -1;
    }
    qint64 writeData(const char *, qint64) override
    {
        m_job->fail(QWebEngineUrlRequestJob::RequestFailed);
        return -1;
    }
    void close() override
    {
        QIODevice::close();
        deleteLater();
    }

private:
    QWebEngineUrlRequestJob *m_job;
};

class FailOnReadUrlSchemeHandler : public QWebEngineUrlSchemeHandler
{
public:
    void requestStarted(QWebEngineUrlRequestJob *job) override
    {
        job->reply(QByteArrayLiteral("text/plain"), new FailingIODevice(job));
    }
};


void tst_QWebEngineProfile::urlSchemeHandlerFailRequest()
{
    FailingUrlSchemeHandler handler;
    QWebEngineProfile profile;
    profile.installUrlSchemeHandler("foo", &handler);
    QWebEngineView view;
    QSignalSpy loadFinishedSpy(&view, SIGNAL(loadFinished(bool)));
    view.setPage(new QWebEnginePage(&profile, &view));
    view.settings()->setAttribute(QWebEngineSettings::ErrorPageEnabled, false);
    view.load(QUrl(QStringLiteral("foo://bar")));
    view.show();
    QTRY_COMPARE_WITH_TIMEOUT(loadFinishedSpy.size(), 1, 30000);
    QCOMPARE(toPlainTextSync(view.page()), QString());
}

void tst_QWebEngineProfile::urlSchemeHandlerFailOnRead()
{
    FailOnReadUrlSchemeHandler handler;
    QWebEngineProfile profile;
    profile.installUrlSchemeHandler("foo", &handler);
    QWebEngineView view;
    QSignalSpy loadFinishedSpy(&view, SIGNAL(loadFinished(bool)));
    view.setPage(new QWebEnginePage(&profile, &view));
    view.settings()->setAttribute(QWebEngineSettings::ErrorPageEnabled, false);
    view.load(QUrl(QStringLiteral("foo://bar")));
    view.show();
    QTRY_COMPARE_WITH_TIMEOUT(loadFinishedSpy.size(), 1, 30000);
    QCOMPARE(toPlainTextSync(view.page()), QString());
}

void tst_QWebEngineProfile::urlSchemeHandlerStreaming()
{
    StreamingUrlSchemeHandler handler;
    QWebEngineProfile profile;
    profile.installUrlSchemeHandler("stream", &handler);
    QWebEngineView view;
    QSignalSpy loadFinishedSpy(&view, SIGNAL(loadFinished(bool)));
    view.setPage(new QWebEnginePage(&profile, &view));
    view.settings()->setAttribute(QWebEngineSettings::ErrorPageEnabled, false);
    view.load(QUrl(QStringLiteral("stream://whatever")));
    view.show();
    QTRY_COMPARE_WITH_TIMEOUT(loadFinishedSpy.size(), 1, 30000);
    QByteArray result;
    result.append(1000, 'c');
    QCOMPARE(toPlainTextSync(view.page()), QString::fromLatin1(result));
}

void tst_QWebEngineProfile::urlSchemeHandlerStreaming2()
{
    StreamingUrlSchemeHandler2 handler;
    QWebEngineProfile profile;
    profile.installUrlSchemeHandler("stream", &handler);
    QWebEngineView view;
    QSignalSpy loadFinishedSpy(&view, SIGNAL(loadFinished(bool)));
    view.setPage(new QWebEnginePage(&profile, &view));
    view.settings()->setAttribute(QWebEngineSettings::ErrorPageEnabled, false);
    view.load(QUrl(QStringLiteral("stream://whatever")));
    view.show();
    QTRY_COMPARE_WITH_TIMEOUT(loadFinishedSpy.size(), 1, 30000);
    QByteArray result;
    result.append(1000, 'c');
    QCOMPARE(toPlainTextSync(view.page()), QString::fromLatin1(result));
}

class ExtraHeaderInterceptor : public QWebEngineUrlRequestInterceptor
{
public:
    ExtraHeaderInterceptor() { }

    void setExtraHeader(const QByteArray &key, const QByteArray &value)
    {
        m_extraKey = key;
        m_extraValue = value;
    }

    void interceptRequest(QWebEngineUrlRequestInfo &info) override
    {
        if (info.requestUrl().scheme() == QLatin1String("myscheme"))
            info.setHttpHeader(m_extraKey, m_extraValue);
    }

    QByteArray m_extraKey;
    QByteArray m_extraValue;
};

class RequestHeadersUrlSchemeHandler : public ReplyingUrlSchemeHandler
{
public:
    void setExpectedHeader(const QByteArray &key, const QByteArray &value)
    {
        m_expectedKey = key;
        m_expectedValue = value;
    }
    void requestStarted(QWebEngineUrlRequestJob *job) override
    {
        const auto requestHeaders = job->requestHeaders();
        QVERIFY(requestHeaders.contains(m_expectedKey));
        QCOMPARE(requestHeaders.value(m_expectedKey), m_expectedValue);
        ReplyingUrlSchemeHandler::requestStarted(job);
    }
    QByteArray m_expectedKey;
    QByteArray m_expectedValue;
};

void tst_QWebEngineProfile::urlSchemeHandlerRequestHeaders()
{
    RequestHeadersUrlSchemeHandler handler;
    ExtraHeaderInterceptor interceptor;

    handler.setExpectedHeader("Hello", "World");
    interceptor.setExtraHeader("Hello", "World");

    QWebEngineProfile profile;
    profile.installUrlSchemeHandler("myscheme", &handler);
    profile.setUrlRequestInterceptor(&interceptor);

    QWebEnginePage page(&profile);
    QSignalSpy loadFinishedSpy(&page, SIGNAL(loadFinished(bool)));
    page.load(QUrl(QStringLiteral("myscheme://whatever")));
    QTRY_COMPARE_WITH_TIMEOUT(loadFinishedSpy.size(), 1, 30000);
}

void tst_QWebEngineProfile::urlSchemeHandlerInstallation()
{
    FailingUrlSchemeHandler handler;
    QWebEngineProfile profile;

    // Builtin schemes that *cannot* be overridden.
    for (auto scheme : { "about", "blob", "data", "javascript", "qrc", "https", "http", "file",
                         "ftp", "wss", "ws", "filesystem", "FileSystem" }) {
        QTest::ignoreMessage(
                QtWarningMsg,
                QRegularExpression("Cannot install a URL scheme handler overriding internal scheme.*"));
        auto prevHandler = profile.urlSchemeHandler(scheme);
        profile.installUrlSchemeHandler(scheme, &handler);
        QCOMPARE(profile.urlSchemeHandler(scheme), prevHandler);
    }

    // Builtin schemes that *can* be overridden.
    for (auto scheme : { "gopher", "GOPHER" }) {
        profile.installUrlSchemeHandler(scheme, &handler);
        QCOMPARE(profile.urlSchemeHandler(scheme), &handler);
        profile.removeUrlScheme(scheme);
    }

    // Other schemes should be registered with QWebEngineUrlScheme first, but
    // handler installation still succeeds to preserve backwards compatibility.
    QTest::ignoreMessage(
            QtWarningMsg,
            QRegularExpression("Please register the custom scheme.*"));
    profile.installUrlSchemeHandler("tst", &handler);
    QCOMPARE(profile.urlSchemeHandler("tst"), &handler);

    // Existing handler cannot be overridden.
    FailingUrlSchemeHandler handler2;
    QTest::ignoreMessage(
            QtWarningMsg,
            QRegularExpression("URL scheme handler already installed.*"));
    profile.installUrlSchemeHandler("tst", &handler2);
    QCOMPARE(profile.urlSchemeHandler("tst"), &handler);
    profile.removeUrlScheme("tst");
}

#if QT_CONFIG(webengine_webchannel)
class XhrStatusHost : public QObject
{
    Q_OBJECT
public:
    std::map<QUrl, int> requests;

    bool isReady()
    {
        static const auto sig = QMetaMethod::fromSignal(&XhrStatusHost::load);
        return isSignalConnected(sig);
    }

Q_SIGNALS:
    void load(QUrl url);

public Q_SLOTS:
    void loadFinished(QUrl url, int status)
    {
        requests[url] = status;
    }

private:
};

class XhrStatusUrlSchemeHandler : public QWebEngineUrlSchemeHandler
{
public:
    void requestStarted(QWebEngineUrlRequestJob *job) override
    {
        QString path = job->requestUrl().path();
        if (path == "/") {
            QBuffer *buffer = new QBuffer(job);
            buffer->open(QBuffer::ReadWrite);
            buffer->write(QByteArrayLiteral(R"(
<html>
  <body>
    <script src="qwebchannel.js"></script>
    <script>
      new QWebChannel(qt.webChannelTransport, (channel) => {
        const host = channel.objects.host;
        host.load.connect((url) => {
          const xhr = new XMLHttpRequest();
          xhr.onload = () => { host.loadFinished(url, xhr.status); };
          xhr.onerror = () => { host.loadFinished(url, -1); };
          xhr.open("GET", url, true);
          xhr.send();
        });
      });
    </script>
  </body>
</html>
)"));
            buffer->seek(0);
            job->reply("text/html", buffer);
        } else if (path == "/qwebchannel.js") {
            QFile *file = new QFile(":/qtwebchannel/qwebchannel.js", job);
            file->open(QFile::ReadOnly);
            job->reply("application/javascript", file);
        } else if (path == "/ok") {
            QBuffer *buffer = new QBuffer(job);
            buffer->buffer() = QByteArrayLiteral("ok");
            job->reply("text/plain", buffer);
        } else if (path == "/redirect") {
            QUrl url = job->requestUrl();
            url.setPath("/ok");
            job->redirect(url);
        } else if (path == "/fail") {
            job->fail(QWebEngineUrlRequestJob::RequestFailed);
        } else {
            job->fail(QWebEngineUrlRequestJob::UrlNotFound);
        }
    }
};
#endif

void tst_QWebEngineProfile::urlSchemeHandlerXhrStatus()
{
#if QT_CONFIG(webengine_webchannel)
    XhrStatusUrlSchemeHandler handler;
    XhrStatusHost host;
    QWebEngineProfile profile;
    QWebEnginePage page(&profile);
    QWebChannel channel;
    channel.registerObject("host", &host);
    profile.installUrlSchemeHandler("aviancarrier", &handler);
    page.setWebChannel(&channel);
    page.load(QUrl("aviancarrier:/"));
    QTRY_VERIFY_WITH_TIMEOUT(host.isReady(), 30000);
    host.load(QUrl("aviancarrier:/ok"));
    host.load(QUrl("aviancarrier:/redirect"));
    host.load(QUrl("aviancarrier:/fail"));
    host.load(QUrl("aviancarrier:/notfound"));
    QTRY_COMPARE(host.requests.size(), 4u);
    QCOMPARE(host.requests[QUrl("aviancarrier:/ok")], 200);
    QCOMPARE(host.requests[QUrl("aviancarrier:/redirect")], 200);
    QCOMPARE(host.requests[QUrl("aviancarrier:/fail")], -1);
    QCOMPARE(host.requests[QUrl("aviancarrier:/notfound")], -1);
#else
    QSKIP("No QtWebChannel");
#endif
}

class ScriptsUrlSchemeHandler : public QWebEngineUrlSchemeHandler
{
public:
    void requestStarted(QWebEngineUrlRequestJob *job) override
    {
        auto *script = new QBuffer(job);
        script->setData(QByteArrayLiteral("window.test = 'SUCCESS';"));
        job->reply("text/javascript", script);
    }
};

void tst_QWebEngineProfile::urlSchemeHandlerScriptModule()
{
    ScriptsUrlSchemeHandler handler;
    QWebEngineProfile profile;
    profile.installUrlSchemeHandler("aviancarrier", &handler);
    QWebEnginePage page(&profile);
    QSignalSpy loadFinishedSpy(&page, SIGNAL(loadFinished(bool)));
    page.setHtml(QStringLiteral("<html><head><script src=\"aviancarrier:///\"></script></head><body>Test1</body></html>"));
    QTRY_COMPARE(loadFinishedSpy.size(), 1);
    QCOMPARE(evaluateJavaScriptSync(&page, QStringLiteral("test")).toString(), QStringLiteral("SUCCESS"));

    loadFinishedSpy.clear();
    page.setHtml(QStringLiteral("<html><head><script type=\"module\" src=\"aviancarrier:///\"></script></head><body>Test2</body></html>"));
    QTRY_COMPARE(loadFinishedSpy.size(), 1);
    QCOMPARE(evaluateJavaScriptSync(&page, QStringLiteral("test")).toString(), QStringLiteral("SUCCESS"));
}

class LongReplyUrlSchemeHandler : public QWebEngineUrlSchemeHandler
{
public:
    LongReplyUrlSchemeHandler(QObject *parent = nullptr) : QWebEngineUrlSchemeHandler(parent) {}
    ~LongReplyUrlSchemeHandler() {}

    void requestStarted(QWebEngineUrlRequestJob *job) override
    {
        QBuffer *buffer = new QBuffer(job);
        buffer->setData(QByteArray(128 * 1024, ' ') +
                        "<html><head><title>Minify this!</title></head></html>");
        job->reply("text/html", buffer);
    }
};

void tst_QWebEngineProfile::urlSchemeHandlerLongReply()
{
    LongReplyUrlSchemeHandler handler;
    QWebEngineProfile profile;
    profile.installUrlSchemeHandler("aviancarrier", &handler);
    QWebEnginePage page(&profile);
    page.load(QUrl("aviancarrier:/"));
    QTRY_COMPARE(page.title(), QString("Minify this!"));
}

void tst_QWebEngineProfile::customUserAgent()
{
    QString defaultUserAgent = QWebEngineProfile::defaultProfile()->httpUserAgent();
    QWebEnginePage page;
    QSignalSpy loadFinishedSpy(&page, SIGNAL(loadFinished(bool)));
    page.setHtml(QStringLiteral("<html><body>Hello world!</body></html>"));
    QTRY_COMPARE(loadFinishedSpy.size(), 1);

    // First test the user-agent is default
    QCOMPARE(evaluateJavaScriptSync(&page, QStringLiteral("navigator.userAgent")).toString(), defaultUserAgent);

    const QString testUserAgent = QStringLiteral("tst_QWebEngineProfile 1.0");
    QWebEngineProfile testProfile;
    testProfile.setHttpUserAgent(testUserAgent);

    // Test a new profile with custom user-agent works
    QWebEnginePage page2(&testProfile);
    QSignalSpy loadFinishedSpy2(&page2, SIGNAL(loadFinished(bool)));
    page2.setHtml(QStringLiteral("<html><body>Hello again!</body></html>"));
    QTRY_COMPARE(loadFinishedSpy2.size(), 1);
    QCOMPARE(evaluateJavaScriptSync(&page2, QStringLiteral("navigator.userAgent")).toString(), testUserAgent);
    QCOMPARE(evaluateJavaScriptSync(&page, QStringLiteral("navigator.userAgent")).toString(), defaultUserAgent);

    // Test an existing page and profile with custom user-agent works
    QWebEngineProfile::defaultProfile()->setHttpUserAgent(testUserAgent);
    QCOMPARE(evaluateJavaScriptSync(&page, QStringLiteral("navigator.userAgent")).toString(), testUserAgent);
}

void tst_QWebEngineProfile::httpAcceptLanguage()
{
    QWebEnginePage page;
    QSignalSpy loadFinishedSpy(&page, SIGNAL(loadFinished(bool)));
    page.setHtml(QStringLiteral("<html><body>Hello world!</body></html>"));
    QTRY_COMPARE(loadFinishedSpy.size(), 1);

    QStringList defaultLanguages = evaluateJavaScriptSync(&page, QStringLiteral("navigator.languages")).toStringList();

    const QString testLang = QStringLiteral("xx-YY");
    QWebEngineProfile testProfile;
    testProfile.setHttpAcceptLanguage(testLang);

    // Test a completely new profile
    QWebEnginePage page2(&testProfile);
    QSignalSpy loadFinishedSpy2(&page2, SIGNAL(loadFinished(bool)));
    page2.setHtml(QStringLiteral("<html><body>Hello again!</body></html>"));
    QTRY_COMPARE(loadFinishedSpy2.size(), 1);
    QCOMPARE(evaluateJavaScriptSync(&page2, QStringLiteral("navigator.languages")).toStringList(), QStringList(testLang));
    // Test the old one wasn't affected
    QCOMPARE(evaluateJavaScriptSync(&page, QStringLiteral("navigator.languages")).toStringList(), defaultLanguages);

    // Test changing an existing page and profile
    QWebEngineProfile::defaultProfile()->setHttpAcceptLanguage(testLang);
    QCOMPARE(evaluateJavaScriptSync(&page, QStringLiteral("navigator.languages")).toStringList(), QStringList(testLang));
}

void tst_QWebEngineProfile::downloadItem()
{
    qRegisterMetaType<QWebEngineDownloadRequest *>();
    QWebEngineProfile testProfile;
    QWebEnginePage page(&testProfile);
    QSignalSpy downloadSpy(&testProfile, SIGNAL(downloadRequested(QWebEngineDownloadRequest *)));
    page.load(QUrl::fromLocalFile(QCoreApplication::applicationFilePath()));
    QTRY_COMPARE(downloadSpy.size(), 1);
}

void tst_QWebEngineProfile::changePersistentPath()
{
    TestServer server;
    QVERIFY(server.start());

    AutoDir dataDir1(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                     + QStringLiteral("/QtWebEngine/changePersistentPath1"));
    AutoDir dataDir2(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                     + QStringLiteral("/QtWebEngine/changePersistentPath2"));

    QWebEngineProfile testProfile(QStringLiteral("changePersistentPath1"));
    QCOMPARE(testProfile.persistentStoragePath(), dataDir1.path());

    // Make sure the profile has been used:
    QWebEnginePage page(&testProfile);
    QVERIFY(loadSync(&page, server.url("/hedgehog.html")));

    // Test we do not crash (QTBUG-55322):
    testProfile.setPersistentStoragePath(dataDir2.path());
    QCOMPARE(testProfile.persistentStoragePath(), dataDir2.path());
    QVERIFY(loadSync(&page, server.url("/hedgehog.html")));
    QVERIFY(dataDir2.exists());

    (void)server.stop();
}

void tst_QWebEngineProfile::changeHttpUserAgent()
{
    TestServer server;
    QVERIFY(server.start());

    QList<QByteArray> userAgents;
    connect(&server, &HttpServer::newRequest, [&](HttpReqRep *rr) {
        if (rr->requestPath() == "/hedgehog.html")
            userAgents.push_back(rr->requestHeader(QByteArrayLiteral("user-agent")));
    });

    QWebEngineProfile profile(QStringLiteral("changeHttpUserAgent"));
    std::unique_ptr<QWebEnginePage> page;
    page.reset(new QWebEnginePage(&profile));
    QVERIFY(loadSync(page.get(), server.url("/hedgehog.html")));
    page.reset();
    profile.setHttpUserAgent("webturbine/42");
    page.reset(new QWebEnginePage(&profile));
    QVERIFY(loadSync(page.get(), server.url("/hedgehog.html")));

    QCOMPARE(userAgents.size(), 2);
    QCOMPARE(userAgents[1], "webturbine/42");
    QVERIFY(userAgents[0] != userAgents[1]);

    QVERIFY(server.stop());
}

void tst_QWebEngineProfile::changeHttpAcceptLanguage()
{
    TestServer server;
    QVERIFY(server.start());

    QList<QByteArray> languages;
    connect(&server, &HttpServer::newRequest, [&](HttpReqRep *rr) {
        if (rr->requestPath() == "/hedgehog.html")
            languages.push_back(rr->requestHeader(QByteArrayLiteral("accept-language")));
    });

    QWebEngineProfile profile(QStringLiteral("changeHttpAcceptLanguage"));
    std::unique_ptr<QWebEnginePage> page;
    page.reset(new QWebEnginePage(&profile));
    QVERIFY(loadSync(page.get(), server.url("/hedgehog.html")));
    page.reset();
    profile.setHttpAcceptLanguage("fi");
    page.reset(new QWebEnginePage(&profile));
    QVERIFY(loadSync(page.get(), server.url("/hedgehog.html")));

    QCOMPARE(languages.size(), 2);
    QCOMPARE(languages[1], "fi");
    QVERIFY(languages[0] != languages[1]);

    QVERIFY(server.stop());
}

void tst_QWebEngineProfile::changePersistentCookiesPolicy()
{
    TestServer server;
    QVERIFY(server.start());

    AutoDir dataDir("./tst_QWebEngineProfile_dataDir");

    QWebEngineProfile profile(QStringLiteral("changePersistentCookiesPolicy"));
    QWebEnginePage page(&profile);

    profile.setPersistentStoragePath(dataDir.path());
    profile.setPersistentCookiesPolicy(QWebEngineProfile::NoPersistentCookies);

    QVERIFY(loadSync(&page, server.url("/hedgehog.html")));
    QVERIFY(!dataDir.exists("Cookies"));

    profile.setPersistentCookiesPolicy(QWebEngineProfile::ForcePersistentCookies);

    QVERIFY(loadSync(&page, server.url("/hedgehog.html")));
    QVERIFY(dataDir.exists("Cookies"));

    (void)server.stop();
}

class InitiatorSpy : public QWebEngineUrlSchemeHandler
{
public:
    QUrl initiator;
    void requestStarted(QWebEngineUrlRequestJob *job) override
    {
        initiator = job->initiator();
        job->fail(QWebEngineUrlRequestJob::RequestDenied);
    }
};

void tst_QWebEngineProfile::initiator()
{
    InitiatorSpy handler;
    QWebEngineProfile profile;
    profile.installUrlSchemeHandler("foo", &handler);
    QWebEnginePage page(&profile, nullptr);
    QSignalSpy loadFinishedSpy(&page, SIGNAL(loadFinished(bool)));
    page.load(QUrl("about:blank"));
    QTRY_COMPARE(loadFinishedSpy.size(), 1);
    loadFinishedSpy.clear();

    // about:blank has a unique origin, so initiator should be QUrl("null")
    evaluateJavaScriptSync(&page, "window.location = 'foo:bar'");
    QTRY_COMPARE(loadFinishedSpy.size(), 1);
    loadFinishedSpy.clear();
    QCOMPARE(handler.initiator, QUrl("null"));

    page.setHtml("", QUrl("http://test:123/foo%20bar"));
    QTRY_COMPARE(loadFinishedSpy.size(), 1);
    loadFinishedSpy.clear();

    // baseUrl determines the origin, so QUrl("http://test:123")
    evaluateJavaScriptSync(&page, "window.location = 'foo:bar'");
    QTRY_COMPARE(loadFinishedSpy.size(), 1);
    loadFinishedSpy.clear();
    QCOMPARE(handler.initiator, QUrl("http://test:123"));

    // Directly calling load/setUrl should have initiator QUrl(), meaning
    // browser-initiated, trusted.
    page.load(QUrl("foo:bar"));
    QTRY_COMPARE_WITH_TIMEOUT(loadFinishedSpy.size(), 1, 10000);
    QCOMPARE(handler.initiator, QUrl());
}

void tst_QWebEngineProfile::badDeleteOrder()
{
    QWebEngineProfile *profile = new QWebEngineProfile();
    QWebEngineView *view = new QWebEngineView();
    view->resize(640, 480);
    view->show();
    QVERIFY(QTest::qWaitForWindowExposed(view));
    QWebEnginePage *page = new QWebEnginePage(profile, view);
    view->setPage(page);

    QSignalSpy spyLoadFinished(page, SIGNAL(loadFinished(bool)));
    page->setHtml(QStringLiteral("<html><body><h1>Badly handled page!</h1></body></html>"));
    QTRY_COMPARE(spyLoadFinished.size(), 1);

    delete profile;
    delete view;
}

void tst_QWebEngineProfile::qtbug_71895()
{
    QWebEngineView view;
    QSignalSpy loadSpy(view.page(), SIGNAL(loadFinished(bool)));
    view.setUrl(QUrl("https://www.qt.io"));
    view.show();
    view.page()->profile()->clearHttpCache();
    view.page()->profile()->setHttpCacheType(QWebEngineProfile::NoCache);
    view.page()->profile()->cookieStore()->deleteAllCookies();
    view.page()->profile()->setPersistentCookiesPolicy(QWebEngineProfile::NoPersistentCookies);
    bool gotSignal = loadSpy.size() || loadSpy.wait(20000);
    if (!gotSignal)
        QSKIP("Couldn't load page from network, skipping test.");
}


QTEST_MAIN(tst_QWebEngineProfile)
#include "tst_qwebengineprofile.moc"
