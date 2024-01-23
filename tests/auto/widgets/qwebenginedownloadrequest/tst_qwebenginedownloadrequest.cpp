// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <util.h>

#include <QCoreApplication>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>
#include <QRegularExpression>
#include <QWebEngineDownloadRequest>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QWebEngineView>
#include <httpserver.h>

class tst_QWebEngineDownloadRequest : public QObject
{
    Q_OBJECT

public:
    enum UserAction {
        SaveLink,
        ClickLink,
    };

    enum FileAction {
        FileIsDownloaded,
        FileIsDisplayed,
    };

private Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();
    void cleanupTestCase();

    void downloadLink_data();
    void downloadLink();
    void downloadTwoLinks_data();
    void downloadTwoLinks();
    void downloadPage_data();
    void downloadPage();
    void downloadViaSetUrl();
    void downloadFileNot1();
    void downloadFileNot2();
    void downloadDeleted();
    void downloadDeletedByProfile();
    void downloadUniqueFilename_data();
    void downloadUniqueFilename();
    void downloadUniqueFilenameWithTimestamp();
    void downloadToDefaultLocation();
    void downloadToNonExistentDir();
    void downloadToReadOnlyDir();
    void downloadToDirectoryWithFileName_data();
    void downloadToDirectoryWithFileName();
    void downloadDataUrls_data();
    void downloadDataUrls();

private:
    void saveLink(QPoint linkPos);
    void clickLink(QPoint linkPos);
    void simulateUserAction(QPoint linkPos, UserAction action);

    HttpServer *m_server;
    QWebEngineProfile *m_profile;
    QWebEnginePage *m_page;
    QWebEngineView *m_view;
    QSet<QWebEngineDownloadRequest *> m_requestedDownloads;
    QSet<QWebEngineDownloadRequest *> m_finishedDownloads;
};

Q_DECLARE_METATYPE(tst_QWebEngineDownloadRequest::UserAction)
Q_DECLARE_METATYPE(tst_QWebEngineDownloadRequest::FileAction)

void tst_QWebEngineDownloadRequest::initTestCase()
{
    m_server = new HttpServer();
    m_profile = new QWebEngineProfile;
    m_profile->setHttpCacheType(QWebEngineProfile::NoCache);
    m_profile->settings()->setAttribute(QWebEngineSettings::AutoLoadIconsForPage, false);
    connect(m_profile, &QWebEngineProfile::downloadRequested, [this](QWebEngineDownloadRequest *item) {
        m_requestedDownloads.insert(item);
        connect(item, &QWebEngineDownloadRequest::destroyed, [this, item](){
            m_requestedDownloads.remove(item);
            m_finishedDownloads.remove(item);
        });
        connect(item, &QWebEngineDownloadRequest::isFinishedChanged, [this, item](){
            m_finishedDownloads.insert(item);
        });
    });
    m_page = new QWebEnginePage(m_profile);
    m_view = new QWebEngineView;
    m_view->setPage(m_page);
    m_view->resize(640, 480);
    m_view->show();
}

void tst_QWebEngineDownloadRequest::init()
{
    QVERIFY(m_server->start());
}

void tst_QWebEngineDownloadRequest::cleanup()
{
    for (QWebEngineDownloadRequest *item : m_finishedDownloads) {
        item->deleteLater();
    }
    QTRY_COMPARE(m_requestedDownloads.size(), 0);
    QCOMPARE(m_finishedDownloads.size(), 0);
    QVERIFY(m_server->stop());
    // Set download path to default.
    m_profile->setDownloadPath("");
}

void tst_QWebEngineDownloadRequest::cleanupTestCase()
{
    delete m_view;
    delete m_page;
    delete m_profile;
    delete m_server;
}

void tst_QWebEngineDownloadRequest::saveLink(QPoint linkPos)
{
    // Simulate right-clicking on link and choosing "save link as" from menu.
    QSignalSpy menuSpy(m_view, &QWebEngineView::customContextMenuRequested);
    m_view->setContextMenuPolicy(Qt::CustomContextMenu);
    auto event1 =
            new QContextMenuEvent(QContextMenuEvent::Mouse, linkPos, m_view->mapToGlobal(linkPos));
    auto event2 = new QMouseEvent(QEvent::MouseButtonPress, linkPos, m_view->mapToGlobal(linkPos),
                                  Qt::RightButton, {}, {});
    auto event3 = new QMouseEvent(QEvent::MouseButtonRelease, linkPos, m_view->mapToGlobal(linkPos),
                                  Qt::RightButton, {}, {});
    QTRY_VERIFY(m_view->focusWidget());
    QWidget *renderWidget = m_view->focusWidget();
    QCoreApplication::postEvent(renderWidget, event1);
    QCoreApplication::postEvent(renderWidget, event2);
    QCoreApplication::postEvent(renderWidget, event3);
    QTRY_COMPARE(menuSpy.size(), 1);
    m_page->triggerAction(QWebEnginePage::DownloadLinkToDisk);
}

void tst_QWebEngineDownloadRequest::clickLink(QPoint linkPos)
{
    // Simulate left-clicking on link.
    QTRY_VERIFY(m_view->focusWidget());
    QWidget *renderWidget = m_view->focusWidget();
    QTest::mouseClick(renderWidget, Qt::LeftButton, {}, linkPos);
}

void tst_QWebEngineDownloadRequest::simulateUserAction(QPoint linkPos, UserAction action)
{
    switch (action) {
    case SaveLink: return saveLink(linkPos);
    case ClickLink: return clickLink(linkPos);
    }
}

void tst_QWebEngineDownloadRequest::downloadLink_data()
{
    QTest::addColumn<UserAction>("userAction");
    QTest::addColumn<bool>("anchorHasDownloadAttribute");
    QTest::addColumn<QByteArray>("fileName");
    QTest::addColumn<QByteArray>("fileContents");
    QTest::addColumn<QByteArray>("fileMimeTypeDeclared");
    QTest::addColumn<QByteArray>("fileMimeTypeDetected");
    QTest::addColumn<QByteArray>("fileDisposition");
    QTest::addColumn<bool>("fileHasReferer");
    QTest::addColumn<FileAction>("fileAction");

    // SaveLink should always trigger a download, even for empty files.
    QTest::newRow("save link to empty file")
        /* userAction                 */ << SaveLink
        /* anchorHasDownloadAttribute */ << false
        /* fileName                   */ << QByteArrayLiteral("foo.txt")
        /* fileContents               */ << QByteArrayLiteral("")
        /* fileMimeTypeDeclared       */ << QByteArrayLiteral("text/plain")
        /* fileMimeTypeDetected       */ << QByteArrayLiteral("text/plain")
        /* fileDisposition            */ << QByteArrayLiteral("")
        /* fileHasReferer             */ << true
        /* fileAction                 */ << FileIsDownloaded;

    // SaveLink should always trigger a download, also for text files.
    QTest::newRow("save link to text file")
        /* userAction                 */ << SaveLink
        /* anchorHasDownloadAttribute */ << false
        /* fileName                   */ << QByteArrayLiteral("foo.txt")
        /* fileContents               */ << QByteArrayLiteral("bar")
        /* fileMimeTypeDeclared       */ << QByteArrayLiteral("text/plain")
        /* fileMimeTypeDetected       */ << QByteArrayLiteral("text/plain")
        /* fileDisposition            */ << QByteArrayLiteral("")
        /* fileHasReferer             */ << true
        /* fileAction                 */ << FileIsDownloaded;

    // ... adding the "download" attribute should have no effect.
    QTest::newRow("save link to text file (attribute)")
        /* userAction                 */ << SaveLink
        /* anchorHasDownloadAttribute */ << true
        /* fileName                   */ << QByteArrayLiteral("foo.txt")
        /* fileContents               */ << QByteArrayLiteral("bar")
        /* fileMimeTypeDeclared       */ << QByteArrayLiteral("text/plain")
        /* fileMimeTypeDetected       */ << QByteArrayLiteral("text/plain")
        /* fileDisposition            */ << QByteArrayLiteral("")
        /* fileHasReferer             */ << true
        /* fileAction                 */ << FileIsDownloaded;

    // ... adding the "attachment" content disposition should also have no effect.
    QTest::newRow("save link to text file (attachment)")
        /* userAction                 */ << SaveLink
        /* anchorHasDownloadAttribute */ << false
        /* fileName                   */ << QByteArrayLiteral("foo.txt")
        /* fileContents               */ << QByteArrayLiteral("bar")
        /* fileMimeTypeDeclared       */ << QByteArrayLiteral("text/plain")
        /* fileMimeTypeDetected       */ << QByteArrayLiteral("text/plain")
        /* fileDisposition            */ << QByteArrayLiteral("attachment")
        /* fileHasReferer             */ << true
        /* fileAction                 */ << FileIsDownloaded;

    // ... even adding both should have no effect.
    QTest::newRow("save link to text file (attribute+attachment)")
        /* userAction                 */ << SaveLink
        /* anchorHasDownloadAttribute */ << true
        /* fileName                   */ << QByteArrayLiteral("foo.txt")
        /* fileContents               */ << QByteArrayLiteral("bar")
        /* fileMimeTypeDeclared       */ << QByteArrayLiteral("text/plain")
        /* fileMimeTypeDetected       */ << QByteArrayLiteral("text/plain")
        /* fileDisposition            */ << QByteArrayLiteral("attachment")
        /* fileHasReferer             */ << true
        /* fileAction                 */ << FileIsDownloaded;

    // Navigating to an empty file should show an empty page.
    QTest::newRow("navigate to empty file")
        /* userAction                 */ << ClickLink
        /* anchorHasDownloadAttribute */ << false
        /* fileName                   */ << QByteArrayLiteral("foo.txt")
        /* fileContents               */ << QByteArrayLiteral("")
        /* fileMimeTypeDeclared       */ << QByteArrayLiteral("")
        /* fileMimeTypeDetected       */ << QByteArrayLiteral("")
        /* fileDisposition            */ << QByteArrayLiteral("")
        /* fileHasReferer             */ << true
        /* fileAction                 */ << FileIsDisplayed;

    // Navigating to a text file should show the text file.
    QTest::newRow("navigate to text file")
        /* userAction                 */ << ClickLink
        /* anchorHasDownloadAttribute */ << false
        /* fileName                   */ << QByteArrayLiteral("foo.txt")
        /* fileContents               */ << QByteArrayLiteral("bar")
        /* fileMimeTypeDeclared       */ << QByteArrayLiteral("text/plain")
        /* fileMimeTypeDetected       */ << QByteArrayLiteral("text/plain")
        /* fileDisposition            */ << QByteArrayLiteral("")
        /* fileHasReferer             */ << true
        /* fileAction                 */ << FileIsDisplayed;

    // ... unless the link has the "download" attribute: then the file should be downloaded.
    QTest::newRow("navigate to text file (attribute)")
        /* userAction                 */ << ClickLink
        /* anchorHasDownloadAttribute */ << true
        /* fileName                   */ << QByteArrayLiteral("foo.txt")
        /* fileContents               */ << QByteArrayLiteral("bar")
        /* fileMimeTypeDeclared       */ << QByteArrayLiteral("text/plain")
        /* fileMimeTypeDetected       */ << QByteArrayLiteral("text/plain")
        /* fileDisposition            */ << QByteArrayLiteral("")
        /* fileHasReferer             */ << true
        /* fileAction                 */ << FileIsDownloaded;

    // ... same with the content disposition header save for the download type.
    QTest::newRow("navigate to text file (attachment)")
        /* userAction                 */ << ClickLink
        /* anchorHasDownloadAttribute */ << false
        /* fileName                   */ << QByteArrayLiteral("foo.txt")
        /* fileContents               */ << QByteArrayLiteral("bar")
        /* fileMimeTypeDeclared       */ << QByteArrayLiteral("text/plain")
        /* fileMimeTypeDetected       */ << QByteArrayLiteral("text/plain")
        /* fileDisposition            */ << QByteArrayLiteral("attachment")
        /* fileHasReferer             */ << true
        /* fileAction                 */ << FileIsDownloaded;

    // ... and both.
    QTest::newRow("navigate to text file (attribute+attachment)")
        /* userAction                 */ << ClickLink
        /* anchorHasDownloadAttribute */ << true
        /* fileName                   */ << QByteArrayLiteral("foo.txt")
        /* fileContents               */ << QByteArrayLiteral("bar")
        /* fileMimeTypeDeclared       */ << QByteArrayLiteral("text/plain")
        /* fileMimeTypeDetected       */ << QByteArrayLiteral("text/plain")
        /* fileDisposition            */ << QByteArrayLiteral("attachment")
        /* fileHasReferer             */ << true
        /* fileAction                 */ << FileIsDownloaded;

    // The file's extension has no effect.
    QTest::newRow("navigate to supposed zip file")
        /* userAction                 */ << ClickLink
        /* anchorHasDownloadAttribute */ << false
        /* fileName                   */ << QByteArrayLiteral("foo.zip")
        /* fileContents               */ << QByteArrayLiteral("bar")
        /* fileMimeTypeDeclared       */ << QByteArrayLiteral("")
        /* fileMimeTypeDetected       */ << QByteArrayLiteral("")
        /* fileDisposition            */ << QByteArrayLiteral("")
        /* fileHasReferer             */ << true
        /* fileAction                 */ << FileIsDisplayed;

    // ... the file's mime type however does.
    QTest::newRow("navigate to supposed zip file (application/zip)")
        /* userAction                 */ << ClickLink
        /* anchorHasDownloadAttribute */ << false
        /* fileName                   */ << QByteArrayLiteral("foo.zip")
        /* fileContents               */ << QByteArrayLiteral("bar")
        /* fileMimeTypeDeclared       */ << QByteArrayLiteral("application/zip")
        /* fileMimeTypeDetected       */ << QByteArrayLiteral("application/zip")
        /* fileDisposition            */ << QByteArrayLiteral("")
        /* fileHasReferer             */ << true
        /* fileAction                 */ << FileIsDownloaded;

    // ... but we're not very picky about the exact type.
    QTest::newRow("navigate to supposed zip file (application/octet-stream)")
        /* userAction                 */ << ClickLink
        /* anchorHasDownloadAttribute */ << false
        /* fileName                   */ << QByteArrayLiteral("foo.zip")
        /* fileContents               */ << QByteArrayLiteral("bar")
        /* fileMimeTypeDeclared       */ << QByteArrayLiteral("application/octet-stream")
        /* fileMimeTypeDetected       */ << QByteArrayLiteral("application/octet-stream")
        /* fileDisposition            */ << QByteArrayLiteral("")
        /* fileHasReferer             */ << true
        /* fileAction                 */ << FileIsDownloaded;

    // empty zip file (consisting only of "end of central directory record")
    QByteArray zipFile = QByteArrayLiteral("PK\x05\x06") + QByteArray(18, 0);

    // The mime type is guessed automatically if not provided by the server.
    QTest::newRow("navigate to actual zip file")
        /* userAction                 */ << ClickLink
        /* anchorHasDownloadAttribute */ << false
        /* fileName                   */ << QByteArrayLiteral("foo.zip")
        /* fileContents               */ << zipFile
        /* fileMimeTypeDeclared       */ << QByteArrayLiteral("")
        /* fileMimeTypeDetected       */ << QByteArrayLiteral("application/octet-stream")
        /* fileDisposition            */ << QByteArrayLiteral("")
        /* fileHasReferer             */ << true
        /* fileAction                 */ << FileIsDownloaded;

    // The mime type is not guessed automatically if provided by the server.
    QTest::newRow("navigate to actual zip file (application/zip)")
        /* userAction                 */ << ClickLink
        /* anchorHasDownloadAttribute */ << false
        /* fileName                   */ << QByteArrayLiteral("foo.zip")
        /* fileContents               */ << zipFile
        /* fileMimeTypeDeclared       */ << QByteArrayLiteral("application/zip")
        /* fileMimeTypeDetected       */ << QByteArrayLiteral("application/zip")
        /* fileDisposition            */ << QByteArrayLiteral("")
        /* fileHasReferer             */ << true
        /* fileAction                 */ << FileIsDownloaded;
}

void tst_QWebEngineDownloadRequest::downloadLink()
{
    QFETCH(UserAction, userAction);
    QFETCH(bool, anchorHasDownloadAttribute);
    QFETCH(QByteArray, fileName);
    QFETCH(QByteArray, fileContents);
    QFETCH(QByteArray, fileMimeTypeDeclared);
    QFETCH(QByteArray, fileMimeTypeDetected);
    QFETCH(QByteArray, fileDisposition);
    QFETCH(bool, fileHasReferer);
    QFETCH(FileAction, fileAction);

    // Set up HTTP server
    int indexRequestCount = 0;
    int fileRequestCount = 0;
    QByteArray fileRequestReferer;
    ScopedConnection sc1 = connect(m_server, &HttpServer::newRequest, [&](HttpReqRep *rr) {
        if (rr->requestMethod() == "GET" && rr->requestPath() == "/") {
            indexRequestCount++;

            rr->setResponseHeader(QByteArrayLiteral("content-type"), QByteArrayLiteral("text/html"));
            QByteArray html;
            html += QByteArrayLiteral("<html><body><a href=\"");
            html += fileName;
            html += QByteArrayLiteral("\" ");
            if (anchorHasDownloadAttribute)
                html += QByteArrayLiteral("download");
            html += QByteArrayLiteral(">Link</a></body></html>");
            rr->setResponseBody(html);
            rr->sendResponse();
        } else if (rr->requestMethod() == "GET" && rr->requestPath() == "/" + fileName) {
            fileRequestCount++;
            fileRequestReferer = rr->requestHeader(QByteArrayLiteral("referer"));

            if (!fileDisposition.isEmpty())
                rr->setResponseHeader(QByteArrayLiteral("content-disposition"), fileDisposition);
            if (!fileMimeTypeDeclared.isEmpty())
                rr->setResponseHeader(QByteArrayLiteral("content-type"), fileMimeTypeDeclared);
            rr->setResponseBody(fileContents);
            rr->sendResponse();
        } else {
            rr->sendResponse(404);
        }
    });

    // Set up profile and download handler
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    QByteArray slashFileName = QByteArrayLiteral("/") + fileName;
    QString suggestedPath =
        QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + slashFileName;
    QString downloadDirectory = tmpDir.path();
    QString downloadFileName = fileName;
    QString downloadPath = tmpDir.path() + slashFileName;
    QUrl downloadUrl = m_server->url(slashFileName);
    int acceptedCount = 0;
    int finishedCount = 0;
    ScopedConnection sc2 = connect(m_profile, &QWebEngineProfile::downloadRequested, [&](QWebEngineDownloadRequest *item) {
        QCOMPARE(item->state(), QWebEngineDownloadRequest::DownloadRequested);
        QCOMPARE(item->isFinished(), false);
        QCOMPARE(item->totalBytes(), fileContents.size());
        QCOMPARE(item->receivedBytes(), 0);
        QCOMPARE(item->interruptReason(), QWebEngineDownloadRequest::NoReason);
        QCOMPARE(item->isSavePageDownload(), false);
        QCOMPARE(item->mimeType(), QString(fileMimeTypeDetected));
        QCOMPARE(QDir(item->downloadDirectory()).filePath(item->downloadFileName()), suggestedPath);
        QCOMPARE(item->savePageFormat(), QWebEngineDownloadRequest::UnknownSaveFormat);
        QCOMPARE(item->url(), downloadUrl);
        QCOMPARE(item->page(), m_page);

        connect(item, &QWebEngineDownloadRequest::isFinishedChanged, [&, item]() {
            QCOMPARE(item->state(), QWebEngineDownloadRequest::DownloadCompleted);
            QCOMPARE(item->isFinished(), true);
            QCOMPARE(item->totalBytes(), fileContents.size());
            QCOMPARE(item->receivedBytes(), fileContents.size());
            QCOMPARE(item->interruptReason(), QWebEngineDownloadRequest::NoReason);
            QCOMPARE(item->isSavePageDownload(), false);
            QCOMPARE(item->mimeType(), QString(fileMimeTypeDetected));
            QCOMPARE(QDir(item->downloadDirectory()).filePath(item->downloadFileName()), downloadPath);
            QCOMPARE(item->savePageFormat(), QWebEngineDownloadRequest::UnknownSaveFormat);
            QCOMPARE(item->url(), downloadUrl);
            QCOMPARE(item->page(), m_page);

            finishedCount++;
        });
        item->setDownloadDirectory(downloadDirectory);
        item->setDownloadFileName(downloadFileName);
        item->accept();

        acceptedCount++;
    });

    // Load an HTML page with a link
    //
    // The only variation being whether the <a> element has a "download"
    // attribute or not.
    QSignalSpy loadSpy(m_page, &QWebEnginePage::loadFinished);
    m_view->load(m_server->url());
    QTRY_COMPARE(loadSpy.size(), 1);
    QCOMPARE(loadSpy.takeFirst().value(0).toBool(), true);
    QCOMPARE(indexRequestCount, 1);

    simulateUserAction(QPoint(10, 10), userAction);

    // If file is expected to be displayed and not downloaded then end test
    if (fileAction == FileIsDisplayed) {
        QTRY_COMPARE(loadSpy.size(), 1);
        QCOMPARE(loadSpy.takeFirst().value(0).toBool(), true);
        QCOMPARE(acceptedCount, 0);
        return;
    }

    // Otherwise file is downloaded
    QTRY_COMPARE(acceptedCount, 1);
    QTRY_COMPARE(finishedCount, 1);
    QCOMPARE(fileRequestCount, 1);
    if (fileHasReferer)
        QCOMPARE(fileRequestReferer, m_server->url().toEncoded());
    else
        QCOMPARE(fileRequestReferer, QByteArray());
    QFile file(downloadPath);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QCOMPARE(file.readAll(), fileContents);
}

void tst_QWebEngineDownloadRequest::downloadTwoLinks_data()
{
    QTest::addColumn<UserAction>("action1");
    QTest::addColumn<UserAction>("action2");
    QTest::newRow("Save+Save") << SaveLink << SaveLink;
    QTest::newRow("Save+Click") << SaveLink << ClickLink;
    QTest::newRow("Click+Save") << ClickLink << SaveLink;
    QTest::newRow("Click+Click") << ClickLink << ClickLink;
}

void tst_QWebEngineDownloadRequest::downloadTwoLinks()
{
    QFETCH(UserAction, action1);
    QFETCH(UserAction, action2);

    // Set up HTTP server
    int file1RequestCount = 0;
    int file2RequestCount = 0;
    ScopedConnection sc1 = connect(m_server, &HttpServer::newRequest, [&](HttpReqRep *rr) {
        if (rr->requestMethod() == "GET" && rr->requestPath() == "/") {
            rr->setResponseHeader(QByteArrayLiteral("content-type"), QByteArrayLiteral("text/html"));
            rr->setResponseBody(QByteArrayLiteral("<html><body><a href=\"file1\" download>Link1</a><br/><a href=\"file2\">Link2</a></body></html>"));
            rr->sendResponse();
        } else if (rr->requestMethod() == "GET" && rr->requestPath() == "/file1") {
            file1RequestCount++;
            rr->setResponseHeader(QByteArrayLiteral("content-type"), QByteArrayLiteral("text/plain"));
            rr->setResponseBody(QByteArrayLiteral("file1"));
            rr->sendResponse();
        } else if (rr->requestMethod() == "GET" && rr->requestPath() == "/file2") {
            file2RequestCount++;
            rr->setResponseHeader(QByteArrayLiteral("content-type"), QByteArrayLiteral("text/plain"));
            rr->setResponseHeader(QByteArrayLiteral("content-disposition"), QByteArrayLiteral("attachment"));
            rr->setResponseBody(QByteArrayLiteral("file2"));
            rr->sendResponse();
        }
    });

    // Set up profile and download handler
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    QString standardDir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    int acceptedCount = 0;
    int finishedCount = 0;
    ScopedConnection sc2 = connect(m_profile, &QWebEngineProfile::downloadRequested, [&](QWebEngineDownloadRequest *item) {
        QCOMPARE(item->state(), QWebEngineDownloadRequest::DownloadRequested);
        QCOMPARE(item->isFinished(), false);
        QCOMPARE(item->totalBytes(), 5); // strlen("fileN")
        QCOMPARE(item->receivedBytes(), 0);
        QCOMPARE(item->interruptReason(), QWebEngineDownloadRequest::NoReason);
        QCOMPARE(item->savePageFormat(), QWebEngineDownloadRequest::UnknownSaveFormat);
        QCOMPARE(item->mimeType(), QStringLiteral("text/plain"));
        QString filePart = QChar('/') + item->url().fileName();
        QString fileName =  item->url().fileName();
        QCOMPARE(QDir(item->downloadDirectory()).filePath(item->downloadFileName()), standardDir + filePart);

        // type() is broken due to race condition in DownloadManagerDelegateQt
        if (action1 == ClickLink && action2 == ClickLink && filePart != QStringLiteral("/file1") && filePart != QStringLiteral("/file2"))
                QFAIL(qPrintable("Unexpected file name: " + filePart));

        connect(item, &QWebEngineDownloadRequest::isFinishedChanged, [&]() {
            finishedCount++;
        });
        item->setDownloadDirectory(tmpDir.path());
        item->setDownloadFileName(fileName);
        item->accept();

        acceptedCount++;
    });

    QSignalSpy loadSpy(m_page, &QWebEnginePage::loadFinished);
    m_view->load(m_server->url());
    QTRY_COMPARE(loadSpy.size(), 1);
    QCOMPARE(loadSpy.takeFirst().value(0).toBool(), true);

    // Trigger downloads
    simulateUserAction(QPoint(10, 10), action1);
    simulateUserAction(QPoint(10, 30), action2);

    // Wait for downloads
    QTRY_COMPARE(file1RequestCount, 1);
    QTRY_COMPARE(file2RequestCount, 1);
    QTRY_COMPARE(acceptedCount, 2);
    QTRY_COMPARE(finishedCount, 2);
}

void tst_QWebEngineDownloadRequest::downloadPage_data()
{
    QTest::addColumn<bool>("saveWithPageAction");
    QTest::addColumn<QWebEngineDownloadRequest::SavePageFormat>("savePageFormat");
    QTest::newRow("SingleHtmlSaveFormat")   << false << QWebEngineDownloadRequest::SingleHtmlSaveFormat;
    QTest::newRow("CompleteHtmlSaveFormat") << false << QWebEngineDownloadRequest::CompleteHtmlSaveFormat;
    QTest::newRow("MimeHtmlSaveFormat")     << false << QWebEngineDownloadRequest::MimeHtmlSaveFormat;
    QTest::newRow("SavePageAction")         << true  << QWebEngineDownloadRequest::MimeHtmlSaveFormat;
}

void tst_QWebEngineDownloadRequest::downloadPage()
{
    QFETCH(bool, saveWithPageAction);
    QFETCH(QWebEngineDownloadRequest::SavePageFormat, savePageFormat);

    // Set up HTTP server
    int indexRequestCount = 0;
    ScopedConnection sc1 = connect(m_server, &HttpServer::newRequest, [&](HttpReqRep *rr) {
        if (rr->requestMethod() == "GET" && rr->requestPath() == "/") {
            indexRequestCount++;
            rr->setResponseHeader(QByteArrayLiteral("content-type"), QByteArrayLiteral("text/html"));
            rr->setResponseBody(QByteArrayLiteral("<html><body>Hello</body></html>"));
            rr->sendResponse();
        }
    });

    // Set up profile and download handler
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    QString downloadFileName("test.html"), downloadPath = tmpDir.filePath(downloadFileName);
    QUrl downloadUrl = m_server->url("/");
    int acceptedCount = 0;
    int finishedCount = 0;
    ScopedConnection sc2 = connect(m_profile, &QWebEngineProfile::downloadRequested, [&](QWebEngineDownloadRequest *item) {
        QCOMPARE(item->state(), saveWithPageAction ? QWebEngineDownloadRequest::DownloadRequested : QWebEngineDownloadRequest::DownloadInProgress);
        QCOMPARE(item->isFinished(), false);
        QCOMPARE(item->totalBytes(), -1);
        QCOMPARE(item->receivedBytes(), 0);
        QCOMPARE(item->interruptReason(), QWebEngineDownloadRequest::NoReason);
        QCOMPARE(item->isSavePageDownload(), true);
        // FIXME(juvaldma): why is mimeType always the same?
        QCOMPARE(item->mimeType(), QStringLiteral("application/x-mimearchive"));
        QCOMPARE(item->savePageFormat(), savePageFormat);
        QCOMPARE(item->url(), downloadUrl);
        QCOMPARE(item->page(), m_page);

        if (saveWithPageAction) {
            QVERIFY(!item->downloadDirectory().isEmpty());
            QVERIFY(!item->downloadFileName().isEmpty());
            item->setDownloadDirectory(tmpDir.path());
            item->setDownloadFileName(downloadFileName);
            item->accept();
        } // save with explicit path accepts download automatically

        QCOMPARE(QDir(item->downloadDirectory()).filePath(item->downloadFileName()), downloadPath);

        connect(item, &QWebEngineDownloadRequest::isFinishedChanged, [&, item]() {
            QCOMPARE(item->state(), QWebEngineDownloadRequest::DownloadCompleted);
            QCOMPARE(item->isFinished(), true);
            QCOMPARE(item->totalBytes(), item->receivedBytes());
            QVERIFY(item->receivedBytes() > 0);
            QCOMPARE(item->interruptReason(), QWebEngineDownloadRequest::NoReason);
            QCOMPARE(item->isSavePageDownload(), true);
            QCOMPARE(item->mimeType(), QStringLiteral("application/x-mimearchive"));
            QCOMPARE(QDir(item->downloadDirectory()).filePath(item->downloadFileName()), downloadPath);
            QCOMPARE(item->savePageFormat(), savePageFormat);
            QCOMPARE(item->url(), downloadUrl);
            QCOMPARE(item->page(), m_page);

            finishedCount++;
        });

        acceptedCount++;
    });

    // Load some HTML
    QSignalSpy loadSpy(m_page, &QWebEnginePage::loadFinished);
    m_page->load(m_server->url());
    QTRY_COMPARE(loadSpy.size(), 1);
    QCOMPARE(loadSpy.takeFirst().value(0).toBool(), true);
    QCOMPARE(indexRequestCount, 1);

    // Save some HTML
    if (saveWithPageAction)
        m_page->triggerAction(QWebEnginePage::SavePage);
    else
        m_page->save(downloadPath, savePageFormat);

    QTRY_COMPARE(acceptedCount, 1);
    QTRY_COMPARE(finishedCount, 1);
    QFile file(downloadPath);
    QVERIFY(file.exists());
}

void tst_QWebEngineDownloadRequest::downloadViaSetUrl()
{
    // Reproduce the scenario described in QTBUG-63388 by triggering downloads
    // of the same file multiple times via QWebEnginePage::setUrl

    // Set up HTTP server
    ScopedConnection sc1 = connect(m_server, &HttpServer::newRequest, [&](HttpReqRep *rr) {
        if (rr->requestMethod() == "GET" && rr->requestPath() == "/") {
            rr->setResponseHeader(QByteArrayLiteral("content-type"), QByteArrayLiteral("text/html"));
            rr->setResponseBody(QByteArrayLiteral("<html><body>Hello</body></html>"));
            rr->sendResponse();
        } else if (rr->requestMethod() == "GET" && rr->requestPath() == "/file") {
            rr->setResponseHeader(QByteArrayLiteral("content-disposition"), QByteArrayLiteral("attachment"));
            rr->setResponseBody(QByteArrayLiteral("redacted"));
            rr->sendResponse();
        }
    });

    // Set up profile and download handler
    QList<QUrl> downloadUrls;
    ScopedConnection sc2 = connect(m_profile, &QWebEngineProfile::downloadRequested, [&](QWebEngineDownloadRequest *item) {
        downloadUrls.append(item->url());
    });

    // Set up the test scenario by trying to load some unrelated HTML.
    QSignalSpy loadSpy(m_page, &QWebEnginePage::loadFinished);
    QSignalSpy urlSpy(m_page, &QWebEnginePage::urlChanged);
    const QUrl indexUrl = m_server->url();
    m_page->setUrl(indexUrl);
    QTRY_COMPARE(loadSpy.size(), 1);
    QTRY_COMPARE(urlSpy.size(), 1);
    QCOMPARE(loadSpy.takeFirst().value(0).toBool(), true);
    QCOMPARE(urlSpy.takeFirst().value(0).toUrl(), indexUrl);

    // Download files via setUrl. With QTBUG-63388 after the first iteration the
    // downloads would be triggered for indexUrl and not fileUrl.
    const QUrl fileUrl = m_server->url(QByteArrayLiteral("/file"));
    for (int i = 0; i != 3; ++i) {
        m_page->setUrl(fileUrl);
        QCOMPARE(m_page->url(), fileUrl);
        QTRY_COMPARE(loadSpy.size(), 1);
        QTRY_COMPARE(urlSpy.size(), 2);
        QTRY_COMPARE(downloadUrls.size(), 1);
        QCOMPARE(loadSpy.takeFirst().value(0).toBool(), false);
        QCOMPARE(urlSpy.takeFirst().value(0).toUrl(), fileUrl);
        QCOMPARE(urlSpy.takeFirst().value(0).toUrl(), indexUrl);
        QCOMPARE(downloadUrls.takeFirst(), fileUrl);
        QCOMPARE(m_page->url(), indexUrl);
    }
}

void tst_QWebEngineDownloadRequest::downloadFileNot1()
{
    // Trigger file download via download() but don't accept().

    ScopedConnection sc1 = connect(m_server, &HttpServer::newRequest, [&](HttpReqRep *rr) {
        rr->sendResponse(404);
    });

    QPointer<QWebEngineDownloadRequest> downloadItem;
    int downloadCount = 0;
    ScopedConnection sc2 = connect(m_profile, &QWebEngineProfile::downloadRequested, [&](QWebEngineDownloadRequest *item) {
        QVERIFY(item);
        QCOMPARE(item->state(), QWebEngineDownloadRequest::DownloadRequested);
        downloadItem = item;
        downloadCount++;
    });

    m_page->download(m_server->url(QByteArrayLiteral("/file")));
    QTRY_COMPARE(downloadCount, 1);
    QVERIFY(!downloadItem);
}

void tst_QWebEngineDownloadRequest::downloadFileNot2()
{
    // Trigger file download via download() but call cancel() instead of accept().

    ScopedConnection sc1 = connect(m_server, &HttpServer::newRequest, [&](HttpReqRep *rr) {
        rr->sendResponse(404);
    });

    QPointer<QWebEngineDownloadRequest> downloadItem;
    int downloadCount = 0;
    ScopedConnection sc2 = connect(m_profile, &QWebEngineProfile::downloadRequested, [&](QWebEngineDownloadRequest *item) {
        QVERIFY(item);
        QCOMPARE(item->state(), QWebEngineDownloadRequest::DownloadRequested);
        item->cancel();
        downloadItem = item;
        downloadCount++;
    });

    m_page->download(m_server->url(QByteArrayLiteral("/file")));
    QTRY_COMPARE(downloadCount, 1);
    QVERIFY(downloadItem);
    QCOMPARE(downloadItem->state(), QWebEngineDownloadRequest::DownloadCancelled);
}

void tst_QWebEngineDownloadRequest::downloadDeleted()
{
    QPointer<QWebEngineDownloadRequest> downloadItem;
    int downloadCount = 0, finishedCount = 0;

    ScopedConnection sc2 = connect(m_profile, &QWebEngineProfile::downloadRequested, [&](QWebEngineDownloadRequest *item) {
        QVERIFY(item);
        QCOMPARE(item->state(), QWebEngineDownloadRequest::DownloadRequested);
        downloadItem = item;
        connect(downloadItem, &QWebEngineDownloadRequest::isFinishedChanged, [&]() { ++finishedCount; });
        ++downloadCount;
        // accept and schedule deletion, and check if it still finishes
        item->accept();
        item->deleteLater();
        QVERIFY(downloadItem);
    });

    m_page->download(m_server->url(QByteArrayLiteral("/file")));
    QTRY_COMPARE(downloadCount, 1);
    QTRY_COMPARE(finishedCount, 1);
    QTRY_VERIFY(!downloadItem);
    QCOMPARE(downloadCount, 1);
    QCOMPARE(finishedCount, 1);
}

void tst_QWebEngineDownloadRequest::downloadDeletedByProfile()
{
    QPointer<QWebEngineProfile> profile(new QWebEngineProfile);
    profile->setHttpCacheType(QWebEngineProfile::NoCache);
    profile->settings()->setAttribute(QWebEngineSettings::AutoLoadIconsForPage, false);

    bool downloadFinished = false;
    QPointer<QWebEngineDownloadRequest> downloadItem;
    connect(profile, &QWebEngineProfile::downloadRequested, [&] (QWebEngineDownloadRequest *item) {
        connect(item, &QWebEngineDownloadRequest::isFinishedChanged, [&] () {
            downloadFinished = true;
        });
        downloadItem = item;
        item->accept();
    });

    QPointer<QWebEnginePage> page(new QWebEnginePage(profile));
    page->download(m_server->url(QByteArrayLiteral("/file")));

    QTRY_COMPARE(downloadItem.isNull(), false);
    QVERIFY(downloadItem);

    page->deleteLater();
    profile->deleteLater();

    QTRY_COMPARE(downloadFinished, true);
    QTRY_COMPARE(downloadItem.isNull(), true);
}

void tst_QWebEngineDownloadRequest::downloadUniqueFilename_data()
{
    QTest::addColumn<QString>("baseName");
    QTest::addColumn<QString>("extension");

    QTest::newRow("txt") << QString("test(1.test)") << QString("txt");
    QTest::newRow("tar.gz") << QString("test(1.test)") << QString("tar.gz");
}

void tst_QWebEngineDownloadRequest::downloadUniqueFilename()
{
    QFETCH(QString, baseName);
    QFETCH(QString, extension);
    QString fileName = QString("%1.%2").arg(baseName).arg(extension);
    QString downloadedFilePath;
    QString suggestedFileName;
    bool downloadFinished = false;

    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    m_profile->setDownloadPath(tmpDir.path());

    // Set up HTTP server
    ScopedConnection sc1 = connect(m_server, &HttpServer::newRequest, [&](HttpReqRep *rr) {
        auto requestPath = QString::fromUtf8(rr->requestPath());
        if (rr->requestMethod() == "GET" && requestPath == ("/" + fileName)) {
            rr->setResponseHeader(QByteArrayLiteral("content-type"), QByteArrayLiteral("application/octet-stream"));
            rr->setResponseHeader(QByteArrayLiteral("content-disposition"), QByteArrayLiteral("attachment"));
            rr->setResponseBody(QByteArrayLiteral("a"));
            rr->sendResponse();
        } else {
            rr->sendResponse(404);
        }
    });

    // Set up profile and download handler
    ScopedConnection sc2 = connect(m_profile, &QWebEngineProfile::downloadRequested, [&](QWebEngineDownloadRequest *item) {
        suggestedFileName = item->suggestedFileName();
        item->accept();
        connect(item, &QWebEngineDownloadRequest::isFinishedChanged, [&, item]() {
            QCOMPARE(item->state(), QWebEngineDownloadRequest::DownloadCompleted);
            QCOMPARE(item->isFinished(), true);
            QCOMPARE(item->totalBytes(), item->receivedBytes());
            QVERIFY(item->receivedBytes() > 0);
            QCOMPARE(item->interruptReason(), QWebEngineDownloadRequest::NoReason);
            QCOMPARE(item->isSavePageDownload(), false);
            downloadedFilePath = QDir(item->downloadDirectory()).filePath(item->downloadFileName());
            downloadFinished = true;
        });
    });

    m_page->setUrl(m_server->url("/" + fileName));
    QTRY_VERIFY(downloadFinished);
    QVERIFY(QFile(downloadedFilePath).exists());
    QCOMPARE(downloadedFilePath, m_profile->downloadPath() + "/" + baseName + "." + extension);

    for (int i = 1; i <= 2; ++i) {
        downloadFinished = false;
        m_page->setUrl(m_server->url("/" + fileName));
        QTRY_VERIFY(downloadFinished);
        QVERIFY(QFile(downloadedFilePath).exists());
        QCOMPARE(downloadedFilePath, m_profile->downloadPath() + "/" + baseName + " (" + QString::number(i) + ")." + extension);
        QCOMPARE(suggestedFileName, fileName);
    }
}

void tst_QWebEngineDownloadRequest::downloadUniqueFilenameWithTimestamp()
{
    // Set up HTTP server
    QString baseName("test(1.test)");
    QString extension("txt");
    QString fileName = QString("%1.%2").arg(baseName).arg(extension);
    QString downloadedFilePath;
    QString suggestedFileName;
    bool downloadFinished = false;

    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    m_profile->setDownloadPath(tmpDir.path());

    ScopedConnection sc1 = connect(m_server, &HttpServer::newRequest, [&](HttpReqRep *rr) {
        auto requestPath = QString::fromUtf8(rr->requestPath());
        if (rr->requestMethod() == "GET" && requestPath == ("/" + fileName)) {
            rr->setResponseHeader(QByteArrayLiteral("content-type"), QByteArrayLiteral("application/octet-stream"));
            rr->setResponseHeader(QByteArrayLiteral("content-disposition"), QByteArrayLiteral("attachment"));
            rr->setResponseBody(QByteArrayLiteral("a"));
            rr->sendResponse();
        } else {
            rr->sendResponse(404);
        }
    });

    // Set up profile and download handler
    ScopedConnection sc2 = connect(m_profile, &QWebEngineProfile::downloadRequested, [&](QWebEngineDownloadRequest *item) {
        suggestedFileName = item->suggestedFileName();
        item->accept();
        connect(item, &QWebEngineDownloadRequest::isFinishedChanged, [&, item]() {
            QCOMPARE(item->state(), QWebEngineDownloadRequest::DownloadCompleted);
            QCOMPARE(item->isFinished(), true);
            QCOMPARE(item->totalBytes(), item->receivedBytes());
            QVERIFY(item->receivedBytes() > 0);
            QCOMPARE(item->interruptReason(), QWebEngineDownloadRequest::NoReason);
            QCOMPARE(item->page(), m_page);
            downloadFinished = true;
            downloadedFilePath = QDir(item->downloadDirectory()).filePath(item->downloadFileName());
        });
    });

    // Create the first empty file without uniquifier.
    {
        QFile file(m_profile->downloadPath() + "/" + fileName);
        file.open(QIODevice::ReadWrite);
    }

    // Create 99 empty files with uniquifier.
    for (int i = 1; i < 100; i++) {
        QFile file(m_profile->downloadPath() + "/" + baseName + " (" + QString::number(i) + ")." + extension);
        file.open(QIODevice::ReadWrite);
    }

    // Create 100th (kMaxUniqueFiles) empty file with uniquifier.
    m_page->setUrl(m_server->url("/" + fileName));
    QTRY_VERIFY(downloadFinished);
    QVERIFY(QFile(downloadedFilePath).exists());
    QCOMPARE(downloadedFilePath, m_profile->downloadPath() + "/" + baseName + " (100)." + extension);
    QCOMPARE(suggestedFileName, fileName);

    // Check if the downloaded files are suffixed with timestamp after the 100th download.
    for (int i = 101; i < 103; i++) {
        downloadFinished = false;
        m_page->setUrl(m_server->url("/" + fileName));
        QTRY_VERIFY(downloadFinished);
        QVERIFY(QFile(downloadedFilePath).exists());
        QRegularExpression fileNameCheck("^.*" + QRegularExpression::escape(baseName) + " - (.*)[.]" + QRegularExpression::escape(extension) + "$");
        QRegularExpressionMatch match = fileNameCheck.match(downloadedFilePath);
        QVERIFY(match.hasMatch());
        // ISO 8601 Date and time in UTC
        QRegularExpression timestamp("^(-?(?:[1-9][0-9]*)?[0-9]{4})-(1[0-2]|0[1-9])-(3[01]|0[1-9]|["
                                     "12][0-9])T(2[0-3]|[01][0-9])([0-5][0-9])([0-5][0-9])([.][0-9]"
                                     "+)?(Z|[+-](?:2[0-3]|[01][0-9])[0-5][0-9])?$");
        QVERIFY(timestamp.match(match.captured(1)).hasMatch());
        QCOMPARE(suggestedFileName, fileName);
    }
}

void tst_QWebEngineDownloadRequest::downloadToDefaultLocation()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());

    QCOMPARE(m_profile->downloadPath(), QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));

    m_profile->setDownloadPath("");
    QCOMPARE(m_profile->downloadPath(),  QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));

    m_profile->setDownloadPath(tmpDir.path());
    QCOMPARE(m_profile->downloadPath(), tmpDir.path());

    m_profile->setDownloadPath(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
    QCOMPARE(m_profile->downloadPath(), QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
}

void tst_QWebEngineDownloadRequest::downloadToNonExistentDir()
{
    QString baseName("test(1.test)");
    QString extension("txt");
    QString fileName = QString("%1.%2").arg(baseName).arg(extension);
    QString downloadedFilePath;
    QString suggestedFileName;
    bool downloadFinished = false;

    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    m_profile->setDownloadPath(tmpDir.path());

    // Set up HTTP server
    ScopedConnection sc1 = connect(m_server, &HttpServer::newRequest, [&](HttpReqRep *rr) {
        auto requestPath = QString::fromUtf8(rr->requestPath());
        if (rr->requestMethod() == "GET" && requestPath == ("/" + fileName)) {
            rr->setResponseHeader(QByteArrayLiteral("content-type"), QByteArrayLiteral("application/octet-stream"));
            rr->setResponseHeader(QByteArrayLiteral("content-disposition"), QByteArrayLiteral("attachment"));
            rr->setResponseBody(QByteArrayLiteral("a"));
            rr->sendResponse();
        } else {
            rr->sendResponse(404);
        }
    });

    // Set up profile and download handler
    ScopedConnection sc2 = connect(m_profile, &QWebEngineProfile::downloadRequested, [&](QWebEngineDownloadRequest *item) {
        suggestedFileName = item->suggestedFileName();
        item->accept();
        connect(item, &QWebEngineDownloadRequest::isFinishedChanged, [&, item]() {
            QCOMPARE(item->state(), QWebEngineDownloadRequest::DownloadCompleted);
            QCOMPARE(item->isFinished(), true);
            QCOMPARE(item->totalBytes(), item->receivedBytes());
            QVERIFY(item->receivedBytes() > 0);
            QCOMPARE(item->interruptReason(), QWebEngineDownloadRequest::NoReason);
            QCOMPARE(item->page(), m_page);
            downloadFinished = true;
            downloadedFilePath = QDir(item->downloadDirectory()).filePath(item->downloadFileName());
        });
    });

    // Set a non-existent directory for the default download location.
    QString nonExistentDownloadPath(m_profile->downloadPath() + "/non_existent_dir");
    m_profile->setDownloadPath(nonExistentDownloadPath);
    QCOMPARE(m_profile->downloadPath(), nonExistentDownloadPath);
    m_page->setUrl(m_server->url("/" + fileName));
    QTRY_VERIFY(downloadFinished);
    QVERIFY(QFile(downloadedFilePath).exists());
    QCOMPARE(downloadedFilePath, nonExistentDownloadPath + "/" + fileName);
    QCOMPARE(suggestedFileName, fileName);
}

void tst_QWebEngineDownloadRequest::downloadToReadOnlyDir()
{
#ifdef Q_OS_WIN
    QSKIP("Cannot change file permissions on Windows.");
#endif
    QString baseName("test(1.test)");
    QString extension("txt");
    QString fileName = QString("%1.%2").arg(baseName).arg(extension);
    QString downloadedFilePath;
    QString suggestedFileName;
    bool downloadAccepted = false;
    bool downloadFinished = false;

    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    m_profile->setDownloadPath(tmpDir.path());

    // Set up HTTP server
    ScopedConnection sc1 = connect(m_server, &HttpServer::newRequest, [&](HttpReqRep *rr) {
        auto requestPath = QString::fromUtf8(rr->requestPath());
        if (rr->requestMethod() == "GET" && requestPath == ("/" + fileName)) {
            rr->setResponseHeader(QByteArrayLiteral("content-type"), QByteArrayLiteral("application/octet-stream"));
            rr->setResponseHeader(QByteArrayLiteral("content-disposition"), QByteArrayLiteral("attachment"));
            rr->setResponseBody(QByteArrayLiteral("a"));
            rr->sendResponse();
        } else {
            rr->sendResponse(404);
        }
    });

    QPointer<QWebEngineDownloadRequest> downloadItem;
    ScopedConnection sc2 = connect(m_profile, &QWebEngineProfile::downloadRequested, [&](QWebEngineDownloadRequest *item) {
        suggestedFileName = item->suggestedFileName();
        downloadItem = item;
        item->accept();
        connect(item, &QWebEngineDownloadRequest::isFinishedChanged, [&]() {
            downloadFinished = true;
        });
        downloadAccepted = true;
    });

    // Change permission for directory.
    QFile(m_profile->downloadPath()).setPermissions(QFileDevice::ReadOwner);
    QVERIFY(QFile(m_profile->downloadPath()).exists());

    m_page->setUrl(m_server->url("/" + fileName));
    QTRY_VERIFY(downloadAccepted);

    QVERIFY(downloadItem);
    QTRY_COMPARE(downloadItem->state(), QWebEngineDownloadRequest::DownloadInterrupted);
    QCOMPARE(downloadItem->isFinished(), false);
    QCOMPARE(downloadItem->interruptReason(), QWebEngineDownloadRequest::FileAccessDenied);
    QVERIFY(!QFile(downloadedFilePath).exists());
    QCOMPARE(suggestedFileName, fileName);

    // Clear m_requestedDownloads explicitly because download is accepted but never finished.
    m_requestedDownloads.clear();
    QVERIFY(!downloadFinished);
    QFile(m_profile->downloadPath()).setPermissions(QFileDevice::WriteOwner);
}

void tst_QWebEngineDownloadRequest::downloadToDirectoryWithFileName_data()
{
    QTest::addColumn<bool>("setDirectoryFirst");

    QTest::newRow("setDirectoryFirst") << true;
    QTest::newRow("setFileNameFirst") << false;
}

void tst_QWebEngineDownloadRequest::downloadToDirectoryWithFileName()
{
    QFETCH(bool, setDirectoryFirst);
    QString downloadDirectory;
    QString downloadFileName;
    QString downloadedFilePath;
    QString downloadedSuggestedFileName;
    QString fileName = "test.txt";
    QString uniqueFileName = "test (1).txt";

    bool downloadFinished = false;

    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    m_profile->setDownloadPath(tmpDir.path());

    // Set up HTTP server
    ScopedConnection sc1 = connect(m_server, &HttpServer::newRequest, [&](HttpReqRep *rr) {
        auto requestPath = QString::fromUtf8(rr->requestPath());
        if (rr->requestMethod() == "GET" && requestPath == ("/" + fileName)) {
            rr->setResponseHeader(QByteArrayLiteral("content-type"), QByteArrayLiteral("application/octet-stream"));
            rr->setResponseHeader(QByteArrayLiteral("content-disposition"), QByteArrayLiteral("attachment"));
            rr->setResponseBody(QByteArrayLiteral("a"));
            rr->sendResponse();
        } else {
            rr->sendResponse(404);
        }
    });

    // Set up profile and download handler
    ScopedConnection sc2 = connect(m_profile, &QWebEngineProfile::downloadRequested, [&](QWebEngineDownloadRequest *item) {
        QSignalSpy fileNameSpy(item, &QWebEngineDownloadRequest::downloadFileNameChanged);
        QSignalSpy directorySpy(item, &QWebEngineDownloadRequest::downloadDirectoryChanged);
        bool isUniquifiedFileName = false;
        if (!downloadDirectory.isEmpty() && setDirectoryFirst) {
            const QString &originalFileName = item->downloadFileName();
            item->setDownloadDirectory(downloadDirectory);
            QCOMPARE(item->downloadDirectory(), downloadDirectory);
            QCOMPARE(directorySpy.size(), 1);
            isUniquifiedFileName = (originalFileName != item->downloadFileName());
            QCOMPARE(fileNameSpy.size(), isUniquifiedFileName ? 1 : 0);
        }

        if (!downloadFileName.isEmpty()) {
            item->setDownloadFileName(downloadFileName);
            QCOMPARE(item->downloadFileName(), downloadFileName);
            QCOMPARE(fileNameSpy.size(), isUniquifiedFileName ? 2 : 1);
        }

        if (!downloadDirectory.isEmpty() && !setDirectoryFirst) {
            item->setDownloadDirectory(downloadDirectory);
            QCOMPARE(item->downloadDirectory(), downloadDirectory);
            QCOMPARE(directorySpy.size(), 1);
        }

        item->accept();

        connect(item, &QWebEngineDownloadRequest::isFinishedChanged, [&, item]() {
            QCOMPARE(item->state(), QWebEngineDownloadRequest::DownloadCompleted);
            QCOMPARE(item->isFinished(), true);
            QCOMPARE(item->totalBytes(), item->receivedBytes());
            QVERIFY(item->receivedBytes() > 0);
            QCOMPARE(item->interruptReason(), QWebEngineDownloadRequest::NoReason);
            QCOMPARE(item->page(), m_page);
            downloadFinished = true;
            downloadedFilePath = QDir(item->downloadDirectory()).filePath(item->downloadFileName());
            downloadedSuggestedFileName = item->suggestedFileName();
        });
    });

    // Download file to the default download directory.
    downloadDirectory = "";
    downloadFileName = "";
    m_page->setUrl(m_server->url("/" + fileName));
    QTRY_VERIFY(downloadFinished);
    QVERIFY(QFile(downloadedFilePath).exists());
    QCOMPARE(downloadedFilePath, QDir(m_profile->downloadPath()).filePath(fileName));
    QCOMPARE(downloadedSuggestedFileName, fileName);

    // Download the same file to another directory
    downloadFinished = false;
    downloadDirectory = m_profile->downloadPath() + QDir::separator() + "test1"  + QDir::separator();
    downloadFileName = "";
    m_page->setUrl(m_server->url("/" + fileName));
    QTRY_VERIFY(downloadFinished);
    QVERIFY(QFile(downloadedFilePath).exists());
    QCOMPARE(downloadedFilePath, QDir(downloadDirectory).filePath(fileName));
    QCOMPARE(downloadedSuggestedFileName, fileName);

    // Download the same file to the same directory and the file name must be unique.
    downloadFinished = false;
    downloadDirectory = m_profile->downloadPath() + QDir::separator() + "test1"  + QDir::separator();
    downloadFileName = "";
    m_page->setUrl(m_server->url("/" + fileName));
    QTRY_VERIFY(downloadFinished);
    QVERIFY(QFile(downloadedFilePath).exists());
    QCOMPARE(downloadedFilePath, QDir(downloadDirectory).filePath(uniqueFileName));
    QCOMPARE(downloadedSuggestedFileName, fileName);

    // Download another file to the same directory and set file name by
    // QWebEngineDownloadRequest::setDownloadDirectory() and setDownloadFileName() to avoid uniquification.
    downloadFinished = false;
    downloadDirectory = m_profile->downloadPath() + QDir::separator() + "test1"  + QDir::separator();
    downloadFileName = "test1.txt";
    m_page->setUrl(m_server->url("/" + fileName));
    QTRY_VERIFY(downloadFinished);
    QVERIFY(QFile(downloadedFilePath).exists());
    QCOMPARE(downloadedFilePath, QDir(downloadDirectory).filePath(downloadFileName));
    QCOMPARE(downloadedSuggestedFileName, fileName);

    // Download the same file to another directory without uniquifying the file name
    downloadFinished = false;
    downloadDirectory = m_profile->downloadPath() + QDir::separator() + "test2"  + QDir::separator();
    downloadFileName = "test1.txt";
    m_page->setUrl(m_server->url("/" + fileName));
    QTRY_VERIFY(downloadFinished);
    QVERIFY(QFile(downloadedFilePath).exists());
    QCOMPARE(downloadedFilePath, QDir(downloadDirectory).filePath(downloadFileName));
    QCOMPARE(downloadedSuggestedFileName, fileName);

    // Download the same file to same directory and set file name by
    // QWebEngineDownloadRequest::setDownloadDirectory() and setDownloadFileName() to avoid uniquification.
    downloadFinished = false;
    downloadDirectory = m_profile->downloadPath() + QDir::separator() + "test2"  + QDir::separator();
    downloadFileName = "test1.txt";
    m_page->setUrl(m_server->url("/" + fileName));
    QTRY_VERIFY(downloadFinished);
    QVERIFY(QFile(downloadedFilePath).exists());
    QCOMPARE(downloadedFilePath, QDir(downloadDirectory).filePath(downloadFileName));
    QCOMPARE(downloadedSuggestedFileName, fileName);

    // Download the same file in the same directory.
    // Use the suggested file name (test.txt) and the file name will not be unique because this file name don't yet exists.
    downloadFinished = false;
    downloadDirectory = m_profile->downloadPath() + QDir::separator() + "test2"  + QDir::separator();
    downloadFileName = "";
    m_page->setUrl(m_server->url("/" + fileName));
    QTRY_VERIFY(downloadFinished);
    QVERIFY(QFile(downloadedFilePath).exists());
    QCOMPARE(downloadedFilePath, QDir(downloadDirectory).filePath(fileName));
    QCOMPARE(downloadedSuggestedFileName, fileName);
}

void tst_QWebEngineDownloadRequest::downloadDataUrls_data()
{
    QTest::addColumn<QByteArray>("htmlData");
    QTest::addColumn<QString>("expectedFileName");
    QTest::newRow("data url without slash") << QByteArrayLiteral("<html><head><meta charset=\"utf-8\"></head><body><a href=\"data:application/gzip;base64,dGVzdA==\">data URL without slash</a><br/></body></html>") << QStringLiteral("qwe_download.gz") ;
    QTest::newRow("data url with slash") << QByteArrayLiteral("<html><head><meta charset=\"utf-8\"></head><body><a href=\"data:application/gzip;base64,dGVzcnI/dGVzdA==\">data URL with filename</a><br/></body></html>") << QStringLiteral("qwe_download.gz") ;
    QTest::newRow("data url with download tag") << QByteArrayLiteral("<html><head><meta charset=\"utf-8\"></head><body><a href=\"data:application/gzip;base64,dGVzdA/IHRlc3Q=\" download=\"filename.gz\">data URL with filename</a><br/></body></html>") << QStringLiteral("filename.gz") ;

}

void tst_QWebEngineDownloadRequest::downloadDataUrls()
{
    QFETCH(QByteArray, htmlData);
    QFETCH(QString, expectedFileName);
    // Set up HTTP server
    ScopedConnection sc1 = connect(m_server, &HttpServer::newRequest, [&](HttpReqRep *rr) {
        if (rr->requestMethod() == "GET" && rr->requestPath() == "/") {
            rr->setResponseHeader(QByteArrayLiteral("content-type"), QByteArrayLiteral("text/html"));
            rr->setResponseBody(htmlData);
            rr->sendResponse();
        }
    });

    // Set up profile and download handler
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    m_profile->setDownloadPath(tmpDir.path());

    int downloadRequestCount = 0;
    ScopedConnection sc2 = connect(m_profile, &QWebEngineProfile::downloadRequested, [&](QWebEngineDownloadRequest *item) {
        QCOMPARE(item->state(), QWebEngineDownloadRequest::DownloadRequested);
        QCOMPARE(item->downloadFileName(), expectedFileName);
        downloadRequestCount++;
    });

    QSignalSpy loadSpy(m_page, &QWebEnginePage::loadFinished);
    m_view->load(m_server->url());
    QTRY_COMPARE(loadSpy.size(), 1);
    QCOMPARE(loadSpy.takeFirst().value(0).toBool(), true);

    // Trigger download
    simulateUserAction(QPoint(10, 10), UserAction::ClickLink);
    QTRY_COMPARE(downloadRequestCount, 1);
}

QTEST_MAIN(tst_QWebEngineDownloadRequest)
#include "tst_qwebenginedownloadrequest.moc"
