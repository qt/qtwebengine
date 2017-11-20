/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

#include <QCoreApplication>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>
#include <QWebEngineDownloadItem>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineView>
#include <httpserver.h>
#include <waitforsignal.h>

static std::unique_ptr<HttpReqRep> waitForFaviconRequest(HttpServer *server)
{
    auto rr = waitForRequest(server);
    if (!rr ||
        rr->requestMethod() != QByteArrayLiteral("GET") ||
        rr->requestPath() != QByteArrayLiteral("/favicon.ico"))
        return nullptr;
    rr->setResponseStatus(404);
    rr->sendResponse();
    return std::move(rr);
}

class tst_QWebEngineDownloads : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void downloadLink_data();
    void downloadLink();
    void downloadTwoLinks();
    void downloadPage_data();
    void downloadPage();
    void downloadViaSetUrl();
};

enum DownloadTestUserAction {
    SaveLink,
    Navigate,
};

enum DownloadTestFileAction {
    FileIsDownloaded,
    FileIsDisplayed,
};

Q_DECLARE_METATYPE(DownloadTestUserAction);
Q_DECLARE_METATYPE(DownloadTestFileAction);

void tst_QWebEngineDownloads::downloadLink_data()
{
    QTest::addColumn<DownloadTestUserAction>("userAction");
    QTest::addColumn<bool>("anchorHasDownloadAttribute");
    QTest::addColumn<QByteArray>("fileName");
    QTest::addColumn<QByteArray>("fileContents");
    QTest::addColumn<QByteArray>("fileMimeTypeDeclared");
    QTest::addColumn<QByteArray>("fileMimeTypeDetected");
    QTest::addColumn<QByteArray>("fileDisposition");
    QTest::addColumn<bool>("fileHasReferer");
    QTest::addColumn<DownloadTestFileAction>("fileAction");
    QTest::addColumn<QWebEngineDownloadItem::DownloadType>("downloadType");

    // SaveLink should always trigger a download, even for empty files.
    QTest::newRow("save link to empty file")
        /* userAction                 */ << SaveLink
        /* anchorHasDownloadAttribute */ << false
        /* fileName                   */ << QByteArrayLiteral("foo.txt")
        /* fileContents               */ << QByteArrayLiteral("")
        /* fileMimeTypeDeclared       */ << QByteArrayLiteral("")
        /* fileMimeTypeDetected       */ << QByteArrayLiteral("")
        /* fileDisposition            */ << QByteArrayLiteral("")
        /* fileHasReferer             */ << true
        /* fileAction                 */ << FileIsDownloaded
        /* downloadType               */ << QWebEngineDownloadItem::DownloadAttribute;

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
        /* fileAction                 */ << FileIsDownloaded
        /* downloadType               */ << QWebEngineDownloadItem::DownloadAttribute;

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
        /* fileAction                 */ << FileIsDownloaded
        /* downloadType               */ << QWebEngineDownloadItem::DownloadAttribute;

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
        /* fileAction                 */ << FileIsDownloaded
        /* downloadType               */ << QWebEngineDownloadItem::UserRequested;

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
        /* fileAction                 */ << FileIsDownloaded
        /* downloadType               */ << QWebEngineDownloadItem::UserRequested;

    // Navigating to an empty file should show an empty page.
    QTest::newRow("navigate to empty file")
        /* userAction                 */ << Navigate
        /* anchorHasDownloadAttribute */ << false
        /* fileName                   */ << QByteArrayLiteral("foo.txt")
        /* fileContents               */ << QByteArrayLiteral("")
        /* fileMimeTypeDeclared       */ << QByteArrayLiteral("")
        /* fileMimeTypeDetected       */ << QByteArrayLiteral("")
        /* fileDisposition            */ << QByteArrayLiteral("")
        /* fileHasReferer             */ << true
        /* fileAction                 */ << FileIsDisplayed
        /* downloadType               */ << /* unused */ QWebEngineDownloadItem::DownloadAttribute;

    // Navigating to a text file should show the text file.
    QTest::newRow("navigate to text file")
        /* userAction                 */ << Navigate
        /* anchorHasDownloadAttribute */ << false
        /* fileName                   */ << QByteArrayLiteral("foo.txt")
        /* fileContents               */ << QByteArrayLiteral("bar")
        /* fileMimeTypeDeclared       */ << QByteArrayLiteral("text/plain")
        /* fileMimeTypeDetected       */ << QByteArrayLiteral("text/plain")
        /* fileDisposition            */ << QByteArrayLiteral("")
        /* fileHasReferer             */ << true
        /* fileAction                 */ << FileIsDisplayed
        /* downloadType               */ << /* unused */ QWebEngineDownloadItem::DownloadAttribute;

    // ... unless the link has the "download" attribute: then the file should be downloaded.
    QTest::newRow("navigate to text file (attribute)")
        /* userAction                 */ << Navigate
        /* anchorHasDownloadAttribute */ << true
        /* fileName                   */ << QByteArrayLiteral("foo.txt")
        /* fileContents               */ << QByteArrayLiteral("bar")
        /* fileMimeTypeDeclared       */ << QByteArrayLiteral("text/plain")
        /* fileMimeTypeDetected       */ << QByteArrayLiteral("text/plain")
        /* fileDisposition            */ << QByteArrayLiteral("")
        /* fileHasReferer             */ << false // crbug.com/455987
        /* fileAction                 */ << FileIsDownloaded
        /* downloadType               */ << QWebEngineDownloadItem::DownloadAttribute;

    // ... same with the content disposition header save for the download type.
    QTest::newRow("navigate to text file (attachment)")
        /* userAction                 */ << Navigate
        /* anchorHasDownloadAttribute */ << false
        /* fileName                   */ << QByteArrayLiteral("foo.txt")
        /* fileContents               */ << QByteArrayLiteral("bar")
        /* fileMimeTypeDeclared       */ << QByteArrayLiteral("text/plain")
        /* fileMimeTypeDetected       */ << QByteArrayLiteral("text/plain")
        /* fileDisposition            */ << QByteArrayLiteral("attachment")
        /* fileHasReferer             */ << true
        /* fileAction                 */ << FileIsDownloaded
        /* downloadType               */ << QWebEngineDownloadItem::Attachment;

    // ... and both.
    QTest::newRow("navigate to text file (attribute+attachment)")
        /* userAction                 */ << Navigate
        /* anchorHasDownloadAttribute */ << true
        /* fileName                   */ << QByteArrayLiteral("foo.txt")
        /* fileContents               */ << QByteArrayLiteral("bar")
        /* fileMimeTypeDeclared       */ << QByteArrayLiteral("text/plain")
        /* fileMimeTypeDetected       */ << QByteArrayLiteral("text/plain")
        /* fileDisposition            */ << QByteArrayLiteral("attachment")
        /* fileHasReferer             */ << false // crbug.com/455987
        /* fileAction                 */ << FileIsDownloaded
        /* downloadType               */ << QWebEngineDownloadItem::Attachment;

    // The file's extension has no effect.
    QTest::newRow("navigate to supposed zip file")
        /* userAction                 */ << Navigate
        /* anchorHasDownloadAttribute */ << false
        /* fileName                   */ << QByteArrayLiteral("foo.zip")
        /* fileContents               */ << QByteArrayLiteral("bar")
        /* fileMimeTypeDeclared       */ << QByteArrayLiteral("")
        /* fileMimeTypeDetected       */ << QByteArrayLiteral("")
        /* fileDisposition            */ << QByteArrayLiteral("")
        /* fileHasReferer             */ << true
        /* fileAction                 */ << FileIsDisplayed
        /* downloadType               */ << /* unused */ QWebEngineDownloadItem::DownloadAttribute;

    // ... the file's mime type however does.
    QTest::newRow("navigate to supposed zip file (application/zip)")
        /* userAction                 */ << Navigate
        /* anchorHasDownloadAttribute */ << false
        /* fileName                   */ << QByteArrayLiteral("foo.zip")
        /* fileContents               */ << QByteArrayLiteral("bar")
        /* fileMimeTypeDeclared       */ << QByteArrayLiteral("application/zip")
        /* fileMimeTypeDetected       */ << QByteArrayLiteral("application/zip")
        /* fileDisposition            */ << QByteArrayLiteral("")
        /* fileHasReferer             */ << true
        /* fileAction                 */ << FileIsDownloaded
        /* downloadType               */ << QWebEngineDownloadItem::DownloadAttribute;

    // ... but we're not very picky about the exact type.
    QTest::newRow("navigate to supposed zip file (application/octet-stream)")
        /* userAction                 */ << Navigate
        /* anchorHasDownloadAttribute */ << false
        /* fileName                   */ << QByteArrayLiteral("foo.zip")
        /* fileContents               */ << QByteArrayLiteral("bar")
        /* fileMimeTypeDeclared       */ << QByteArrayLiteral("application/octet-stream")
        /* fileMimeTypeDetected       */ << QByteArrayLiteral("application/octet-stream")
        /* fileDisposition            */ << QByteArrayLiteral("")
        /* fileHasReferer             */ << true
        /* fileAction                 */ << FileIsDownloaded
        /* downloadType               */ << QWebEngineDownloadItem::DownloadAttribute;

    // empty zip file (consisting only of "end of central directory record")
    QByteArray zipFile = QByteArrayLiteral("PK\x05\x06") + QByteArray(18, 0);

    // The mime type is guessed automatically if not provided by the server.
    QTest::newRow("navigate to actual zip file")
        /* userAction                 */ << Navigate
        /* anchorHasDownloadAttribute */ << false
        /* fileName                   */ << QByteArrayLiteral("foo.zip")
        /* fileContents               */ << zipFile
        /* fileMimeTypeDeclared       */ << QByteArrayLiteral("")
        /* fileMimeTypeDetected       */ << QByteArrayLiteral("application/octet-stream")
        /* fileDisposition            */ << QByteArrayLiteral("")
        /* fileHasReferer             */ << true
        /* fileAction                 */ << FileIsDownloaded
        /* downloadType               */ << QWebEngineDownloadItem::DownloadAttribute;

    // The mime type is not guessed automatically if provided by the server.
    QTest::newRow("navigate to actual zip file (application/zip)")
        /* userAction                 */ << Navigate
        /* anchorHasDownloadAttribute */ << false
        /* fileName                   */ << QByteArrayLiteral("foo.zip")
        /* fileContents               */ << zipFile
        /* fileMimeTypeDeclared       */ << QByteArrayLiteral("application/zip")
        /* fileMimeTypeDetected       */ << QByteArrayLiteral("application/zip")
        /* fileDisposition            */ << QByteArrayLiteral("")
        /* fileHasReferer             */ << true
        /* fileAction                 */ << FileIsDownloaded
        /* downloadType               */ << QWebEngineDownloadItem::DownloadAttribute;
}

void tst_QWebEngineDownloads::downloadLink()
{
    QFETCH(DownloadTestUserAction, userAction);
    QFETCH(bool, anchorHasDownloadAttribute);
    QFETCH(QByteArray, fileName);
    QFETCH(QByteArray, fileContents);
    QFETCH(QByteArray, fileMimeTypeDeclared);
    QFETCH(QByteArray, fileMimeTypeDetected);
    QFETCH(QByteArray, fileDisposition);
    QFETCH(bool, fileHasReferer);
    QFETCH(DownloadTestFileAction, fileAction);
    QFETCH(QWebEngineDownloadItem::DownloadType, downloadType);

    HttpServer server;
    QWebEngineProfile profile;
    QWebEnginePage page(&profile);
    QWebEngineView view;
    view.setPage(&page);

    // 1. Load an HTML page with a link
    //
    // The only variation being whether the <a> element has a "download"
    // attribute or not.
    view.load(server.url());
    view.show();
    auto indexRR = waitForRequest(&server);
    QVERIFY(indexRR);
    QCOMPARE(indexRR->requestMethod(), QByteArrayLiteral("GET"));
    QCOMPARE(indexRR->requestPath(), QByteArrayLiteral("/"));
    indexRR->setResponseHeader(QByteArrayLiteral("content-type"), QByteArrayLiteral("text/html"));
    QByteArray html;
    html += QByteArrayLiteral("<html><body><a href=\"");
    html += fileName;
    html += QByteArrayLiteral("\" ");
    if (anchorHasDownloadAttribute)
        html += QByteArrayLiteral("download");
    html += QByteArrayLiteral(">Link</a></body></html>");
    indexRR->setResponseBody(html);
    indexRR->sendResponse();
    bool loadOk = false;
    QVERIFY(waitForSignal(&page, &QWebEnginePage::loadFinished, [&](bool ok) { loadOk = ok; }));
    QVERIFY(loadOk);

    // 1.1. Ignore favicon request
    auto favIconRR = waitForFaviconRequest(&server);
    QVERIFY(favIconRR);

    // 2. Simulate user action
    //
    // - Navigate: user left-clicks on link
    // - SaveLink: user right-clicks on link and chooses "save link as" from menu
    QTRY_VERIFY(view.focusWidget());
    QWidget *renderWidget = view.focusWidget();
    QPoint linkPos(10, 10);
    if (userAction == SaveLink) {
        view.setContextMenuPolicy(Qt::CustomContextMenu);
        auto event1 = new QContextMenuEvent(QContextMenuEvent::Mouse, linkPos);
        auto event2 = new QMouseEvent(QEvent::MouseButtonPress, linkPos, Qt::RightButton, {}, {});
        auto event3 = new QMouseEvent(QEvent::MouseButtonRelease, linkPos, Qt::RightButton, {}, {});
        QCoreApplication::postEvent(renderWidget, event1);
        QCoreApplication::postEvent(renderWidget, event2);
        QCoreApplication::postEvent(renderWidget, event3);
        QVERIFY(waitForSignal(&view, &QWidget::customContextMenuRequested));
        page.triggerAction(QWebEnginePage::DownloadLinkToDisk);
    } else
        QTest::mouseClick(renderWidget, Qt::LeftButton, {}, linkPos);

    // 3. Deliver requested file
    //
    // Request/response headers vary.
    auto fileRR = waitForRequest(&server);
    QVERIFY(fileRR);
    QCOMPARE(fileRR->requestMethod(), QByteArrayLiteral("GET"));
    QCOMPARE(fileRR->requestPath(), QByteArrayLiteral("/") + fileName);
    if (fileHasReferer)
        QCOMPARE(fileRR->requestHeader(QByteArrayLiteral("referer")), server.url().toEncoded());
    else
        QCOMPARE(fileRR->requestHeader(QByteArrayLiteral("referer")), QByteArray());
    if (!fileDisposition.isEmpty())
        fileRR->setResponseHeader(QByteArrayLiteral("content-disposition"), fileDisposition);
    if (!fileMimeTypeDeclared.isEmpty())
        fileRR->setResponseHeader(QByteArrayLiteral("content-type"), fileMimeTypeDeclared);
    fileRR->setResponseBody(fileContents);
    fileRR->sendResponse();

    // 4a. File is displayed and not downloaded - end test
    if (fileAction == FileIsDisplayed) {
        QVERIFY(waitForSignal(&page, &QWebEnginePage::loadFinished, [&](bool ok) { loadOk = ok; }));
        QVERIFY(loadOk);
        return;
    }

    // 4b. File is downloaded - check QWebEngineDownloadItem attributes
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    QByteArray slashFileName = QByteArrayLiteral("/") + fileName;
    QString suggestedPath =
        QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + slashFileName;
    QString downloadPath = tmpDir.path() + slashFileName;
    QUrl downloadUrl = server.url(slashFileName);
    QWebEngineDownloadItem *downloadItem = nullptr;
    QVERIFY(waitForSignal(&profile, &QWebEngineProfile::downloadRequested,
                          [&](QWebEngineDownloadItem *item) {
        QCOMPARE(item->state(), QWebEngineDownloadItem::DownloadRequested);
        QCOMPARE(item->isFinished(), false);
        QCOMPARE(item->totalBytes(), -1);
        QCOMPARE(item->receivedBytes(), 0);
        QCOMPARE(item->interruptReason(), QWebEngineDownloadItem::NoReason);
        QCOMPARE(item->type(), downloadType);
        QCOMPARE(item->mimeType(), QString(fileMimeTypeDetected));
        QCOMPARE(item->path(), suggestedPath);
        QCOMPARE(item->savePageFormat(), QWebEngineDownloadItem::UnknownSaveFormat);
        QCOMPARE(item->url(), downloadUrl);
        item->setPath(downloadPath);
        item->accept();
        downloadItem = item;
    }));
    QVERIFY(downloadItem);
    bool finishOk = false;
    QVERIFY(waitForSignal(downloadItem, &QWebEngineDownloadItem::finished, [&]() {
        auto item = downloadItem;
        QCOMPARE(item->state(), QWebEngineDownloadItem::DownloadCompleted);
        QCOMPARE(item->isFinished(), true);
        QCOMPARE(item->totalBytes(), fileContents.size());
        QCOMPARE(item->receivedBytes(), fileContents.size());
        QCOMPARE(item->interruptReason(), QWebEngineDownloadItem::NoReason);
        QCOMPARE(item->type(), downloadType);
        QCOMPARE(item->mimeType(), QString(fileMimeTypeDetected));
        QCOMPARE(item->path(), downloadPath);
        QCOMPARE(item->savePageFormat(), QWebEngineDownloadItem::UnknownSaveFormat);
        QCOMPARE(item->url(), downloadUrl);
        finishOk = true;
    }));
    QVERIFY(finishOk);

    // 5. Check actual file contents
    QFile file(downloadPath);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QCOMPARE(file.readAll(), fileContents);
}

void tst_QWebEngineDownloads::downloadTwoLinks()
{
    HttpServer server;
    QWebEngineProfile profile;
    QWebEnginePage page(&profile);
    QWebEngineView view;
    view.setPage(&page);

    view.load(server.url());
    view.show();
    auto indexRR = waitForRequest(&server);
    QVERIFY(indexRR);
    QCOMPARE(indexRR->requestMethod(), QByteArrayLiteral("GET"));
    QCOMPARE(indexRR->requestPath(), QByteArrayLiteral("/"));
    indexRR->setResponseHeader(QByteArrayLiteral("content-type"), QByteArrayLiteral("text/html"));
    indexRR->setResponseBody(QByteArrayLiteral("<html><body><a href=\"file1\" download>Link1</a><br/><a href=\"file2\">Link2</a></body></html>"));
    indexRR->sendResponse();
    bool loadOk = false;
    QVERIFY(waitForSignal(&page, &QWebEnginePage::loadFinished, [&](bool ok){ loadOk = ok; }));
    QVERIFY(loadOk);

    auto favIconRR = waitForFaviconRequest(&server);
    QVERIFY(favIconRR);

    QTRY_VERIFY(view.focusWidget());
    QWidget *renderWidget = view.focusWidget();
    QTest::mouseClick(renderWidget, Qt::LeftButton, {}, QPoint(10, 10));
    QTest::mouseClick(renderWidget, Qt::LeftButton, {}, QPoint(10, 30));

    auto file1RR = waitForRequest(&server);
    QVERIFY(file1RR);
    QCOMPARE(file1RR->requestMethod(), QByteArrayLiteral("GET"));
    QCOMPARE(file1RR->requestPath(), QByteArrayLiteral("/file1"));
    auto file2RR = waitForRequest(&server);
    QVERIFY(file2RR);
    QCOMPARE(file2RR->requestMethod(), QByteArrayLiteral("GET"));
    QCOMPARE(file2RR->requestPath(), QByteArrayLiteral("/file2"));

    file1RR->setResponseHeader(QByteArrayLiteral("content-type"), QByteArrayLiteral("text/plain"));
    file1RR->setResponseBody(QByteArrayLiteral("file1"));
    file1RR->sendResponse();
    file2RR->setResponseHeader(QByteArrayLiteral("content-type"), QByteArrayLiteral("text/plain"));
    file2RR->setResponseHeader(QByteArrayLiteral("content-disposition"), QByteArrayLiteral("attachment"));
    file2RR->setResponseBody(QByteArrayLiteral("file2"));
    file2RR->sendResponse();

    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    QString standardDir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    QWebEngineDownloadItem *item1 = nullptr;
    QVERIFY(waitForSignal(&profile, &QWebEngineProfile::downloadRequested,
                          [&](QWebEngineDownloadItem *item) {
        QCOMPARE(item->state(), QWebEngineDownloadItem::DownloadRequested);
        QCOMPARE(item->isFinished(), false);
        QCOMPARE(item->totalBytes(), -1);
        QCOMPARE(item->receivedBytes(), 0);
        QCOMPARE(item->interruptReason(), QWebEngineDownloadItem::NoReason);
        QCOMPARE(item->type(), QWebEngineDownloadItem::DownloadAttribute);
        QCOMPARE(item->mimeType(), QStringLiteral("text/plain"));
        QCOMPARE(item->path(), standardDir + QByteArrayLiteral("/file1"));
        QCOMPARE(item->savePageFormat(), QWebEngineDownloadItem::UnknownSaveFormat);
        QCOMPARE(item->url(), server.url(QByteArrayLiteral("/file1")));
        item->setPath(tmpDir.path() + QByteArrayLiteral("/file1"));
        item->accept();
        item1 = item;
    }));
    QVERIFY(item1);

    QWebEngineDownloadItem *item2 = nullptr;
    QVERIFY(waitForSignal(&profile, &QWebEngineProfile::downloadRequested,
                          [&](QWebEngineDownloadItem *item) {
        QCOMPARE(item->state(), QWebEngineDownloadItem::DownloadRequested);
        QCOMPARE(item->isFinished(), false);
        QCOMPARE(item->totalBytes(), -1);
        QCOMPARE(item->receivedBytes(), 0);
        QCOMPARE(item->interruptReason(), QWebEngineDownloadItem::NoReason);
        QCOMPARE(item->type(), QWebEngineDownloadItem::DownloadAttribute);
        QCOMPARE(item->mimeType(), QStringLiteral("text/plain"));
        QCOMPARE(item->path(), standardDir + QByteArrayLiteral("/file2"));
        QCOMPARE(item->savePageFormat(), QWebEngineDownloadItem::UnknownSaveFormat);
        QCOMPARE(item->url(), server.url(QByteArrayLiteral("/file2")));
        item->setPath(tmpDir.path() + QByteArrayLiteral("/file2"));
        item->accept();
        item2 = item;
    }));
    QVERIFY(item2);
}

void tst_QWebEngineDownloads::downloadPage_data()
{
    QTest::addColumn<QWebEngineDownloadItem::SavePageFormat>("savePageFormat");
    QTest::newRow("SingleHtmlSaveFormat") << QWebEngineDownloadItem::SingleHtmlSaveFormat;
    QTest::newRow("CompleteHtmlSaveFormat") << QWebEngineDownloadItem::CompleteHtmlSaveFormat;
    QTest::newRow("MimeHtmlSaveFormat") << QWebEngineDownloadItem::MimeHtmlSaveFormat;
}

void tst_QWebEngineDownloads::downloadPage()
{
    QFETCH(QWebEngineDownloadItem::SavePageFormat, savePageFormat);

    HttpServer server;
    QWebEngineProfile profile;
    QWebEnginePage page(&profile);
    QWebEngineView view;
    view.setPage(&page);

    view.load(server.url());
    view.show();
    auto indexRR = waitForRequest(&server);
    QVERIFY(indexRR);
    QCOMPARE(indexRR->requestMethod(), QByteArrayLiteral("GET"));
    QCOMPARE(indexRR->requestPath(), QByteArrayLiteral("/"));
    indexRR->setResponseHeader(QByteArrayLiteral("content-type"), QByteArrayLiteral("text/html"));
    indexRR->setResponseBody(QByteArrayLiteral("<html><body>Hello</body></html>"));
    indexRR->sendResponse();
    bool loadOk = false;
    QVERIFY(waitForSignal(&page, &QWebEnginePage::loadFinished, [&](bool ok){ loadOk = ok; }));
    QVERIFY(loadOk);

    auto favIconRR = waitForFaviconRequest(&server);
    QVERIFY(favIconRR);

    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    QString downloadPath = tmpDir.path() + QStringLiteral("/test.html");
    page.save(downloadPath, savePageFormat);

    QWebEngineDownloadItem *downloadItem = nullptr;
    QUrl downloadUrl = server.url("/");
    QVERIFY(waitForSignal(&profile, &QWebEngineProfile::downloadRequested,
                        [&](QWebEngineDownloadItem *item) {
        QCOMPARE(item->state(), QWebEngineDownloadItem::DownloadInProgress);
        QCOMPARE(item->isFinished(), false);
        QCOMPARE(item->totalBytes(), -1);
        QCOMPARE(item->receivedBytes(), 0);
        QCOMPARE(item->interruptReason(), QWebEngineDownloadItem::NoReason);
        QCOMPARE(item->type(), QWebEngineDownloadItem::SavePage);
        // FIXME why is mimeType always the same?
        QCOMPARE(item->mimeType(), QStringLiteral("application/x-mimearchive"));
        QCOMPARE(item->path(), downloadPath);
        QCOMPARE(item->savePageFormat(), savePageFormat);
        QCOMPARE(item->url(), downloadUrl);
        // no need to call item->accept()
        downloadItem = item;
    }));
    QVERIFY(downloadItem);
    bool finishOk = false;
    QVERIFY(waitForSignal(downloadItem, &QWebEngineDownloadItem::finished, [&]() {
        auto item = downloadItem;
        QCOMPARE(item->state(), QWebEngineDownloadItem::DownloadCompleted);
        QCOMPARE(item->isFinished(), true);
        QCOMPARE(item->totalBytes(), item->receivedBytes());
        QVERIFY(item->receivedBytes() > 0);
        QCOMPARE(item->interruptReason(), QWebEngineDownloadItem::NoReason);
        QCOMPARE(item->type(), QWebEngineDownloadItem::SavePage);
        QCOMPARE(item->mimeType(), QStringLiteral("application/x-mimearchive"));
        QCOMPARE(item->path(), downloadPath);
        QCOMPARE(item->savePageFormat(), savePageFormat);
        QCOMPARE(item->url(), downloadUrl);
        finishOk = true;
    }));
    QVERIFY(finishOk);

    QFile file(downloadPath);
    QVERIFY(file.exists());
}

void tst_QWebEngineDownloads::downloadViaSetUrl()
{
    // Reproduce the scenario described in QTBUG-63388 by triggering downloads
    // of the same file multiple times via QWebEnginePage::setUrl

    HttpServer server;
    QWebEngineProfile profile;
    QWebEnginePage page(&profile);
    QSignalSpy loadSpy(&page, &QWebEnginePage::loadFinished);
    QSignalSpy urlSpy(&page, &QWebEnginePage::urlChanged);
    const QUrl indexUrl = server.url();
    const QUrl fileUrl = server.url(QByteArrayLiteral("/file"));

    // Set up the test scenario by trying to load some unrelated HTML.

    page.setUrl(indexUrl);

    auto indexRR = waitForRequest(&server);
    QVERIFY(indexRR);
    QCOMPARE(indexRR->requestMethod(), QByteArrayLiteral("GET"));
    QCOMPARE(indexRR->requestPath(), QByteArrayLiteral("/"));
    indexRR->setResponseHeader(QByteArrayLiteral("content-type"), QByteArrayLiteral("text/html"));
    indexRR->setResponseBody(QByteArrayLiteral("<html><body>Hello</body></html>"));
    indexRR->sendResponse();

    auto indexFavRR = waitForFaviconRequest(&server);
    QVERIFY(indexFavRR);

    QTRY_COMPARE(loadSpy.count(), 1);
    QTRY_COMPARE(urlSpy.count(), 1);
    QCOMPARE(loadSpy.takeFirst().value(0).toBool(), true);
    QCOMPARE(urlSpy.takeFirst().value(0).toUrl(), indexUrl);

    // Download files via setUrl. With QTBUG-63388 after the first iteration the
    // downloads would be triggered for indexUrl and not fileUrl.

    QVector<QUrl> downloadUrls;
    QObject::connect(&profile, &QWebEngineProfile::downloadRequested, [&](QWebEngineDownloadItem *item) {
        downloadUrls.append(item->url());
    });

    for (int i = 0; i != 3; ++i) {
        page.setUrl(fileUrl);
        QCOMPARE(page.url(), fileUrl);

        auto fileRR = waitForRequest(&server);
        QVERIFY(fileRR);
        fileRR->setResponseHeader(QByteArrayLiteral("content-disposition"), QByteArrayLiteral("attachment"));
        fileRR->setResponseBody(QByteArrayLiteral("redacted"));
        fileRR->sendResponse();

        auto fileFavRR = waitForFaviconRequest(&server);
        QVERIFY(fileFavRR);

        QTRY_COMPARE(loadSpy.count(), 1);
        QTRY_COMPARE(urlSpy.count(), 2);
        QTRY_COMPARE(downloadUrls.count(), 1);
        QCOMPARE(loadSpy.takeFirst().value(0).toBool(), false);
        QCOMPARE(urlSpy.takeFirst().value(0).toUrl(), fileUrl);
        QCOMPARE(urlSpy.takeFirst().value(0).toUrl(), indexUrl);
        QCOMPARE(downloadUrls.takeFirst(), fileUrl);
        QCOMPARE(page.url(), indexUrl);
    }
}

QTEST_MAIN(tst_QWebEngineDownloads)
#include "tst_qwebenginedownloads.moc"
