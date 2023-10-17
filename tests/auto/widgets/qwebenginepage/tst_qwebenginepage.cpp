/*
    Copyright (C) 2023 The Qt Company Ltd.
    Copyright (C) 2009 Girish Ramakrishnan <girish@forwardbias.in>
    Copyright (C) 2010 Holger Hans Peter Freyther

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <widgetutil.h>
#include <QtNetwork/private/qtnetworkglobal_p.h>
#include <QtWebEngineCore/qtwebenginecore-config.h>
#include <QtWebEngineCore/private/qtwebenginecoreglobal_p.h>
#include <QByteArray>
#include <QClipboard>
#include <QDir>
#include <QGraphicsWidget>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMainWindow>
#include <QMenu>
#include <QMimeDatabase>
#include <QNetworkProxy>
#include <QPaintEngine>
#include <QPushButton>
#include <QScreen>
#include <QWheelEvent>
#if defined(QT_STATEMACHINE_LIB)
#    include <QStateMachine>
#endif
#include <QtGui/QClipboard>
#include <QtTest/QtTest>
#include <QTextCharFormat>
#if QT_CONFIG(webengine_webchannel)
#include <QWebChannel>
#endif
#include <httpserver.h>
#include <qnetworkcookiejar.h>
#include <qnetworkreply.h>
#include <qnetworkrequest.h>
#include <qwebenginedownloadrequest.h>
#include <qwebenginefilesystemaccessrequest.h>
#include <qwebenginefindtextresult.h>
#include <qwebenginefullscreenrequest.h>
#include <qwebenginehistory.h>
#include <qwebenginenavigationrequest.h>
#include <qwebenginenewwindowrequest.h>
#include <qwebenginenotification.h>
#include <qwebenginepage.h>
#include <qwebengineprofile.h>
#include <qwebenginequotarequest.h>
#include <qwebengineregisterprotocolhandlerrequest.h>
#include <qwebenginescript.h>
#include <qwebenginescriptcollection.h>
#include <qwebenginesettings.h>
#include <qwebengineurlrequestinterceptor.h>
#include <qwebengineurlrequestjob.h>
#include <qwebengineurlscheme.h>
#include <qwebengineurlschemehandler.h>
#include <qwebengineview.h>
#include <qimagewriter.h>
#include <QColorSpace>
#include <QQuickRenderControl>
#include <QQuickWindow>

static void removeRecursive(const QString& dirname)
{
    QDir dir(dirname);
    QFileInfoList entries(dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot));
    for (int i = 0; i < entries.size(); ++i)
        if (entries[i].isDir())
            removeRecursive(entries[i].filePath());
        else
            dir.remove(entries[i].fileName());
    QDir().rmdir(dirname);
}

struct TestBasePage : QWebEnginePage
{
    explicit TestBasePage(QWebEngineProfile *profile, QObject *parent = nullptr) : QWebEnginePage(profile, parent) { }
    explicit TestBasePage(QObject *parent = nullptr) : QWebEnginePage(parent) { }
    QSignalSpy loadSpy { this, &QWebEnginePage::loadFinished };
};

class tst_QWebEnginePage : public QObject
{
    Q_OBJECT

public:
    tst_QWebEnginePage();
    virtual ~tst_QWebEnginePage();

public Q_SLOTS:
    void init();
    void cleanup();
    void cleanupFiles();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void comboBoxPopupPositionAfterMove_data();
    void comboBoxPopupPositionAfterMove();
    void comboBoxPopupPositionAfterChildMove_data();
    void comboBoxPopupPositionAfterChildMove();
    void acceptNavigationRequest();
    void acceptNavigationRequestNavigationType();
    void acceptNavigationRequestRelativeToNothing();
#ifndef Q_OS_MACOS
    void geolocationRequestJS_data();
    void geolocationRequestJS();
#endif
    void loadFinished();
    void actionStates();
    void pasteImage();
    void popupFormSubmission();
    void callbackSpyDeleted();
    void multipleProfilesAndLocalStorage();
    void textSelection();
    void backActionUpdate();
    void localStorageVisibility();
    void consoleOutput();
    void userAgentNewlineStripping();
    void renderWidgetHostViewNotShowTopLevel();
    void getUserMediaRequest_data();
    void getUserMediaRequest();
    void getUserMediaRequestDesktopAudio();
    void getUserMediaRequestSettingDisabled();
    void getUserMediaRequestDesktopVideoManyPages();
    void getUserMediaRequestDesktopVideoManyRequests();
    void savePage();

    void crashTests_LazyInitializationOfMainFrame();

#if defined(ENABLE_WEBGL) && ENABLE_WEBGL
    void acceleratedWebGLScreenshotWithoutView();
    void unacceleratedWebGLScreenshotWithoutView();
#endif

    void testJSPrompt();
    void findText();
    void findTextResult();
    void findTextSuccessiveShouldCallAllCallbacks();
    void findTextCalledOnMatch();
    void findTextActiveMatchOrdinal();
    void deleteQWebEngineViewTwice();
#if defined(QT_STATEMACHINE_LIB)
    void loadSignalsOrder_data();
    void loadSignalsOrder();
#endif
    void openWindowDefaultSize();

#ifdef Q_OS_MACOS
    void macCopyUnicodeToClipboard();
#endif

    void runJavaScript();
    void runJavaScriptDisabled();
    void runJavaScriptFromSlot();
    void fullScreenRequested();
    void requestQuota_data();
    void requestQuota();


    // Tests from tst_QWebEngineFrame
    void symmetricUrl();
    void progressSignal();
    void urlChange();
    void requestedUrlAfterSetAndLoadFailures();
    void asyncAndDelete();
    void earlyToHtml();
    void setHtml();
    void setHtmlWithImageResource();
    void setHtmlWithStylesheetResource();
    void setHtmlWithBaseURL();
    void setHtmlWithJSAlert();
    void setHtmlWithModuleImport();
    void baseUrl_data();
    void baseUrl();
    void scrollPosition();
    void scrollbarsOff();
    void evaluateWillCauseRepaint();
    void setContent_data();
    void setContent();
    void setUrlWithPendingLoads();
    void setUrlToEmpty();
    void setUrlToInvalid();
    void setUrlToBadDomain();
    void setUrlToBadPort();
    void setUrlHistory();
    void setUrlUsingStateObject();
    void setUrlThenLoads_data();
    void setUrlThenLoads();
    void loadInSignalHandlers_data();
    void loadInSignalHandlers();
    void loadFromQrc();
#if QT_CONFIG(webengine_webchannel)
    void restoreHistory();
#endif
    void toPlainTextLoadFinishedRace_data();
    void toPlainTextLoadFinishedRace();
    void setZoomFactor();
    void mouseButtonTranslation();
    void mouseMovementProperties();

    void viewSource();
    void viewSourceURL_data();
    void viewSourceURL();
    void viewSourceCredentials();
    void proxyConfigWithUnexpectedHostPortPair();
    void registerProtocolHandler_data();
    void registerProtocolHandler();
    void dataURLFragment();
    void devTools();
    void openLinkInDifferentProfile();
    void openLinkInNewPage_data();
    void openLinkInNewPage();
    void triggerActionWithoutMenu();
    void dynamicFrame();

    void notificationPermission_data();
    void notificationPermission();
    void sendNotification();
    void contentsSize();

    void setLifecycleState();
    void setVisible();
    void discardPreservesProperties();
    void discardBeforeInitialization();
    void automaticUndiscard();
    void setLifecycleStateWithDevTools();
    void discardPreservesCommittedLoad();
    void discardAbortsPendingLoad();
    void discardAbortsPendingLoadAndPreservesCommittedLoad();
    void recommendedState();
    void recommendedStateAuto();
    void setLifecycleStateAndReload();

    void editActionsWithExplicitFocus();
    void editActionsWithInitialFocus();
    void editActionsWithFocusOnIframe();
    void editActionsWithoutSelection();

    void customUserAgentInNewTab();
    void openNewTabInDifferentProfile_data();
    void openNewTabInDifferentProfile();
    void renderProcessCrashed();
    void renderProcessPid();
    void backgroundColor();
    void audioMuted();
    void closeContents();
    void isSafeRedirect_data();
    void isSafeRedirect();

    void testChooseFilesParameters_data();
    void testChooseFilesParameters();
    void fileSystemAccessDialog();

    void localToRemoteNavigation();
    void clientHints();
    void childFrameInput();
    void openLinkInNewPageWithWebWindowType_data();
    void openLinkInNewPageWithWebWindowType();
    void keepInterceptorAfterNewWindowRequested();

private:
    static bool isFalseJavaScriptResult(QWebEnginePage *page, const QString &javaScript);
    static bool isTrueJavaScriptResult(QWebEnginePage *page, const QString &javaScript);
    static bool isEmptyListJavaScriptResult(QWebEnginePage *page, const QString &javaScript);

    QWebEngineView* m_view;
    QWebEnginePage* m_page;
    QScopedPointer<QPointingDevice> s_touchDevice =
            QScopedPointer<QPointingDevice>(QTest::createTouchDevice());

    QString tmpDirPath() const
    {
        static QString tmpd = QDir::tempPath() + "/tst_qwebenginepage-"
            + QDateTime::currentDateTime().toString(QLatin1String("yyyyMMddhhmmss"));
        return tmpd;
    }

    void makeClick(const QPointer<QWindow> window, bool withTouch = false,
                   const QPoint &p = QPoint())
    {
        QVERIFY2(window, "window is gone");
        if (!withTouch) {
            QTest::mouseClick(window, Qt::LeftButton, Qt::KeyboardModifiers(), p);
        } else {
            QTest::touchEvent(window, s_touchDevice.get()).press(1, p);
            QTest::touchEvent(window, s_touchDevice.get()).release(1, p);
        }
    };

    void makeScroll(QWidget *target, QPointF pos, QPoint globalPos, QPoint angleDelta)
    {
        QWheelEvent ev(pos, globalPos, QPoint(0, 0), angleDelta, Qt::NoButton, Qt::NoModifier,
                       Qt::NoScrollPhase, false);
        QGuiApplication::sendEvent(target, &ev);
    }
};

tst_QWebEnginePage::tst_QWebEnginePage()
{
}

tst_QWebEnginePage::~tst_QWebEnginePage()
{
}

void tst_QWebEnginePage::init()
{
    m_view = new QWebEngineView();
    m_page = m_view->page();
    m_page->settings()->setAttribute(QWebEngineSettings::ErrorPageEnabled, false);
    m_view->settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, true);
}

void tst_QWebEnginePage::cleanup()
{
    delete m_view;
}

void tst_QWebEnginePage::cleanupFiles()
{
    removeRecursive(tmpDirPath());
}

void tst_QWebEnginePage::initTestCase()
{
    QLocale::setDefault(QLocale("en"));
    cleanupFiles(); // In case there are old files from previous runs

    // Set custom path since the CI doesn't install test plugins.
    // Stolen from qtlocation/tests/auto/positionplugintest.
    QString searchPath = QCoreApplication::applicationDirPath();
#ifdef Q_OS_WIN
    searchPath += QStringLiteral("/..");
#endif
    searchPath += QStringLiteral("/../../../plugins");
    QCoreApplication::addLibraryPath(searchPath);

    QWebEngineUrlScheme echo("echo");
    echo.setSyntax(QWebEngineUrlScheme::Syntax::Path);
    QWebEngineUrlScheme::registerScheme(echo);

    QWebEngineUrlScheme local("local");
    local.setFlags(QWebEngineUrlScheme::LocalScheme);
    QWebEngineUrlScheme::registerScheme(local);

    QWebEngineUrlScheme remote("remote");
    remote.setFlags(QWebEngineUrlScheme::CorsEnabled);
    QWebEngineUrlScheme::registerScheme(remote);
}

void tst_QWebEnginePage::cleanupTestCase()
{
    cleanupFiles(); // Be nice
}

class EchoingUrlSchemeHandler : public QWebEngineUrlSchemeHandler
{
public:
    EchoingUrlSchemeHandler(QObject *parent = nullptr)
        : QWebEngineUrlSchemeHandler(parent)
    {
    }
    ~EchoingUrlSchemeHandler() = default;

    void requestStarted(QWebEngineUrlRequestJob *job) override
    {
        QBuffer *buffer = new QBuffer(job);
        buffer->setData(job->requestUrl().toString(QUrl::RemoveScheme).toUtf8());
        job->reply("text/plain;charset=utf-8", buffer);
    }
};

class NavigationRequestOverride : public QWebEnginePage
{
public:
    NavigationRequestOverride(QWebEngineProfile* profile, bool initialValue) : QWebEnginePage(profile, nullptr), m_acceptNavigationRequest(initialValue) {}

    bool m_acceptNavigationRequest;
protected:
    bool acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame) override
    {
        Q_UNUSED(url);
        Q_UNUSED(isMainFrame);
        if (type == QWebEnginePage::NavigationTypeTyped)
            return true;
        return m_acceptNavigationRequest;
    }
};

void tst_QWebEnginePage::acceptNavigationRequest()
{
    QWebEngineProfile profile;
    profile.installUrlSchemeHandler("echo", new EchoingUrlSchemeHandler(&profile));
    NavigationRequestOverride page(&profile, false);

    QSignalSpy loadSpy(&page, SIGNAL(loadFinished(bool)));

    page.setHtml(QString("<html><body><form name='tstform' action='foo' method='get'>"
                            "<input type='text'><input type='submit'></form></body></html>"),
                 QUrl("echo:/"));
    QTRY_COMPARE_WITH_TIMEOUT(loadSpy.size(), 1, 20000);

    evaluateJavaScriptSync(&page, "tstform.submit();");
    QTRY_COMPARE(loadSpy.size(), 2);

    // Content hasn't changed so the form submit will still work
    page.m_acceptNavigationRequest = true;
    evaluateJavaScriptSync(&page, "tstform.submit();");
    QTRY_COMPARE(loadSpy.size(), 3);

    // Now the content has changed
    QCOMPARE(toPlainTextSync(&page), QString("/foo?"));
}

class JSTestPage : public QWebEnginePage
{
Q_OBJECT
public:
    JSTestPage(QObject* parent = 0)
    : QWebEnginePage(parent) {}

    virtual bool shouldInterruptJavaScript()
    {
        return true;
    }
public Q_SLOTS:
    void requestPermission(const QUrl &origin, QWebEnginePage::Feature feature)
    {
        if (m_allowGeolocation)
            setFeaturePermission(origin, feature, PermissionGrantedByUser);
        else
            setFeaturePermission(origin, feature, PermissionDeniedByUser);
    }

public:
    void setGeolocationPermission(bool allow)
    {
        m_allowGeolocation = allow;
    }

private:
    bool m_allowGeolocation;
};

#ifndef Q_OS_MACOS
void tst_QWebEnginePage::geolocationRequestJS_data()
{
    QTest::addColumn<bool>("allowed");
    QTest::addColumn<int>("errorCode");
    QTest::newRow("allowed") << true << 0;
    QTest::newRow("not allowed") << false << 1;
}

void tst_QWebEnginePage::geolocationRequestJS()
{
    QFETCH(bool, allowed);
    QFETCH(int, errorCode);
    QWebEngineView view;
    JSTestPage *newPage = new JSTestPage(&view);
    view.setPage(newPage);
    newPage->setGeolocationPermission(allowed);

    connect(newPage, SIGNAL(featurePermissionRequested(const QUrl&, QWebEnginePage::Feature)),
            newPage, SLOT(requestPermission(const QUrl&, QWebEnginePage::Feature)));

    QSignalSpy spyLoadFinished(newPage, SIGNAL(loadFinished(bool)));
    newPage->setHtml(QString("<html><body>test</body></html>"), QUrl("qrc://secure/origin"));
    QTRY_COMPARE_WITH_TIMEOUT(spyLoadFinished.size(), 1, 20000);

    // Geolocation is only enabled for visible WebContents.
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    if (evaluateJavaScriptSync(newPage, QLatin1String("!navigator.geolocation")).toBool())
        W_QSKIP("Geolocation is not supported.", SkipSingle);

    evaluateJavaScriptSync(newPage, "var errorCode = 0; var done = false; function error(err) { errorCode = err.code; done = true; } function success(pos) { done = true; } navigator.geolocation.getCurrentPosition(success, error)");

    QTRY_VERIFY(evaluateJavaScriptSync(newPage, "done").toBool());
    int result = evaluateJavaScriptSync(newPage, "errorCode").toInt();
    if (result == 2)
        QEXPECT_FAIL("", "No location service available.", Continue);
    QCOMPARE(result, errorCode);
}
#endif

void tst_QWebEnginePage::loadFinished()
{
    QWebEnginePage page;
    QSignalSpy spyLoadStarted(&page, SIGNAL(loadStarted()));
    QSignalSpy spyLoadFinished(&page, SIGNAL(loadFinished(bool)));

    page.load(QUrl("data:text/html,<frameset cols=\"25%,75%\"><frame src=\"data:text/html,"
                                           "<head><meta http-equiv='refresh' content='1'></head>foo \">"
                                           "<frame src=\"data:text/html,bar\"></frameset>"));
    QTRY_COMPARE_WITH_TIMEOUT(spyLoadFinished.size(), 1, 20000);

    QEXPECT_FAIL("", "Behavior change: Load signals are emitted only for the main frame in QtWebEngine.", Continue);
    QTRY_VERIFY_WITH_TIMEOUT(spyLoadStarted.size() > 1, 100);
    QEXPECT_FAIL("", "Behavior change: Load signals are emitted only for the main frame in QtWebEngine.", Continue);
    QTRY_VERIFY_WITH_TIMEOUT(spyLoadFinished.size() > 1, 100);

    spyLoadFinished.clear();

    page.load(QUrl("data:text/html,<frameset cols=\"25%,75%\"><frame src=\"data:text/html,"
                                           "foo \"><frame src=\"data:text/html,bar\"></frameset>"));
    QTRY_COMPARE(spyLoadFinished.size(), 1);
    QCOMPARE(spyLoadFinished.size(), 1);
}

void tst_QWebEnginePage::actionStates()
{
    m_page->load(QUrl("qrc:///resources/script.html"));

    QAction* reloadAction = m_page->action(QWebEnginePage::Reload);
    QAction* stopAction = m_page->action(QWebEnginePage::Stop);

    QTRY_VERIFY(reloadAction->isEnabled());
    QTRY_VERIFY(!stopAction->isEnabled());
}

static QImage imageWithoutAlpha(const QImage &image)
{
    QImage result = image;
    QPainter painter(&result);
    painter.fillRect(result.rect(), Qt::green);
    painter.drawImage(0, 0, image);
    return result;
}

void tst_QWebEnginePage::callbackSpyDeleted()
{
    QWebEnginePage *page = m_view->page();
    CallbackSpy<QVariant> spy;
    QString poorManSleep("function wait(ms){"
                         "  var start = new Date().getTime();"
                         "  var end = start;"
                         "  while (start + ms > end) {"
                             "end = new Date().getTime();"
                         "  }"
                         "}"
                        "wait(1000);");
    page->runJavaScript(poorManSleep, spy.ref());
    //spy deleted before callback
}

void tst_QWebEnginePage::pasteImage()
{
    // Pixels with an alpha value of 0 will have different RGB values after the
    // test -> clipboard -> webengine -> test roundtrip.
    // Clear the alpha channel to make QCOMPARE happy.
    const QImage origImage = imageWithoutAlpha(QImage(":/resources/image.png"));
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setImage(origImage);
    QWebEnginePage *page = m_view->page();
    QSignalSpy spyFinished(m_view, &QWebEngineView::loadFinished);
    page->load(QUrl("qrc:///resources/pasteimage.html"));
    QTRY_VERIFY_WITH_TIMEOUT(!spyFinished.isEmpty(), 20000);
    page->triggerAction(QWebEnginePage::Paste);
    QTRY_VERIFY(evaluateJavaScriptSync(page,
            "window.myImageDataURL ? window.myImageDataURL.length : 0").toInt() > 0);
    QByteArray data = evaluateJavaScriptSync(page, "window.myImageDataURL").toByteArray();
    data.remove(0, data.indexOf(";base64,") + 8);
    QImage image = QImage::fromData(QByteArray::fromBase64(data), "PNG");
    image.setColorSpace(origImage.colorSpace());
    if (image.format() == QImage::Format_RGB32)
        image.reinterpretAsFormat(QImage::Format_ARGB32);
    QCOMPARE(image, origImage);
}

class ConsolePage : public QWebEnginePage
{
public:
    ConsolePage(QObject* parent = 0) : QWebEnginePage(parent) {}

    void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString& message, int lineNumber, const QString& sourceID) override
    {
        levels.append(level);
        messages.append(message);
        lineNumbers.append(lineNumber);
        sourceIDs.append(sourceID);
    }

    QList<int> levels;
    QStringList messages;
    QList<int> lineNumbers;
    QStringList sourceIDs;
};

void tst_QWebEnginePage::consoleOutput()
{
    ConsolePage page;
    // We don't care about the result but want this to be synchronous
    evaluateJavaScriptSync(&page, "this is not valid JavaScript");
    QCOMPARE(page.messages.size(), 1);
    QCOMPARE(page.lineNumbers.at(0), 1);
}

class TestPage : public QWebEnginePage {
    Q_OBJECT
public:
    TestPage(QObject *parent = nullptr, QWebEngineProfile *profile = nullptr) : QWebEnginePage(profile, parent)
    {
        connect(this, &QWebEnginePage::geometryChangeRequested, this, &TestPage::slotGeometryChangeRequested);
        connect(this, &QWebEnginePage::navigationRequested, this, &TestPage::slotNavigationRequested);
        connect(this, &QWebEnginePage::newWindowRequested, this, &TestPage::slotNewWindowRequested);
    }

    struct Navigation {
        QWebEngineNavigationRequest::NavigationType type;
        QUrl url;
        bool isMainFrame;
    };
    QList<Navigation> navigations;

    QList<TestPage*> createdWindows;

    QRect requestedGeometry;

signals:
    void windowCreated();

private Q_SLOTS:
    void slotNavigationRequested(QWebEngineNavigationRequest &request)
    {
        Navigation n;
        n.url = request.url();
        n.type = request.navigationType();
        n.isMainFrame = request.isMainFrame();
        navigations.append(n);
        request.accept();
    }
    void slotNewWindowRequested(QWebEngineNewWindowRequest &request)
    {
        TestPage *page = new TestPage(this, profile());
        createdWindows.append(page);
        emit windowCreated();
        request.openIn(page);
    }

    void slotGeometryChangeRequested(const QRect &geom)
    {
        requestedGeometry = geom;
    }
};

void tst_QWebEnginePage::acceptNavigationRequestNavigationType()
{

    TestPage page;
    QSignalSpy loadSpy(&page, SIGNAL(loadFinished(bool)));

    page.load(QUrl("qrc:///resources/script.html"));
    QTRY_COMPARE_WITH_TIMEOUT(loadSpy.size(), 1, 20000);
    QTRY_COMPARE(page.navigations.size(), 1);

    page.load(QUrl("qrc:///resources/content.html"));
    QTRY_COMPARE_WITH_TIMEOUT(loadSpy.size(), 2, 20000);
    QTRY_COMPARE(page.navigations.size(), 2);

    page.triggerAction(QWebEnginePage::Stop);
    QVERIFY(page.history()->canGoBack());
    page.triggerAction(QWebEnginePage::Back);

    QTRY_COMPARE(loadSpy.size(), 3);
    QTRY_COMPARE(page.navigations.size(), 3);

    page.triggerAction(QWebEnginePage::Reload);
    QTRY_COMPARE(loadSpy.size(), 4);
    QTRY_COMPARE(page.navigations.size(), 4);

    page.load(QUrl("qrc:///resources/reload.html"));
    QTRY_COMPARE_WITH_TIMEOUT(loadSpy.size(), 6, 20000);
    QTRY_COMPARE(page.navigations.size(), 6);

    QList<QWebEngineNavigationRequest::NavigationType> expectedList;
    expectedList << QWebEngineNavigationRequest::TypedNavigation
                 << QWebEngineNavigationRequest::TypedNavigation
                 << QWebEngineNavigationRequest::BackForwardNavigation
                 << QWebEngineNavigationRequest::ReloadNavigation
                 << QWebEngineNavigationRequest::TypedNavigation
                 << QWebEngineNavigationRequest::RedirectNavigation;

    // client side redirect
    page.load(QUrl("qrc:///resources/redirect.html"));
    QTRY_COMPARE_WITH_TIMEOUT(loadSpy.size(), 7, 20000);
    QTRY_COMPARE(page.navigations.size(), 8);
    expectedList += { QWebEngineNavigationRequest::TypedNavigation, QWebEngineNavigationRequest::RedirectNavigation };

    // server side redirect
    HttpServer server;
    server.setResourceDirs({ ":/resources" });
    connect(&server, &HttpServer::newRequest, &server, [&] (HttpReqRep *r) {
        if (r->requestMethod() == "GET") {
            if (r->requestPath() == "/redirect1.html") {
                r->setResponseHeader("Location", server.url("/redirect2.html").toEncoded());
                r->setResponseBody("<html><body>Redirect1</body></html>");
                r->sendResponse(307); // Internal server redirect
            } else if (r->requestPath() == "/redirect2.html") {
                r->setResponseHeader("Location", server.url("/content.html").toEncoded());
                r->setResponseBody("<html><body>Redirect2</body></html>");
                r->sendResponse(301); // Moved permanently
            }
        }
    });
    QVERIFY(server.start());
    page.load(QUrl(server.url("/redirect1.html")));
    QTRY_COMPARE_WITH_TIMEOUT(loadSpy.size(), 8, 20000);
    expectedList += {
        QWebEngineNavigationRequest::TypedNavigation,
        QWebEngineNavigationRequest::RedirectNavigation,
        QWebEngineNavigationRequest::RedirectNavigation
    };

    for (int i = 0; i < expectedList.size(); ++i) {
        QTRY_VERIFY(i < page.navigations.size());
        QCOMPARE(page.navigations[i].type, expectedList[i]);
    }
    QVERIFY(expectedList.size() == page.navigations.size());
}

// Relative url without base url.
//
// See also: QTBUG-48435
void tst_QWebEnginePage::acceptNavigationRequestRelativeToNothing()
{
    TestPage page;
    QSignalSpy loadSpy(&page, SIGNAL(loadFinished(bool)));

    page.setHtml(QString("<html><body><a id='link' href='S0'>limited time offer</a></body></html>"),
                 /* baseUrl: */ QUrl());
    QTRY_COMPARE_WITH_TIMEOUT(loadSpy.size(), 1, 20000);
    page.runJavaScript(QStringLiteral("document.getElementById(\"link\").click()"));
    QTRY_COMPARE_WITH_TIMEOUT(loadSpy.size(), 2, 20000);
    page.setHtml(QString("<html><body><a id='link' href='S0'>limited time offer</a></body></html>"),
                 /* baseUrl: */ QString("qrc:/"));
    QTRY_COMPARE_WITH_TIMEOUT(loadSpy.size(), 3, 20000);
    page.runJavaScript(QStringLiteral("document.getElementById(\"link\").click()"));
    QTRY_COMPARE_WITH_TIMEOUT(loadSpy.size(), 4, 20000);

    // The two setHtml and the second click are counted, while the
    // first click is ignored due to the empty base url.
    QCOMPARE(page.navigations.size(), 3);
    QCOMPARE(page.navigations[0].type, QWebEngineNavigationRequest::TypedNavigation);
    QCOMPARE(page.navigations[1].type, QWebEngineNavigationRequest::TypedNavigation);
    QCOMPARE(page.navigations[2].type, QWebEngineNavigationRequest::LinkClickedNavigation);
    QCOMPARE(page.navigations[2].url, QUrl(QString("qrc:/S0")));
}

void tst_QWebEnginePage::popupFormSubmission()
{
    QWebEngineProfile profile;
    profile.installUrlSchemeHandler("echo", new EchoingUrlSchemeHandler(&profile));
    TestPage page(nullptr, &profile);
    QSignalSpy loadFinishedSpy(&page, SIGNAL(loadFinished(bool)));
    QSignalSpy windowCreatedSpy(&page, SIGNAL(windowCreated()));

    page.settings()->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, true);
    page.setHtml("<form name='form1' method=get action='' target='myNewWin'>"
                 "  <input type='hidden' name='foo' value='bar'>"
                 "</form>", QUrl("echo:"));
    QTRY_COMPARE_WITH_TIMEOUT(loadFinishedSpy.size(), 1, 20000);

    page.runJavaScript("window.open('', 'myNewWin', 'width=500,height=300,toolbar=0');");
    evaluateJavaScriptSync(&page, "document.form1.submit();");
    QTRY_COMPARE(windowCreatedSpy.size(), 1);

    // The number of popup created should be one.
    QVERIFY(page.createdWindows.size() == 1);

    QTRY_VERIFY(!page.createdWindows[0]->url().isEmpty());

    // Check if the form submission was OK.
    QTRY_VERIFY(page.createdWindows[0]->url().toString().contains("?foo=bar"));
}

class TestNetworkManager : public QNetworkAccessManager
{
public:
    TestNetworkManager(QObject* parent) : QNetworkAccessManager(parent) {}

    QList<QUrl> requestedUrls;
    QList<QNetworkRequest> requests;

protected:
    QNetworkReply* createRequest(Operation op, const QNetworkRequest &request, QIODevice* outgoingData) override
    {
        requests.append(request);
        requestedUrls.append(request.url());
        return QNetworkAccessManager::createRequest(op, request, outgoingData);
    }
};

void tst_QWebEnginePage::multipleProfilesAndLocalStorage()
{
    QDir dir(tmpDirPath());
    bool success = dir.mkpath("path1");
    success = success && dir.mkdir("path2");
    QVERIFY(success);
    {
        QWebEngineProfile profile1("test1");
        QWebEngineProfile profile2("test2");
        profile1.settings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
        profile2.settings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
        profile1.setPersistentStoragePath(QDir::toNativeSeparators(tmpDirPath() + "/path1"));
        profile2.setPersistentStoragePath(QDir::toNativeSeparators(tmpDirPath() + "/path2"));

        QWebEnginePage page1(&profile1, nullptr);
        QWebEnginePage page2(&profile2, nullptr);
        QSignalSpy loadSpy1(&page1, SIGNAL(loadFinished(bool)));
        QSignalSpy loadSpy2(&page2, SIGNAL(loadFinished(bool)));

        page1.setHtml(QString("<html><body> </body></html>"), QUrl("http://wwww.example.com"));
        page2.setHtml(QString("<html><body> </body></html>"), QUrl("http://wwww.example.com"));
        QTRY_COMPARE_WITH_TIMEOUT(loadSpy1.size(), 1, 20000);
        QTRY_COMPARE_WITH_TIMEOUT(loadSpy2.size(), 1, 20000);

        evaluateJavaScriptSync(&page1, "localStorage.setItem('test', 'value1');");
        evaluateJavaScriptSync(&page2, "localStorage.setItem('test', 'value2');");

        page1.setHtml(QString("<html><body> </body></html>"), QUrl("http://wwww.example.com"));
        page2.setHtml(QString("<html><body> </body></html>"), QUrl("http://wwww.example.com"));
        QTRY_COMPARE(loadSpy1.size(), 2);
        QTRY_COMPARE(loadSpy2.size(), 2);

        QVariant s1 = evaluateJavaScriptSync(&page1, "localStorage.getItem('test')");
        QCOMPARE(s1.toString(), QString("value1"));
        QVariant s2 = evaluateJavaScriptSync(&page2, "localStorage.getItem('test')");
        QCOMPARE(s2.toString(), QString("value2"));
    }
    // Avoid deleting on-disk dbs before the underlying browser-context has been asynchronously deleted
    QTest::qWait(1000);
    QDir(tmpDirPath() + "/path1").removeRecursively();
    QDir(tmpDirPath() + "/path2").removeRecursively();
}

class CursorTrackedPage : public QWebEnginePage
{
public:

    CursorTrackedPage(QWidget *parent = 0): QWebEnginePage(parent) {
    }

    QString jsSelectedText() {
        return evaluateJavaScriptSync(this, "window.getSelection().toString()").toString();
    }

    int selectionStartOffset() {
        return evaluateJavaScriptSync(this, "window.getSelection().getRangeAt(0).startOffset").toInt();
    }

    int selectionEndOffset() {
        return evaluateJavaScriptSync(this, "window.getSelection().getRangeAt(0).endOffset").toInt();
    }

    // true if start offset == end offset, i.e. no selected text
    int isSelectionCollapsed() {
        return evaluateJavaScriptSync(this, "window.getSelection().getRangeAt(0).collapsed").toBool();
    }
};

void tst_QWebEnginePage::textSelection()
{
    CursorTrackedPage page;

    QString textToSelect("The quick brown fox");
    QString content = QString("<html><body><p id=one>%1</p>" \
        "<p id=two>jumps over the lazy dog</p>" \
        "<p>May the source<br/>be with you!</p></body></html>").arg(textToSelect);

    QSignalSpy loadSpy(&page, SIGNAL(loadFinished(bool)));
    page.setHtml(content);
    QTRY_COMPARE_WITH_TIMEOUT(loadSpy.size(), 1, 20000);

    // these actions must exist
    QVERIFY(page.action(QWebEnginePage::SelectAll) != 0);

    // ..but SelectAll is disabled because the page has no focus due to disabled FocusOnNavigationEnabled.
    QCOMPARE(page.action(QWebEnginePage::SelectAll)->isEnabled(), false);

    // Verify hasSelection returns false since there is no selection yet...
    QVERIFY(!page.hasSelection());
    QVERIFY(page.jsSelectedText().isEmpty());

    // this will select the first paragraph
    QString selectScript = "var range = document.createRange(); " \
        "var node = document.getElementById(\"one\"); " \
        "range.selectNode(node); " \
        "getSelection().addRange(range);";
    evaluateJavaScriptSync(&page, selectScript);

    // Make sure hasSelection returns true, since there is selected text now...
    QTRY_VERIFY(page.hasSelection());
    QCOMPARE(page.selectedText().trimmed(), textToSelect);

    QCOMPARE(page.jsSelectedText().trimmed(), textToSelect);

    // navigate away and check that selection is cleared
    page.load(QUrl("about:blank"));
    QTRY_COMPARE(loadSpy.size(), 2);

    QVERIFY(!page.hasSelection());
    QVERIFY(page.selectedText().isEmpty());

    QVERIFY(page.jsSelectedText().isEmpty());
}


void tst_QWebEnginePage::backActionUpdate()
{
    QWebEngineView view;
    view.resize(640, 480);
    view.show();

    QWebEnginePage *page = view.page();
    QSignalSpy loadSpy(page, &QWebEnginePage::loadFinished);
    QAction *action = page->action(QWebEnginePage::Back);
    QVERIFY(!action->isEnabled());

    page->load(QUrl("qrc:///resources/framedindex.html"));
    QTRY_COMPARE_WITH_TIMEOUT(loadSpy.size(), 1, 20000);
    QVERIFY(!action->isEnabled());

    auto firstAnchorCenterInFrame = [](QWebEnginePage *page, const QString &frameName) {
        QVariantList rectList = evaluateJavaScriptSync(page,
            "(function(){"
            "var frame = document.getElementsByName('" + frameName + "')[0];"
            "var anchor = frame.contentDocument.getElementsByTagName('a')[0];"
            "var rect = anchor.getBoundingClientRect();"
            "return [(rect.left + rect.right) / 2, (rect.top + rect.bottom) / 2];"
        "})()").toList();

        if (rectList.size() != 2) {
            qWarning("firstAnchorCenterInFrame failed.");
            return QPoint();
        }

        return QPoint(rectList.at(0).toInt(), rectList.at(1).toInt());
    };

    QVERIFY(evaluateJavaScriptSync(page, "document.getElementsByName('frame_b')[0].contentDocument == undefined").toBool());
    QTest::mouseClick(view.focusProxy(), Qt::LeftButton, {}, firstAnchorCenterInFrame(page, "frame_c"));
    QTRY_VERIFY(evaluateJavaScriptSync(page, "document.getElementsByName('frame_b')[0].contentDocument != undefined").toBool());
    QTRY_VERIFY(action->isEnabled());
}

void tst_QWebEnginePage::localStorageVisibility()
{
    QWebEngineProfile profile;
    QWebEnginePage webPage1(&profile);
    QWebEnginePage webPage2(&profile);

    webPage1.settings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
    webPage2.settings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, false);

    QSignalSpy loadSpy1(&webPage1, &QWebEnginePage::loadFinished);
    QSignalSpy loadSpy2(&webPage2, &QWebEnginePage::loadFinished);
    webPage1.setHtml(QString("<html><body>test</body></html>"), QUrl("http://www.example.com/"));
    webPage2.setHtml(QString("<html><body>test</body></html>"), QUrl("http://www.example.com/"));
    QTRY_COMPARE_WITH_TIMEOUT(loadSpy1.size(), 1, 20000);
    QTRY_COMPARE_WITH_TIMEOUT(loadSpy2.size(), 1, 20000);

    // The attribute determines the visibility of the window.localStorage object.
    QVERIFY(evaluateJavaScriptSync(&webPage1, QString("(window.localStorage != undefined)")).toBool());
    QVERIFY(!evaluateJavaScriptSync(&webPage2, QString("(window.localStorage != undefined)")).toBool());

    // Toggle local setting for every page and...
    webPage1.settings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, false);
    webPage2.settings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
    // TODO: note this setting is flaky, consider settings().commit()
    // ...first check second page (for storage to appear) as applying settings is batched and done asynchronously
    QTRY_VERIFY(evaluateJavaScriptSync(&webPage2, QString("(window.localStorage != undefined)")).toBool());
    // Switching the feature off does not actively remove the object from webPage1.
// FIXME: 94-based: now it does
//     QVERIFY(evaluateJavaScriptSync(&webPage1, QString("(window.localStorage != undefined)")).toBool());

    // The object disappears only after reloading.
    webPage1.triggerAction(QWebEnginePage::Reload);
    webPage2.triggerAction(QWebEnginePage::Reload);
    QTRY_COMPARE(loadSpy1.size(), 2);
    QTRY_COMPARE(loadSpy2.size(), 2);
    QVERIFY(!evaluateJavaScriptSync(&webPage1, QString("(window.localStorage != undefined)")).toBool());
    QVERIFY(evaluateJavaScriptSync(&webPage2, QString("(window.localStorage != undefined)")).toBool());
}

void tst_QWebEnginePage::userAgentNewlineStripping()
{
    QWebEngineProfile profile;
    QWebEnginePage page(&profile);

    profile.setHttpUserAgent(QStringLiteral("My User Agent\nX-New-Http-Header: Oh Noes!"));
    // The user agent will be updated after a page load.
    page.load(QUrl("about:blank"));

    QTRY_COMPARE(evaluateJavaScriptSync(&page, "navigator.userAgent").toString(), QStringLiteral("My User Agent X-New-Http-Header: Oh Noes!"));
}

void tst_QWebEnginePage::crashTests_LazyInitializationOfMainFrame()
{
    {
        QWebEnginePage webPage;
    }
    {
        QWebEnginePage webPage;
        webPage.selectedText();
    }
    {
        QWebEnginePage webPage;
        webPage.triggerAction(QWebEnginePage::Back, true);
    }
}

#if defined(ENABLE_WEBGL) && ENABLE_WEBGL
// https://bugs.webkit.org/show_bug.cgi?id=54138
static void webGLScreenshotWithoutView(bool accelerated)
{
    QWebEnginePage page;
    page.settings()->setAttribute(QWebEngineSettings::WebGLEnabled, true);
    page.settings()->setAttribute(QWebEngineSettings::AcceleratedCompositingEnabled, accelerated);
    page.setHtml("<html><body>"
                       "<canvas id='webgl' width='300' height='300'></canvas>"
                       "<script>document.getElementById('webgl').getContext('experimental-webgl')</script>"
                       "</body></html>");

    takeScreenshot(&page);
}

void tst_QWebEnginePage::acceleratedWebGLScreenshotWithoutView()
{
    webGLScreenshotWithoutView(true);
}

void tst_QWebEnginePage::unacceleratedWebGLScreenshotWithoutView()
{
    webGLScreenshotWithoutView(false);
}
#endif

/**
 * Test fixups for https://bugs.webkit.org/show_bug.cgi?id=30914
 *
 * From JS we test the following conditions.
 *
 *   OK     + QString() => SUCCESS, empty string (but not null)
 *   OK     + "text"    => SUCCESS, "text"
 *   CANCEL + QString() => CANCEL, null string
 *   CANCEL + "text"    => CANCEL, null string
 */
class JSPromptPage : public QWebEnginePage {
    Q_OBJECT
public:
    JSPromptPage()
    {}

    bool javaScriptPrompt(const QUrl &securityOrigin, const QString& msg, const QString& defaultValue, QString* result) override
    {
        if (msg == QLatin1String("test1")) {
            *result = QString();
            return true;
        } else if (msg == QLatin1String("test2")) {
            *result = QLatin1String("text");
            return true;
        } else if (msg == QLatin1String("test3")) {
            *result = QString();
            return false;
        } else if (msg == QLatin1String("test4")) {
            *result = QLatin1String("text");
            return false;
        }

        qFatal("Unknown msg.");
        return QWebEnginePage::javaScriptPrompt(securityOrigin, msg, defaultValue, result);
    }
};

void tst_QWebEnginePage::testJSPrompt()
{
    JSPromptPage page;
    bool res;
    QSignalSpy loadSpy(&page, SIGNAL(loadFinished(bool)));
    page.setHtml(QStringLiteral("<html><body></body></html>"));
    QTRY_COMPARE_WITH_TIMEOUT(loadSpy.size(), 1, 20000);

    // OK + QString()
    res = evaluateJavaScriptSync(&page,
            "var retval = prompt('test1');"
            "retval=='' && retval.length == 0;").toBool();
    QVERIFY(res);

    // OK + "text"
    res = evaluateJavaScriptSync(&page,
            "var retval = prompt('test2');"
            "retval=='text' && retval.length == 4;").toBool();
    QVERIFY(res);

    // Cancel + QString()
    res = evaluateJavaScriptSync(&page,
            "var retval = prompt('test3');"
            "retval===null;").toBool();
    QVERIFY(res);

    // Cancel + "text"
    res = evaluateJavaScriptSync(&page,
            "var retval = prompt('test4');"
            "retval===null;").toBool();
    QVERIFY(res);
}

void tst_QWebEnginePage::findText()
{
    QSignalSpy loadSpy(m_view, SIGNAL(loadFinished(bool)));
    m_view->setHtml(QString("<html><head></head><body><div>foo bar</div></body></html>"));

    // Showing is required, otherwise all find operations fail.
    m_view->show();
    QTRY_COMPARE(loadSpy.size(), 1);

    // Select whole page contents.
    QTRY_VERIFY(m_view->page()->action(QWebEnginePage::SelectAll)->isEnabled());
    m_view->page()->triggerAction(QWebEnginePage::SelectAll);
    QTRY_COMPARE(m_view->hasSelection(), true);

    // Invoking a stopFinding operation will not change or clear the currently selected text,
    // if nothing was found beforehand.
    {
        CallbackSpy<QWebEngineFindTextResult> callbackSpy;
        QSignalSpy signalSpy(m_view->page(), &QWebEnginePage::findTextFinished);
        m_view->findText("", {}, callbackSpy.ref());
        QVERIFY(callbackSpy.wasCalled());
        QCOMPARE(signalSpy.size(), 1);
        QTRY_COMPARE(m_view->selectedText(), QString("foo bar"));
    }

    // Invoking a startFinding operation with text that won't be found, will clear the current
    // selection.
    {
        CallbackSpy<QWebEngineFindTextResult> callbackSpy;
        QSignalSpy signalSpy(m_view->page(), &QWebEnginePage::findTextFinished);
        m_view->findText("Will not be found", {}, callbackSpy.ref());
        QCOMPARE(callbackSpy.waitForResult().numberOfMatches(), 0);
        QTRY_COMPARE(signalSpy.size(), 1);
        auto result = signalSpy.takeFirst().value(0).value<QWebEngineFindTextResult>();
        QCOMPARE(result.numberOfMatches(), 0);
        QTRY_VERIFY(m_view->selectedText().isEmpty());
    }

    // Select whole page contents again.
    m_view->page()->triggerAction(QWebEnginePage::SelectAll);
    QTRY_COMPARE(m_view->hasSelection(), true);

    // Invoking a startFinding operation with text that will be found, will clear the current
    // selection as well.
    {
        CallbackSpy<QWebEngineFindTextResult> callbackSpy;
        QSignalSpy signalSpy(m_view->page(), &QWebEnginePage::findTextFinished);
        m_view->findText("foo", {}, callbackSpy.ref());
        QVERIFY(callbackSpy.waitForResult().numberOfMatches() > 0);
        QTRY_COMPARE(signalSpy.size(), 1);
        QTRY_VERIFY(m_view->selectedText().isEmpty());
    }

    // Invoking a stopFinding operation after text was found, will set the selected text to the
    // found text.
    {
        CallbackSpy<QWebEngineFindTextResult> callbackSpy;
        QSignalSpy signalSpy(m_view->page(), &QWebEnginePage::findTextFinished);
        m_view->findText("", {}, callbackSpy.ref());
        QTRY_VERIFY(callbackSpy.wasCalled());
        QTRY_COMPARE(signalSpy.size(), 1);
        QTRY_COMPARE(m_view->selectedText(), QString("foo"));
    }

    // Invoking startFinding operation for the same text twice. Without any wait, the second one
    // should interrupt the first one.
    {
        QSignalSpy signalSpy(m_view->page(), &QWebEnginePage::findTextFinished);
        m_view->findText("foo", {});
        m_view->findText("foo", {});
        QTRY_COMPARE(signalSpy.size(), 2);
        QTRY_VERIFY(m_view->selectedText().isEmpty());

        QCOMPARE(signalSpy.at(0).value(0).value<QWebEngineFindTextResult>().numberOfMatches(), 0);
        QCOMPARE(signalSpy.at(1).value(0).value<QWebEngineFindTextResult>().numberOfMatches(), 1);
    }
}

void tst_QWebEnginePage::findTextResult()
{
    QSignalSpy findTextSpy(m_view->page(), &QWebEnginePage::findTextFinished);
    auto signalResult = [&findTextSpy]() -> QList<int> {
        if (findTextSpy.size() != 1)
            return QList<int>({-1, -1});
        auto r = findTextSpy.takeFirst().value(0).value<QWebEngineFindTextResult>();
        return QList<int>({ r.numberOfMatches(), r.activeMatch() });
    };

    // findText will abort in blink if the view has an empty size.
    m_view->resize(800, 600);
    m_view->show();

    QSignalSpy loadSpy(m_view, SIGNAL(loadFinished(bool)));
    m_view->setHtml(QString("<html><head></head><body><div>foo bar</div></body></html>"));
    QTRY_COMPARE(loadSpy.size(), 1);

    QCOMPARE(findTextSync(m_page, ""), false);
    QCOMPARE(signalResult(), QList<int>({0, 0}));

    const QStringList words = { "foo", "bar" };
    for (const QString &subString : words) {
        QCOMPARE(findTextSync(m_page, subString), true);
        QCOMPARE(signalResult(), QList<int>({1, 1}));

        QCOMPARE(findTextSync(m_page, ""), false);
        QCOMPARE(signalResult(), QList<int>({0, 0}));
    }

    QCOMPARE(findTextSync(m_page, "blahhh"), false);
    QCOMPARE(signalResult(), QList<int>({0, 0}));
    QCOMPARE(findTextSync(m_page, ""), false);
    QCOMPARE(signalResult(), QList<int>({0, 0}));
}

void tst_QWebEnginePage::findTextSuccessiveShouldCallAllCallbacks()
{
    CallbackSpy<QWebEngineFindTextResult> spy1;
    CallbackSpy<QWebEngineFindTextResult> spy2;
    CallbackSpy<QWebEngineFindTextResult> spy3;
    CallbackSpy<QWebEngineFindTextResult> spy4;
    CallbackSpy<QWebEngineFindTextResult> spy5;
    QSignalSpy loadSpy(m_view, SIGNAL(loadFinished(bool)));
    m_view->setHtml(QString("<html><head></head><body><div>abcdefg abcdefg abcdefg abcdefg abcdefg</div></body></html>"));
    QTRY_COMPARE_WITH_TIMEOUT(loadSpy.size(), 1, 20000);
    m_page->findText("abcde", {}, spy1.ref());
    m_page->findText("abcd", {}, spy2.ref());
    m_page->findText("abc", {}, spy3.ref());
    m_page->findText("ab", {}, spy4.ref());
    m_page->findText("a", {}, spy5.ref());
    spy5.waitForResult();
    QVERIFY(spy1.wasCalled());
    QVERIFY(spy2.wasCalled());
    QVERIFY(spy3.wasCalled());
    QVERIFY(spy4.wasCalled());
    QVERIFY(spy5.wasCalled());
}

void tst_QWebEnginePage::findTextCalledOnMatch()
{
    QSignalSpy loadSpy(m_view->page(), &QWebEnginePage::loadFinished);

    // findText will abort in blink if the view has an empty size.
    m_view->resize(800, 600);
    m_view->show();
    m_view->setHtml(QString("<html><head></head><body><div>foo bar</div></body></html>"));
    QTRY_COMPARE(loadSpy.size(), 1);

    // CALLBACK
    bool callbackCalled = false;
    m_view->page()->findText("foo", {}, [this, &callbackCalled](QWebEngineFindTextResult result) {
        QVERIFY(result.numberOfMatches());

        m_view->page()->findText("bar", {}, [&callbackCalled](QWebEngineFindTextResult result) {
            QVERIFY(result.numberOfMatches());
            callbackCalled = true;
        });
    });
    QTRY_VERIFY(callbackCalled);

    // SIGNAL
    int findTextFinishedCount = 0;
    connect(m_view->page(), &QWebEnginePage::findTextFinished, [this, &findTextFinishedCount](QWebEngineFindTextResult result) {
        QCOMPARE(result.numberOfMatches(), 1);
        if (findTextFinishedCount == 0)
            m_view->page()->findText("bar");
        findTextFinishedCount++;
    });

    m_view->page()->findText("foo");
    QTRY_COMPARE(findTextFinishedCount, 2);
}

void tst_QWebEnginePage::findTextActiveMatchOrdinal()
{
    QSignalSpy loadSpy(m_view->page(), &QWebEnginePage::loadFinished);
    QSignalSpy findTextSpy(m_view->page(), &QWebEnginePage::findTextFinished);
    QWebEngineFindTextResult result;

    // findText will abort in blink if the view has an empty size.
    m_view->resize(800, 600);
    m_view->show();
    m_view->setHtml(QString("<html><head></head><body><div>foo bar foo bar foo</div></body></html>"));
    QTRY_COMPARE(loadSpy.size(), 1);

    // Iterate over all "foo" matches.
    for (int i = 1; i <= 3; ++i) {
        m_view->page()->findText("foo", {});
        QTRY_COMPARE(findTextSpy.size(), 1);
        result = findTextSpy.takeFirst().value(0).value<QWebEngineFindTextResult>();
        QCOMPARE(result.numberOfMatches(), 3);
        QCOMPARE(result.activeMatch(), i);
    }

    // The last match is followed by the fist one.
    m_view->page()->findText("foo", {});
    QTRY_COMPARE(findTextSpy.size(), 1);
    result = findTextSpy.takeFirst().value(0).value<QWebEngineFindTextResult>();
    QCOMPARE(result.numberOfMatches(), 3);
    QCOMPARE(result.activeMatch(), 1);

    // The first match is preceded by the last one.
    m_view->page()->findText("foo", QWebEnginePage::FindBackward);
    QTRY_COMPARE(findTextSpy.size(), 1);
    result = findTextSpy.takeFirst().value(0).value<QWebEngineFindTextResult>();
    QCOMPARE(result.numberOfMatches(), 3);
    QCOMPARE(result.activeMatch(), 3);

    // Finding another word resets the activeMatch.
    m_view->page()->findText("bar", {});
    QTRY_COMPARE(findTextSpy.size(), 1);
    result = findTextSpy.takeFirst().value(0).value<QWebEngineFindTextResult>();
    QCOMPARE(result.numberOfMatches(), 2);
    QCOMPARE(result.activeMatch(), 1);

    // If no match activeMatch is 0.
    m_view->page()->findText("bla", {});
    QTRY_COMPARE(findTextSpy.size(), 1);
    result = findTextSpy.takeFirst().value(0).value<QWebEngineFindTextResult>();
    QCOMPARE(result.numberOfMatches(), 0);
    QCOMPARE(result.activeMatch(), 0);
}

static QWindow *findNewTopLevelWindow(const QWindowList &oldTopLevelWindows)
{
    const auto tlws = QGuiApplication::topLevelWindows();
    for (auto w : tlws) {
        // note 'offscreen' window is a top-level window
        if (!oldTopLevelWindows.contains(w)
            && !QQuickRenderControl::renderWindowFor(qobject_cast<QQuickWindow *>(w))) {
            return w;
        }
    }
    return nullptr;
}

void tst_QWebEnginePage::comboBoxPopupPositionAfterMove_data()
{
    QTest::addColumn<bool>("withTouch");
    QTest::addRow("mouse") << false;
    QTest::addRow("touch") << true;
}

void tst_QWebEnginePage::comboBoxPopupPositionAfterMove()
{
#if defined(Q_OS_MACOS) && (defined(__arm64__) || defined(__aarch64__))
    QSKIP("This test crashes for Apple M1");
#endif
    QWebEngineView view;
    QTRY_VERIFY(QGuiApplication::primaryScreen());
    view.move(QGuiApplication::primaryScreen()->availableGeometry().topLeft());
    view.resize(640, 480);
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));
    QSignalSpy spyLoadFinished(&view, SIGNAL(loadFinished(bool)));
    view.setHtml(QLatin1String("<html><head></head><body><select id='foo'>"
                               "<option>fran</option><option>troz</option>"
                               "</select></body></html>"));
    QTRY_COMPARE(spyLoadFinished.size(), 1);
    const auto oldTlws = QGuiApplication::topLevelWindows();
    QFETCH(bool, withTouch);
    QPointer<QWindow> window = view.windowHandle();
    auto pos = elementCenter(view.page(), "foo");
    makeClick(window, withTouch, pos);
    QWindow *popup = nullptr;
    QTRY_VERIFY((popup = findNewTopLevelWindow(oldTlws)));
    QVERIFY(QTest::qWaitForWindowExposed(popup));
    QTRY_VERIFY(popup->width() > 0 && popup->height() > 0);
    QTRY_VERIFY(QGuiApplication::topLevelWindows().contains(popup));
    QTRY_VERIFY(!popup->position().isNull());
    QPoint popupPos = popup->position();
    QPointer<QWindow> pw(popup);
    // Close the popup by clicking somewhere into the page.
    makeClick(window, withTouch, QPoint(1, 1));
    QTRY_VERIFY(!QGuiApplication::topLevelWindows().contains(popup));
    QTRY_VERIFY(!pw);
    auto jsViewPosition = [&view]() {
        QLatin1String script("(function() { return [window.screenX, window.screenY]; })()");
        QVariantList posList = evaluateJavaScriptSync(view.page(), script).toList();

        if (posList.size() != 2) {
            qWarning("jsViewPosition failed.");
            return QPoint();
        }

        return QPoint(posList.at(0).toInt(), posList.at(1).toInt());
    };

    // Move the top-level QWebEngineView a little and check the popup's position.
    const QPoint offset(12, 13);
    view.move(view.pos() + offset);
    QTRY_COMPARE(jsViewPosition(), view.pos());
    makeClick(window, withTouch, elementCenter(view.page(), "foo"));
    QTRY_VERIFY((popup = findNewTopLevelWindow(oldTlws)));
    QTRY_VERIFY(popup->width() > 0 && popup->height() > 0);
    QTRY_VERIFY(QGuiApplication::topLevelWindows().contains(popup));
    QTRY_VERIFY(!popup->position().isNull());
    QCOMPARE(popupPos + offset, popup->position());
    makeClick(window, withTouch, QPoint(1, 1));
    QTRY_VERIFY(!QGuiApplication::topLevelWindows().contains(popup));
}

void tst_QWebEnginePage::comboBoxPopupPositionAfterChildMove_data()
{
    QTest::addColumn<bool>("withTouch");
    QTest::addRow("mouse") << false;
    QTest::addRow("touch") << true;
}

void tst_QWebEnginePage::comboBoxPopupPositionAfterChildMove()
{
#if defined(Q_OS_MACOS) && (defined(__arm64__) || defined(__aarch64__))
    QSKIP("This test crashes for Apple M1");
#endif
    QWidget mainWidget;
    mainWidget.setLayout(new QHBoxLayout);

    QWidget spacer;
    mainWidget.layout()->addWidget(&spacer);

    QWebEngineView view;
    mainWidget.layout()->addWidget(&view);

    QScreen *screen = QGuiApplication::primaryScreen();
    Q_ASSERT(screen);
    mainWidget.move(screen->availableGeometry().topLeft());
    mainWidget.resize(640, 480);
    mainWidget.show();
    QVERIFY(QTest::qWaitForWindowExposed(&mainWidget));

    QSignalSpy loadSpy(&view, SIGNAL(loadFinished(bool)));
    view.setHtml(QLatin1String("<html><head></head><body><select autofocus id='foo'>"
                               "<option value=\"narf\">narf</option><option>zort</option>"
                               "</select></body></html>"));
    QTRY_COMPARE(loadSpy.size(), 1);
    const auto oldTlws = QGuiApplication::topLevelWindows();

    QFETCH(bool, withTouch);
    QPointer<QWindow> window = view.window()->windowHandle();
    makeClick(window, withTouch, view.mapTo(view.window(), elementCenter(view.page(), "foo")));

    QWindow *popup = nullptr;
    QTRY_VERIFY((popup = findNewTopLevelWindow(oldTlws)));
    QVERIFY(QTest::qWaitForWindowExposed(popup));
    QTRY_VERIFY(popup->width() > 0 && popup->height() > 0);
    QTRY_VERIFY(QGuiApplication::topLevelWindows().contains(popup));
    QTRY_VERIFY(!popup->position().isNull());
    QPoint popupPos = popup->position();

    // Close the popup by clicking somewhere into the page.
    makeClick(window, withTouch, view.mapTo(view.window(), QPoint(1, 1)));
    QTRY_VERIFY(!QGuiApplication::topLevelWindows().contains(popup));

    int originalViewWidth = view.size().width();
    auto jsViewWidth = [&view]() {
        QLatin1String script("(function() { return window.innerWidth; })()");
        int viewWidth = evaluateJavaScriptSync(view.page(), script).toInt();
        return viewWidth;
    };

    QCOMPARE(jsViewWidth(), originalViewWidth);

    // Resize the "spacer" widget, and implicitly change the global position of the QWebEngineView.
    const int offset = 50;
    spacer.setMinimumWidth(spacer.size().width() + offset);

    QTRY_COMPARE(jsViewWidth(), originalViewWidth - offset);

    makeClick(window, withTouch, view.mapTo(view.window(), elementCenter(view.page(), "foo")));
    QTRY_VERIFY((popup = findNewTopLevelWindow(oldTlws)));
    QVERIFY(QTest::qWaitForWindowExposed(popup));
    QTRY_VERIFY(popup->width() > 0 && popup->height() > 0);
    QTRY_VERIFY(!popup->position().isNull());
    QCOMPARE(popupPos + QPoint(offset, 0), popup->position());
    makeClick(window, withTouch, QPoint(1, 1));
    QTRY_VERIFY(!QGuiApplication::topLevelWindows().contains(popup));
}

#ifdef Q_OS_MACOS
void tst_QWebEnginePage::macCopyUnicodeToClipboard()
{
    QString unicodeText = QString::fromUtf8("αβγδεζηθικλμπ");
    m_page->setHtml(QString("<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" /></head><body>%1</body></html>").arg(unicodeText));
    m_page->triggerAction(QWebEnginePage::SelectAll);
    m_page->triggerAction(QWebEnginePage::Copy);

    QString clipboardData = QString::fromUtf8(QApplication::clipboard()->mimeData()->data(QLatin1String("text/html")));

    QVERIFY(clipboardData.contains(QLatin1String("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />")));
    QVERIFY(clipboardData.contains(unicodeText));
}
#endif

void tst_QWebEnginePage::deleteQWebEngineViewTwice()
{
    for (int i = 0; i < 2; ++i) {
        QMainWindow mainWindow;
        QWebEngineView* webView = new QWebEngineView(&mainWindow);
        mainWindow.setCentralWidget(webView);
        webView->load(QUrl("qrc:///resources/frame_a.html"));
        mainWindow.show();
        QSignalSpy spyFinished(webView, &QWebEngineView::loadFinished);
        QVERIFY(spyFinished.wait());
    }
}

// TODO: Reimplement test without QStateMachine or add qtscxml module dependency
#if defined(QT_STATEMACHINE_LIB)
class SpyForLoadSignalsOrder : public QStateMachine {
    Q_OBJECT
public:
    SpyForLoadSignalsOrder(QWebEnginePage* page, QObject* parent = 0)
        : QStateMachine(parent)
    {
        connect(page, SIGNAL(loadProgress(int)), SLOT(onLoadProgress(int)));

        QState* waitingForLoadStarted = new QState(this);
        QState* waitingForFirstLoadProgress = new QState(this);
        QState* waitingForLastLoadProgress = new QState(this);
        QState* waitingForLoadFinished = new QState(this);
        QFinalState* final = new QFinalState(this);

        waitingForLoadStarted->addTransition(page, SIGNAL(loadStarted()), waitingForFirstLoadProgress);
        waitingForFirstLoadProgress->addTransition(this, SIGNAL(firstLoadProgress()), waitingForLastLoadProgress);
        waitingForLastLoadProgress->addTransition(this, SIGNAL(lastLoadProgress()), waitingForLoadFinished);
        waitingForLoadFinished->addTransition(page, SIGNAL(loadFinished(bool)), final);

        setInitialState(waitingForLoadStarted);
        start();
    }
    bool isFinished() const
    {
        return !isRunning();
    }
public Q_SLOTS:
    void onLoadProgress(int progress)
    {
        if (progress == 0)
            emit firstLoadProgress();
        else if (progress == 100)
            emit lastLoadProgress();
    }
Q_SIGNALS:
    void firstLoadProgress();
    void lastLoadProgress();
};

void tst_QWebEnginePage::loadSignalsOrder_data()
{
    QTest::addColumn<QUrl>("url");
    QTest::newRow("inline data") << QUrl("data:text/html,This is first page");
    QTest::newRow("simple page") << QUrl("qrc:///resources/content.html");
    QTest::newRow("frameset page") << QUrl("qrc:///resources/index.html");
}

void tst_QWebEnginePage::loadSignalsOrder()
{
    QFETCH(QUrl, url);
    QWebEnginePage page;
    SpyForLoadSignalsOrder loadSpy(&page);
    QSignalSpy spyLoadSpy(&loadSpy, &SpyForLoadSignalsOrder::started);
    QVERIFY(spyLoadSpy.wait(500));
    page.load(url);
    QTRY_VERIFY_WITH_TIMEOUT(loadSpy.isFinished(), 20000);
}
#endif // defined(QT_STATEMACHINE_LIB)

void tst_QWebEnginePage::renderWidgetHostViewNotShowTopLevel()
{
    QWebEnginePage page;
    QSignalSpy spyLoadFinished(&page, SIGNAL(loadFinished(bool)));

    page.load(QUrl("http://qt-project.org"));
    if (!spyLoadFinished.wait(10000) || !spyLoadFinished.at(0).at(0).toBool())
        QSKIP("Couldn't load page from network, skipping test.");
    spyLoadFinished.clear();

    // Loading a different domain will force the creation of a separate render
    // process and should therefore create a new RenderWidgetHostViewQtDelegateWidget.
    page.load(QUrl("http://www.wikipedia.org/"));
    if (!spyLoadFinished.wait(10000) || !spyLoadFinished.at(0).at(0).toBool())
        QSKIP("Couldn't load page from network, skipping test.");

    // Make sure that RenderWidgetHostViewQtDelegateWidgets are not shown as top-level.
    // They should only be made visible when parented to a QWebEngineView.
    const QList<QWidget *> widgets = QApplication::topLevelWidgets();
    for (QWidget *widget : widgets)
        QCOMPARE(widget->isVisible(), false);
}

class GetUserMediaTestPage : public QWebEnginePage {
Q_OBJECT

public:
    GetUserMediaTestPage()
        : m_gotRequest(false)
        , m_loadSucceeded(false)
    {
        connect(this, &QWebEnginePage::featurePermissionRequested, this, &GetUserMediaTestPage::onFeaturePermissionRequested);
        connect(this, &QWebEnginePage::loadFinished, [this](bool success){
            m_loadSucceeded = success;
        });
        // We need to load content from a resource in order for the securityOrigin to be valid.
        load(QUrl("qrc:///resources/content.html"));
    }

    void jsGetMedia(const QString &call)
    {
        evaluateJavaScriptSync(this,
            QStringLiteral(
                "var promiseFulfilled = false;"
                "var promiseRejected = false;"
                "navigator.mediaDevices.%1"
                ".then(stream => { promiseFulfilled = true})"
                ".catch(err => { promiseRejected = true})")
            .arg(call));
    }

    void jsGetUserMedia(const QString &constraints)
    {
        jsGetMedia(QStringLiteral("getUserMedia(%1)").arg(constraints));
    }

    bool jsPromiseFulfilled()
    {
        return evaluateJavaScriptSync(this, QStringLiteral("promiseFulfilled")).toBool();
    }

    bool jsPromiseRejected()
    {
        return evaluateJavaScriptSync(this, QStringLiteral("promiseRejected")).toBool();
    }

    void rejectPendingRequest()
    {
        setFeaturePermission(m_requestSecurityOrigin, m_requestedFeature, QWebEnginePage::PermissionDeniedByUser);
        m_gotRequest = false;
    }
    void acceptPendingRequest()
    {
        setFeaturePermission(m_requestSecurityOrigin, m_requestedFeature, QWebEnginePage::PermissionGrantedByUser);
        m_gotRequest = false;
    }

    bool gotFeatureRequest(QWebEnginePage::Feature feature)
    {
        return m_gotRequest && m_requestedFeature == feature;
    }

    bool gotFeatureRequest() const
    {
        return m_gotRequest;
    }

    bool loadSucceeded() const
    {
        return m_loadSucceeded;
    }

private Q_SLOTS:
    void onFeaturePermissionRequested(const QUrl &securityOrigin, QWebEnginePage::Feature feature)
    {
        m_requestedFeature = feature;
        m_requestSecurityOrigin = securityOrigin;
        m_gotRequest = true;
    }

private:
    bool m_gotRequest;
    bool m_loadSucceeded;
    QWebEnginePage::Feature m_requestedFeature;
    QUrl m_requestSecurityOrigin;

};

void tst_QWebEnginePage::getUserMediaRequest_data()
{
    QTest::addColumn<QString>("call");
    QTest::addColumn<QWebEnginePage::Feature>("feature");

    QTest::addRow("device audio")
        << "getUserMedia({audio: true})" << QWebEnginePage::MediaAudioCapture;
    QTest::addRow("device video")
        << "getUserMedia({video: true})" << QWebEnginePage::MediaVideoCapture;
    QTest::addRow("device audio+video")
        << "getUserMedia({audio: true, video: true})" << QWebEnginePage::MediaAudioVideoCapture;
    QTest::addRow("desktop video")
        << "getUserMedia({video: { mandatory: { chromeMediaSource: 'desktop' }}})"
        << QWebEnginePage::DesktopVideoCapture;
    QTest::addRow("desktop audio+video")
        << "getUserMedia({audio: { mandatory: { chromeMediaSource: 'desktop' }}, video: { mandatory: { chromeMediaSource: 'desktop' }}})"
        << QWebEnginePage::DesktopAudioVideoCapture;
    QTest::addRow("display video")
        << "getDisplayMedia()" << QWebEnginePage::DesktopVideoCapture;
}

void tst_QWebEnginePage::getUserMediaRequest()
{
    QFETCH(QString, call);
    QFETCH(QWebEnginePage::Feature, feature);

    GetUserMediaTestPage page;
    QWebEngineView view;
    if (feature == QWebEnginePage::DesktopVideoCapture || feature == QWebEnginePage::DesktopAudioVideoCapture) {
        // Desktop capture needs to be on a desktop.
        view.setPage(&page);
        view.resize(640, 480);
        view.show();
        QVERIFY(QTest::qWaitForWindowExposed(&view));
    }

    QTRY_VERIFY_WITH_TIMEOUT(page.loadSucceeded(), 60000);
    page.settings()->setAttribute(QWebEngineSettings::ScreenCaptureEnabled, true);

    // 1. Rejecting request on C++ side should reject promise on JS side.
    page.jsGetMedia(call);
    QTRY_VERIFY(page.gotFeatureRequest(feature));
    page.rejectPendingRequest();
    QTRY_VERIFY(!page.jsPromiseFulfilled() && page.jsPromiseRejected());

    // 2. Accepting request on C++ side should either fulfill or reject the
    // Promise on JS side. Due to the potential lack of physical media devices
    // deeper in the content layer we cannot guarantee that the promise will
    // always be fulfilled, however in this case an error should be returned to
    // JS instead of leaving the Promise in limbo.
    page.jsGetMedia(call);
    QTRY_VERIFY(page.gotFeatureRequest(feature));
    page.acceptPendingRequest();
    QTRY_VERIFY(page.jsPromiseFulfilled() || page.jsPromiseRejected());

    // 3. Media feature permissions are not remembered.
    page.jsGetMedia(call);
    QTRY_VERIFY(page.gotFeatureRequest(feature));
    page.acceptPendingRequest();
    QTRY_VERIFY(page.jsPromiseFulfilled() || page.jsPromiseRejected());
}

void tst_QWebEnginePage::getUserMediaRequestDesktopAudio()
{
    GetUserMediaTestPage page;
    QTRY_VERIFY_WITH_TIMEOUT(page.loadSucceeded(), 20000);
    page.settings()->setAttribute(QWebEngineSettings::ScreenCaptureEnabled, true);

    // Audio-only desktop capture is not supported. JS Promise should be
    // rejected immediately.

    page.jsGetUserMedia(
        QStringLiteral("{audio: { mandatory: { chromeMediaSource: 'desktop' }}}"));
    QTRY_VERIFY(!page.jsPromiseFulfilled() && page.jsPromiseRejected());

    page.jsGetUserMedia(
        QStringLiteral("{audio: { mandatory: { chromeMediaSource: 'desktop' }}, video: true}"));
    QTRY_VERIFY(!page.jsPromiseFulfilled() && page.jsPromiseRejected());
}

void tst_QWebEnginePage::getUserMediaRequestSettingDisabled()
{
    GetUserMediaTestPage page;
    QTRY_VERIFY_WITH_TIMEOUT(page.loadSucceeded(), 20000);
    page.settings()->setAttribute(QWebEngineSettings::ScreenCaptureEnabled, false);

    // With the setting disabled, the JS Promise should be rejected without
    // asking for permission first.

    page.jsGetUserMedia(QStringLiteral("{video: { mandatory: { chromeMediaSource: 'desktop' }}}"));
    QTRY_VERIFY(!page.jsPromiseFulfilled() && page.jsPromiseRejected());
}

// Try to trigger any possible race condition between the UI thread (permission
// management) and the audio/device thread (desktop capture initialization).
void tst_QWebEnginePage::getUserMediaRequestDesktopVideoManyPages()
{
    const QString constraints = QStringLiteral("{video: { mandatory: { chromeMediaSource: 'desktop' }}}");
    const QWebEnginePage::Feature feature = QWebEnginePage::DesktopVideoCapture;
    std::vector<GetUserMediaTestPage> pages(10);

    // Desktop capture needs to be on a desktop
    std::vector<QWebEngineView> views(10);
    for (size_t i = 0; i < views.size(); ++i) {
        QWebEngineView *view = &(views[i]);
        GetUserMediaTestPage *page = &(pages[i]);
        view->setPage(page);
        view->resize(640, 480);
        view->show();
        QVERIFY(QTest::qWaitForWindowExposed(view));
    }

    for (GetUserMediaTestPage &page : pages)
        QTRY_VERIFY_WITH_TIMEOUT(page.loadSucceeded(), 20000);
    for (GetUserMediaTestPage &page : pages)
        page.settings()->setAttribute(QWebEngineSettings::ScreenCaptureEnabled, true);
    for (GetUserMediaTestPage &page : pages)
        page.jsGetUserMedia(constraints);
    for (GetUserMediaTestPage &page : pages)
        QTRY_VERIFY(page.gotFeatureRequest(feature));
    for (GetUserMediaTestPage &page : pages)
        page.acceptPendingRequest();
    for (GetUserMediaTestPage &page : pages)
        QTRY_VERIFY(page.jsPromiseFulfilled() || page.jsPromiseRejected());
}

// Try to trigger any possible race condition between the UI or audio/device
// threads and the desktop capture thread, where the capture actually happens.
void tst_QWebEnginePage::getUserMediaRequestDesktopVideoManyRequests()
{
    const QString constraints = QStringLiteral("{video: { mandatory: { chromeMediaSource: 'desktop' }}}");
    const QWebEnginePage::Feature feature = QWebEnginePage::DesktopVideoCapture;
    GetUserMediaTestPage page;

    // Desktop capture needs to be on a desktop
    QWebEngineView view;
    view.setPage(&page);
    view.resize(640, 480);
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    QTRY_VERIFY_WITH_TIMEOUT(page.loadSucceeded(), 20000);
    page.settings()->setAttribute(QWebEngineSettings::ScreenCaptureEnabled, true);
    for (int i = 0; i != 100; ++i) {
        page.jsGetUserMedia(constraints);
        QTRY_VERIFY(page.gotFeatureRequest(feature));
        page.acceptPendingRequest();
        QTRY_VERIFY(page.jsPromiseFulfilled() || page.jsPromiseRejected());
    }
}

void tst_QWebEnginePage::savePage()
{
    QWebEngineView view;
    QWebEnginePage *page = view.page();

    connect(page->profile(), &QWebEngineProfile::downloadRequested,
            [] (QWebEngineDownloadRequest *item)
    {
        connect(item, &QWebEngineDownloadRequest::isFinishedChanged,
                &QTestEventLoop::instance(), &QTestEventLoop::exitLoop, Qt::QueuedConnection);
    });

    const QString urlPrefix = QStringLiteral("data:text/html,<h1>");
    const QString text = QStringLiteral("There is Thingumbob shouting!");
    page->load(QUrl(urlPrefix + text));
    QSignalSpy spyFinished(page, &QWebEnginePage::loadFinished);
    QVERIFY(spyFinished.wait());
    QCOMPARE(toPlainTextSync(page), text);

    // Save the loaded page as HTML.
    QTemporaryDir tempDir(QDir::tempPath() + "/tst_qwebengineview-XXXXXX");
    const QString filePath = tempDir.path() + "/thingumbob.html";
    page->save(filePath, QWebEngineDownloadRequest::CompleteHtmlSaveFormat);
    QTestEventLoop::instance().enterLoop(10);

    // Load something else.
    page->load(QUrl(urlPrefix + QLatin1String("It's a Snark!")));
    QVERIFY(spyFinished.wait());
    QVERIFY(toPlainTextSync(page) != text);

    // Load the saved page and compare the contents.
    page->load(QUrl::fromLocalFile(filePath));
    QVERIFY(spyFinished.wait());
    QCOMPARE(toPlainTextSync(page), text);
}

void tst_QWebEnginePage::openWindowDefaultSize()
{
    TestPage page;
    QSignalSpy spyFinished(&page, &QWebEnginePage::loadFinished);
    QSignalSpy windowCreatedSpy(&page, SIGNAL(windowCreated()));
    QWebEngineView view;
    view.setPage(&page);
    page.settings()->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, true);
    view.setUrl(QUrl("about:blank"));
    view.show();
    QTRY_COMPARE(spyFinished.size(), 1);

    // Open a default window.
    page.runJavaScript("window.open()");
    QTRY_COMPARE(windowCreatedSpy.size(), 1);
    // Open a too small window.
    evaluateJavaScriptSync(&page, "window.open('','about:blank','width=10,height=10')");
    QTRY_COMPARE(windowCreatedSpy.size(), 2);

    // The number of popups created should be two.
    QCOMPARE(page.createdWindows.size(), 2);

    QRect requestedGeometry = page.createdWindows[0]->requestedGeometry;
    // Check default size has been requested.
    QCOMPARE(requestedGeometry.width(), 0);
    QCOMPARE(requestedGeometry.height(), 0);

    requestedGeometry = page.createdWindows[1]->requestedGeometry;
    // Check minimum size has been requested.
    QCOMPARE(requestedGeometry.width(), 100);
    QCOMPARE(requestedGeometry.height(), 100);
}

bool tst_QWebEnginePage::isFalseJavaScriptResult(QWebEnginePage *page, const QString &javaScript)
{
    QVariant result = evaluateJavaScriptSync(page, javaScript);
    return !result.isNull() && result.isValid() && result == QVariant(false);
}

bool tst_QWebEnginePage::isTrueJavaScriptResult(QWebEnginePage *page, const QString &javaScript)
{
    QVariant result = evaluateJavaScriptSync(page, javaScript);
    return !result.isNull() && result.isValid() && result == QVariant(true);
}

bool tst_QWebEnginePage::isEmptyListJavaScriptResult(QWebEnginePage *page, const QString &javaScript)
{
    QVariant result = evaluateJavaScriptSync(page, javaScript);
    return !result.isNull() && result.isValid() && result == QList<QVariant>();
}

void tst_QWebEnginePage::runJavaScript()
{
    TestPage page;
    QVariant result;
    QVariantMap map;

    QVERIFY(isFalseJavaScriptResult(&page, "false"));
    QCOMPARE(evaluateJavaScriptSync(&page, "2").toInt(), 2);
    QCOMPARE(evaluateJavaScriptSync(&page, "2.5").toDouble(), 2.5);
    QCOMPARE(evaluateJavaScriptSync(&page, "\"Test\"").toString(), "Test");
    QVERIFY(isEmptyListJavaScriptResult(&page, "[]"));

    map.insert(QStringLiteral("test"), QVariant(2));
    QCOMPARE(evaluateJavaScriptSync(&page, "var el = {\"test\": 2}; el").toMap(), map);

    QVERIFY(evaluateJavaScriptSync(&page, "null").isNull());

    result = evaluateJavaScriptSync(&page, "undefined");
    QVERIFY(result.isNull() && !result.isValid());

    QCOMPARE(evaluateJavaScriptSync(&page, "new Date(42000)").toDate(), QVariant(42.0).toDate());
    QCOMPARE(evaluateJavaScriptSync(&page, "new ArrayBuffer(8)").toByteArray(), QByteArray(8, 0));

    result = evaluateJavaScriptSync(&page, "(function(){})");
    QVERIFY(result.isNull() && !result.isValid());

    QCOMPARE(evaluateJavaScriptSync(&page, "new Promise(function(){})"), QVariant(QVariantMap{}));
}

void tst_QWebEnginePage::runJavaScriptDisabled()
{
    QWebEnginePage page;
    QSignalSpy spy(&page, &QWebEnginePage::loadFinished);
    page.settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, false);
    // Settings changes take effect asynchronously. The load and wait ensure
    // that the settings are applied by the time we start to execute JavaScript.
    page.load(QStringLiteral("about:blank"));
    QTRY_COMPARE_WITH_TIMEOUT(spy.size(), 1, 20000);
    QCOMPARE(evaluateJavaScriptSyncInWorld(&page, QStringLiteral("1+1"), QWebEngineScript::MainWorld),
             QVariant());
    QCOMPARE(evaluateJavaScriptSyncInWorld(&page, QStringLiteral("1+1"), QWebEngineScript::ApplicationWorld),
             QVariant(2));
}

// Based on https://bugreports.qt.io/browse/QTBUG-73876
void tst_QWebEnginePage::runJavaScriptFromSlot()
{
    QWebEngineProfile profile;
    QWebEnginePage page(&profile);

    QSignalSpy loadFinishedSpy(&page, &QWebEnginePage::loadFinished);
    page.setHtml("<html><body>"
                 "  <input type='text' id='input1' value='QtWebEngine' size='50' />"
                 "</body></html>");
    QTRY_COMPARE(loadFinishedSpy.size(), 1);

    bool done = false;
    connect(&page, &QWebEnginePage::selectionChanged, [&]() {
        QTRY_COMPARE(evaluateJavaScriptSync(&page, QStringLiteral("2+2")), QVariant(4));
        done = true;
    });
    evaluateJavaScriptSync(&page, QStringLiteral("const input = document.getElementById('input1');"
                                                 "input.focus();"
                                                 "input.select();"));
    QTRY_VERIFY(done);
}

void tst_QWebEnginePage::fullScreenRequested()
{
    QWebEngineView view;
    QWebEnginePage* page = view.page();
    view.resize(640, 480);
    view.show();

    page->settings()->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);

    QSignalSpy loadSpy(&view, SIGNAL(loadFinished(bool)));
    page->load(QUrl("qrc:///resources/fullscreen.html"));
    QTRY_COMPARE(loadSpy.size(), 1);

    QTRY_VERIFY(isTrueJavaScriptResult(page, "document.webkitFullscreenEnabled"));
    QVERIFY(isFalseJavaScriptResult(page, "document.webkitIsFullScreen"));

    // FullscreenRequest must be a user gesture
    bool acceptRequest = true;
    connect(page, &QWebEnginePage::fullScreenRequested,
        [&acceptRequest](QWebEngineFullScreenRequest request) {
        if (acceptRequest) request.accept(); else request.reject();
    });

    QTest::keyPress(view.focusProxy(), Qt::Key_Space);
    QTRY_VERIFY(isTrueJavaScriptResult(page, "document.webkitIsFullScreen"));

    QTest::mouseMove(view.windowHandle(), QPoint(10,10));
    QTest::mouseClick(view.windowHandle(), Qt::RightButton);
    QTRY_COMPARE(view.findChildren<QMenu *>().size(), 1);
    auto menu = view.findChildren<QMenu *>().first();
    QVERIFY(menu->actions().contains(page->action(QWebEnginePage::ExitFullScreen)));

    page->runJavaScript("document.webkitExitFullscreen()");
    QTRY_VERIFY(isFalseJavaScriptResult(page, "document.webkitIsFullScreen"));

    acceptRequest = false;

    QVERIFY(isTrueJavaScriptResult(page, "document.webkitFullscreenEnabled"));
    QTest::keyPress(view.focusProxy(), Qt::Key_Space);
    QTRY_VERIFY(isFalseJavaScriptResult(page, "document.webkitIsFullScreen"));
}

void tst_QWebEnginePage::requestQuota_data()
{
    QTest::addColumn<QString>("storage");
    QTest::addRow("webkitPersistentStorage") << "navigator.webkitPersistentStorage";
    QTest::addRow("webkitTemporaryStorage") << "navigator.webkitTemporaryStorage";

}

void tst_QWebEnginePage::requestQuota()
{
    QFETCH(QString, storage);

    ConsolePage page;
    QWebEngineView view;
    view.setPage(&page);
    QSignalSpy loadFinishedSpy(&page, SIGNAL(loadFinished(bool)));
    page.load(QUrl("qrc:///resources/content.html"));
    QVERIFY(loadFinishedSpy.wait());

    evaluateJavaScriptSync(&page, QString(
        "var storage = %1;"
        "storage.requestQuota(1024, function(grantedSize) {"
        "   console.log(grantedSize);"
        "});").arg(storage));
    QTRY_COMPARE(page.messages.size(), 1);
    QTRY_COMPARE(page.messages[0], QString("1024"));

    evaluateJavaScriptSync(&page, QString(
        "var storage = %1;"
        "storage.queryUsageAndQuota(function(usedBytes, grantedBytes) {"
        "   console.log(usedBytes);"
        "   console.log(grantedBytes);"
        "});").arg(storage));
    QTRY_COMPARE(page.messages.size(), 3);
    QTRY_COMPARE(page.messages[1], QString("0"));
    QTRY_VERIFY(page.messages[2].toLongLong() >= 1024);
}

void tst_QWebEnginePage::symmetricUrl()
{
    QWebEngineView view;
    QSignalSpy loadFinishedSpy(view.page(), SIGNAL(loadFinished(bool)));

    QVERIFY(view.url().isEmpty());

    QCOMPARE(view.history()->count(), 0);

    QUrl dataUrl("data:text/html,<h1>Test");

    view.setUrl(dataUrl);
    view.show();
    QCOMPARE(view.url(), dataUrl);
    QCOMPARE(view.history()->count(), 0);

    // loading is _not_ immediate, so the text isn't set just yet.
    QVERIFY(toPlainTextSync(view.page()).isEmpty());

    QTRY_COMPARE_WITH_TIMEOUT(loadFinishedSpy.size(), 1, 20000);

    QCOMPARE(view.history()->count(), 1);
    QCOMPARE(toPlainTextSync(view.page()), QString("Test"));

    QUrl dataUrl2("data:text/html,<h1>Test2");
    QUrl dataUrl3("data:text/html,<h1>Test3");

    view.setUrl(dataUrl2);
    view.setUrl(dataUrl3);

    QCOMPARE(view.url(), dataUrl3);

    // setUrl(dataUrl3) might override the pending load for dataUrl2. Or not.
    QTRY_VERIFY(loadFinishedSpy.size() >= 2);
    QTRY_VERIFY(loadFinishedSpy.size() <= 3);

    // setUrl(dataUrl3) might stop Chromium from adding a navigation entry for dataUrl2,
    // depending on whether the load of dataUrl2 could be completed in time.
    QVERIFY(view.history()->count() >= 2);
    QVERIFY(view.history()->count() <= 3);

    QCOMPARE(toPlainTextSync(view.page()), QString("Test3"));
}

void tst_QWebEnginePage::progressSignal()
{
    QSignalSpy progressSpy(m_view, SIGNAL(loadProgress(int)));

    QUrl dataUrl("data:text/html,<h1>Test");
    m_view->setUrl(dataUrl);

    QSignalSpy spyFinished(m_view, &QWebEngineView::loadFinished);
    QVERIFY(spyFinished.wait());

    QVERIFY(progressSpy.size() >= 2);
    int previousValue = -1;
    for (QSignalSpy::ConstIterator it = progressSpy.begin(); it < progressSpy.end(); ++it) {
        int current = (*it).first().toInt();
        // verbose output for faulty condition
        if (!(current >= previousValue)) {
            qDebug() << "faulty progress values:";
            for (QSignalSpy::ConstIterator it2 = progressSpy.begin(); it2 < progressSpy.end(); ++it2)
                qDebug() << (*it2).first().toInt();
            QVERIFY(current >= previousValue);
        }
        previousValue = current;
    }

    // But we always end at 100%
    QCOMPARE(progressSpy.last().first().toInt(), 100);
}

void tst_QWebEnginePage::urlChange()
{
    QSignalSpy urlSpy(m_page, &QWebEnginePage::urlChanged);

    QUrl dataUrl("data:text/html,<h1>Test");
    m_view->setUrl(dataUrl);

    QTRY_COMPARE(urlSpy.size(), 1);
    QCOMPARE(urlSpy.takeFirst().value(0).toUrl(), dataUrl);

    QUrl dataUrl2("data:text/html,<html><head><title>title</title></head><body><h1>Test</body></html>");
    m_view->setUrl(dataUrl2);

    QTRY_COMPARE(urlSpy.size(), 1);
    QCOMPARE(urlSpy.takeFirst().value(0).toUrl(), dataUrl2);

    QUrl testUrl("http://test.qt.io/");
    m_view->setHtml(QStringLiteral("<h1>Test</h1"), testUrl);

    QTRY_COMPARE(urlSpy.size(), 1);
    QCOMPARE(urlSpy.takeFirst().value(0).toUrl(), testUrl);
}

class FakeReply : public QNetworkReply {
    Q_OBJECT

public:
    static const QUrl urlFor404ErrorWithoutContents;

    FakeReply(const QNetworkRequest& request, QObject* parent = 0)
        : QNetworkReply(parent)
    {
        setOperation(QNetworkAccessManager::GetOperation);
        setRequest(request);
        setUrl(request.url());
        if (request.url() == QUrl("qrc:/test1.html")) {
            setHeader(QNetworkRequest::LocationHeader, QString("qrc:/test2.html"));
            setAttribute(QNetworkRequest::RedirectionTargetAttribute, QUrl("qrc:/test2.html"));
            QTimer::singleShot(0, this, SLOT(continueRedirect()));
        }
#if QT_CONFIG(openssl)
        else if (request.url() == QUrl("qrc:/fake-ssl-error.html")) {
            setError(QNetworkReply::SslHandshakeFailedError, tr("Fake error!"));
            QTimer::singleShot(0, this, SLOT(continueError()));
        }
#endif
        else if (request.url().host() == QLatin1String("abcdef.abcdef")) {
            setError(QNetworkReply::HostNotFoundError, tr("Invalid URL"));
            QTimer::singleShot(0, this, SLOT(continueError()));
        } else if (request.url() == FakeReply::urlFor404ErrorWithoutContents) {
            setError(QNetworkReply::ContentNotFoundError, "Not found");
            setAttribute(QNetworkRequest::HttpStatusCodeAttribute, 404);
            QTimer::singleShot(0, this, SLOT(continueError()));
        }

        open(QIODevice::ReadOnly);
    }
    ~FakeReply()
    {
        close();
    }
    void abort() override {}
    void close() override {}

protected:
    qint64 readData(char*, qint64) override
    {
        return 0;
    }

private Q_SLOTS:
    void continueRedirect()
    {
        emit metaDataChanged();
        emit finished();
    }

    void continueError()
    {
        emit errorOccurred(this->error());
        emit finished();
    }
};

const QUrl FakeReply::urlFor404ErrorWithoutContents = QUrl("http://this.will/return-http-404-error-without-contents.html");

class FakeNetworkManager : public QNetworkAccessManager {
    Q_OBJECT

public:
    FakeNetworkManager(QObject* parent) : QNetworkAccessManager(parent) { }

protected:
    QNetworkReply* createRequest(Operation op, const QNetworkRequest& request, QIODevice* outgoingData) override
    {
        QString url = request.url().toString();
        if (op == QNetworkAccessManager::GetOperation) {
#if QT_CONFIG(openssl)
            if (url == "qrc:/fake-ssl-error.html") {
                FakeReply* reply = new FakeReply(request, this);
                QList<QSslError> errors;
                emit sslErrors(reply, errors << QSslError(QSslError::UnspecifiedError));
                return reply;
            }
#endif
            if (url == "qrc:/test1.html" || url == "http://abcdef.abcdef/" || request.url() == FakeReply::urlFor404ErrorWithoutContents)
                return new FakeReply(request, this);
        }

        return QNetworkAccessManager::createRequest(op, request, outgoingData);
    }
};

void tst_QWebEnginePage::requestedUrlAfterSetAndLoadFailures()
{
    QWebEnginePage page;
    page.settings()->setAttribute(QWebEngineSettings::ErrorPageEnabled, false);
    QSignalSpy spy(&page, SIGNAL(loadFinished(bool)));

    const QUrl first("http://abcdef.abcdef/");
    page.setUrl(first);
    QTRY_COMPARE_WITH_TIMEOUT(spy.size(), 1, 20000);
    QCOMPARE(page.url(), first);
    QCOMPARE(page.requestedUrl(), first);
    QVERIFY(!spy.at(0).first().toBool());

    const QUrl second("http://abcdef.abcdef/another_page.html");
    QVERIFY(first != second);

    page.load(second);
    QTRY_COMPARE_WITH_TIMEOUT(spy.size(), 2, 20000);
    QCOMPARE(page.url(), first);
    QCOMPARE(page.requestedUrl(), second);
    QVERIFY(!spy.at(1).first().toBool());
}

void tst_QWebEnginePage::asyncAndDelete()
{
    QScopedPointer<QWebEnginePage> page(new QWebEnginePage);
    CallbackSpy<QString> plainTextSpy;
    CallbackSpy<QString> htmlSpy;
    page->toPlainText(plainTextSpy.ref());
    page->toHtml(htmlSpy.ref());

    page.reset();
    // Pending callbacks should be called with an empty value in the page's destructor.
    QCOMPARE(plainTextSpy.waitForResult(), QString());
    QVERIFY(plainTextSpy.wasCalled());
    QCOMPARE(htmlSpy.waitForResult(), QString());
    QVERIFY(htmlSpy.wasCalled());
}

void tst_QWebEnginePage::earlyToHtml()
{
    QString html("<html><head></head><body></body></html>");
    QCOMPARE(toHtmlSync(m_view->page()), html);
}

void tst_QWebEnginePage::setHtml()
{
    QString html("<html><head></head><body><p>hello world</p></body></html>");
    QSignalSpy spy(m_view->page(), SIGNAL(loadFinished(bool)));
    m_view->page()->setHtml(html);
    QVERIFY(spy.wait());
    QCOMPARE(toHtmlSync(m_view->page()), html);
}

void tst_QWebEnginePage::setHtmlWithImageResource()
{
    // We allow access to qrc resources from any security origin, including local and anonymous

    QLatin1String html("<html><body><p>hello world</p><img src='qrc:/resources/image.png'/></body></html>");
    QWebEnginePage page;

    QSignalSpy spy(&page, SIGNAL(loadFinished(bool)));
    page.setHtml(html, QUrl("file:///path/to/file"));
    QTRY_COMPARE_WITH_TIMEOUT(spy.size(), 1, 12000);

    QCOMPARE(evaluateJavaScriptSync(&page, "document.images.length").toInt(), 1);
    QCOMPARE(evaluateJavaScriptSync(&page, "document.images[0].width").toInt(), 128);
    QCOMPARE(evaluateJavaScriptSync(&page, "document.images[0].height").toInt(), 128);

    // Now we test the opposite: without a baseUrl as a local file, we can still request qrc resources.

    page.setHtml(html);
    QTRY_COMPARE(spy.size(), 2);
    QCOMPARE(evaluateJavaScriptSync(&page, "document.images.length").toInt(), 1);
    QCOMPARE(evaluateJavaScriptSync(&page, "document.images[0].width").toInt(), 128);
    QCOMPARE(evaluateJavaScriptSync(&page, "document.images[0].height").toInt(), 128);
}

void tst_QWebEnginePage::setHtmlWithStylesheetResource()
{
    const char* htmlData =
        "<html>"
            "<head>"
                "<link rel='stylesheet' href='qrc:/resources/style.css' type='text/css' />"
            "</head>"
            "<body>"
                "<p id='idP'>some text</p>"
            "</body>"
        "</html>";
    QLatin1String html(htmlData);
    QWebEnginePage page;
    QSignalSpy spyFinished(&page, &QWebEnginePage::loadFinished);

    // We allow access to qrc resources from any security origin, including local and anonymous
    page.setHtml(html, QUrl("file:///path/to/file"));
    QVERIFY(spyFinished.wait());
    QCOMPARE(evaluateJavaScriptSync(&page, "window.getComputedStyle(document.getElementById('idP')).color").toString(), QString("rgb(255, 0, 0)"));

    page.setHtml(html, QUrl(QLatin1String("qrc:/")));
    QVERIFY(spyFinished.wait());
    QCOMPARE(evaluateJavaScriptSync(&page, "window.getComputedStyle(document.getElementById('idP')).color").toString(), QString("rgb(255, 0, 0)"));

    // Now we test the opposite: without a baseUrl as a local file, we can still request qrc resources.
    page.setHtml(html);
    QVERIFY(spyFinished.wait());
    QCOMPARE(evaluateJavaScriptSync(&page, "window.getComputedStyle(document.getElementById('idP')).color").toString(), QString("rgb(255, 0, 0)"));
}

void tst_QWebEnginePage::setHtmlWithBaseURL()
{
    // This tests if baseUrl is indeed affecting the relative paths from resources.
    // As we are using a local file as baseUrl, its security origin should be able to load local resources.

    if (!QDir(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()).exists())
        W_QSKIP(QString("This test requires access to resources found in '%1'")
                        .arg(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath())
                        .toLatin1()
                        .constData(),
                SkipAll);

    QDir::setCurrent(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath());
    qDebug()<<QDir::current();

    QString html("<html><body><p>hello world</p><img src='resources/image2.png'/></body></html>");

    QWebEnginePage page;

    // in few seconds, the image should be completey loaded
    QSignalSpy spy(&page, SIGNAL(loadFinished(bool)));

    page.setHtml(html,
                 QUrl::fromLocalFile(
                         QString("%1/foo.html").arg(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath())));
    QSignalSpy spyFinished(&page, &QWebEnginePage::loadFinished);
    QVERIFY(spyFinished.wait());
    QCOMPARE(spy.size(), 1);

    QCOMPARE(evaluateJavaScriptSync(&page, "document.images.length").toInt(), 1);
    QCOMPARE(evaluateJavaScriptSync(&page, "document.images[0].width").toInt(), 128);
    QCOMPARE(evaluateJavaScriptSync(&page, "document.images[0].height").toInt(), 128);

    // no history item has to be added.
    QCOMPARE(m_view->page()->history()->count(), 0);
}

class MyPage : public QWebEnginePage
{
public:
    MyPage() :  QWebEnginePage(), alerts(0) {}
    int alerts;

protected:
    void javaScriptAlert(const QUrl &securityOrigin, const QString &msg) override
    {
        alerts++;
        QCOMPARE(securityOrigin, QUrl(QStringLiteral("http://test.origin.com/")));
        QCOMPARE(msg, QString("foo"));
    }
};

void tst_QWebEnginePage::setHtmlWithJSAlert()
{
    QString html("<html><head></head><body><script>alert('foo');</script><p>hello world</p></body></html>");
    MyPage page;
    page.setHtml(html, QUrl(QStringLiteral("http://test.origin.com/path#fragment")));
    QSignalSpy spyFinished(&page, &QWebEnginePage::loadFinished);
    QVERIFY(spyFinished.wait());
    QCOMPARE(page.alerts, 1);
    QCOMPARE(toHtmlSync(&page), html);
}

void tst_QWebEnginePage::setHtmlWithModuleImport()
{
    HttpServer server;
    connect(&server, &HttpServer::newRequest, [&](HttpReqRep *rr) {
        if (rr->requestMethod() == "GET" && rr->requestPath() == "/fibonacci.mjs") {
            rr->setResponseBody("export function fib(n) {\n"
                                "    return n < 2 ? n : fib(n-1) + fib(n-2)\n"
                                "}\n");
            rr->setResponseHeader("Content-Type", "text/javascript");
            rr->sendResponse();
        }
    });
    QVERIFY(server.start());

    QString html("<html>\n"
                 "  <head>\n"
                 "    <script type='module'>\n"
                 "      import {fib} from './fibonacci.mjs'\n"
                 "      window.fib7 = fib(7)\n"
                 "    </script>\n"
                 "  </head>\n"
                 "  <body></body>\n"
                 "</html>\n");

    QWebEnginePage page;
    QSignalSpy spy(&page, &QWebEnginePage::loadFinished);
    page.setHtml(html, server.url());
    QVERIFY(spy.size() || spy.wait());

    QCOMPARE(evaluateJavaScriptSync(&page, "fib7"), QVariant(13));
}

void tst_QWebEnginePage::baseUrl_data()
{
    QTest::addColumn<QString>("html");
    QTest::addColumn<QUrl>("loadUrl");
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<QUrl>("baseUrl");

    QTest::newRow("null") << QString() << QUrl()
                          << QUrl("about:blank") << QUrl("about:blank");

    QTest::newRow("foo") << QString() << QUrl("http://foobar.baz/")
                         << QUrl("http://foobar.baz/") << QUrl("http://foobar.baz/");

    QString html = "<html>"
        "<head>"
            "<base href=\"http://foobaz.bar/\" />"
        "</head>"
    "</html>";
    QTest::newRow("customBaseUrl") << html << QUrl("http://foobar.baz/")
                                   << QUrl("http://foobar.baz/") << QUrl("http://foobaz.bar/");
}

void tst_QWebEnginePage::baseUrl()
{
    QFETCH(QString, html);
    QFETCH(QUrl, loadUrl);
    QFETCH(QUrl, url);
    QFETCH(QUrl, baseUrl);

    QSignalSpy loadSpy(m_page, SIGNAL(loadFinished(bool)));
    m_page->setHtml(html, loadUrl);
    QTRY_COMPARE(loadSpy.size(), 1);
    QCOMPARE(m_page->url(), url);
    QEXPECT_FAIL("null", "Slight change: We now translate QUrl() to about:blank for the virtual url, but not for the baseUrl", Continue);
    QCOMPARE(baseUrlSync(m_page), baseUrl);
}

void tst_QWebEnginePage::scrollPosition()
{
    // enlarged image in a small viewport, to provoke the scrollbars to appear
    QString html(
            "<html><body><img src='qrc:/resources/image.png' height=500 width=500/></body></html>");

    QWebEngineView view;
    view.setFixedSize(200,200);
    view.show();

    QVERIFY(QTest::qWaitForWindowExposed(&view));

    QSignalSpy loadSpy(view.page(), SIGNAL(loadFinished(bool)));
    view.setHtml(html);
    QTRY_COMPARE(loadSpy.size(), 1);

    // try to set the scroll offset programmatically
    view.page()->runJavaScript("window.scrollTo(23, 29);");
    QTRY_COMPARE(view.page()->scrollPosition().x(), 23);
    QCOMPARE(view.page()->scrollPosition().y(), 29);

    int x = evaluateJavaScriptSync(view.page(), "window.scrollX").toInt();
    int y = evaluateJavaScriptSync(view.page(), "window.scrollY").toInt();
    QCOMPARE(x, 23);
    QCOMPARE(y, 29);
}

void tst_QWebEnginePage::scrollbarsOff()
{
    QWebEngineView view;
    view.page()->settings()->setAttribute(QWebEngineSettings::ShowScrollBars, false);

    QString html("<html><body>"
                 "   <div style='margin-top:1000px ; margin-left:1000px'>"
                 "       <a id='offscreen' href='a'>End</a>"
                 "   </div>"
                 "</body></html>");

    QSignalSpy loadSpy(&view, SIGNAL(loadFinished(bool)));
    view.setHtml(html);
    QTRY_COMPARE(loadSpy.size(), 1);
    QVERIFY(evaluateJavaScriptSync(view.page(), "innerWidth == document.documentElement.offsetWidth").toBool());
}

class WebView : public QWebEngineView
{
    Q_OBJECT
signals:
    void repaintRequested();

protected:
    bool event(QEvent *event) override
    {
        if (event->type() == QEvent::UpdateRequest)
            emit repaintRequested();

        return QWebEngineView::event(event);
    }
};

void tst_QWebEnginePage::evaluateWillCauseRepaint()
{
    WebView view;
    view.resize(640, 480);
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    QString html("<html><body>"
                 "  top"
                 "  <div id=\"junk\" style=\"display: block;\">junk</div>"
                 "  bottom"
                 "</body></html>");

    QSignalSpy loadSpy(&view, SIGNAL(loadFinished(bool)));
    view.setHtml(html);
    QTRY_COMPARE(loadSpy.size(), 1);

    evaluateJavaScriptSync(view.page(), "document.getElementById('junk').style.display = 'none';");
    QSignalSpy repaintSpy(&view, &WebView::repaintRequested);
    QVERIFY(repaintSpy.wait());
}

void tst_QWebEnginePage::setContent_data()
{
    QTest::addColumn<QString>("mimeType");
    QTest::addColumn<QByteArray>("testContents");
    QTest::addColumn<QString>("expected");

    QString str = QString::fromUtf8("ὕαλον ϕαγεῖν δύναμαι· τοῦτο οὔ με βλάπτει");
    QTest::newRow("UTF-8 plain text") << "text/plain; charset=utf-8" << str.toUtf8() << str;

    QBuffer out16;
    out16.open(QIODevice::WriteOnly);
    QTextStream stream16(&out16);
    stream16.setEncoding(QStringConverter::Utf16);
    stream16 << str;
    stream16.flush();
    QTest::newRow("UTF-16 plain text") << "text/plain; charset=utf-16" << out16.buffer() << str;

    str = QString::fromUtf8("Une chaîne de caractères à sa façon.");
    QTest::newRow("latin-1 plain text") << "text/plain; charset=iso-8859-1" << str.toLatin1() << str;


}

void tst_QWebEnginePage::setContent()
{
    QFETCH(QString, mimeType);
    QFETCH(QByteArray, testContents);
    QFETCH(QString, expected);
    QSignalSpy loadSpy(m_page, SIGNAL(loadFinished(bool)));
    m_view->setContent(testContents, mimeType);
    QVERIFY(loadSpy.wait());
    QCOMPARE(toPlainTextSync(m_view->page()), expected);
}

class CacheNetworkAccessManager : public QNetworkAccessManager {
public:
    CacheNetworkAccessManager(QObject* parent = 0)
        : QNetworkAccessManager(parent)
        , m_lastCacheLoad(QNetworkRequest::PreferNetwork)
    {
    }

    QNetworkReply* createRequest(Operation, const QNetworkRequest& request, QIODevice*) override
    {
        QVariant cacheLoad = request.attribute(QNetworkRequest::CacheLoadControlAttribute);
        if (cacheLoad.isValid())
            m_lastCacheLoad = static_cast<QNetworkRequest::CacheLoadControl>(cacheLoad.toUInt());
        else
            m_lastCacheLoad = QNetworkRequest::PreferNetwork; // default value
        return new FakeReply(request, this);
    }

    QNetworkRequest::CacheLoadControl lastCacheLoad() const
    {
        return m_lastCacheLoad;
    }

private:
    QNetworkRequest::CacheLoadControl m_lastCacheLoad;
};

void tst_QWebEnginePage::setUrlWithPendingLoads()
{
    QWebEnginePage page;
    page.setHtml("<img src='dummy:'/>");
    page.setUrl(QUrl("about:blank"));
}

void tst_QWebEnginePage::setUrlToEmpty()
{
    int expectedLoadFinishedCount = 0;
    const QUrl aboutBlank("about:blank");
    const QUrl url("qrc:/resources/test2.html");

    QWebEnginePage page;
    QCOMPARE(page.url(), QUrl());
    QCOMPARE(page.requestedUrl(), QUrl());
// Chromium now returns about:blank as the base url here:
//     QCOMPARE(baseUrlSync(&page), QUrl());

    QSignalSpy spy(&page, SIGNAL(loadFinished(bool)));

    // Set existing url
    page.setUrl(url);
    expectedLoadFinishedCount++;
    QVERIFY(spy.wait());

    QCOMPARE(spy.size(), expectedLoadFinishedCount);
    QCOMPARE(page.url(), url);
    QCOMPARE(page.requestedUrl(), url);
    QCOMPARE(baseUrlSync(&page), url);

    // Set empty url
    page.setUrl(QUrl());
    expectedLoadFinishedCount++;

    QTRY_COMPARE(spy.size(), expectedLoadFinishedCount);
    QCOMPARE(page.url(), aboutBlank);
    QCOMPARE(page.requestedUrl(), QUrl());
    QCOMPARE(baseUrlSync(&page), aboutBlank);

    // Set existing url
    page.setUrl(url);
    expectedLoadFinishedCount++;

    QTRY_COMPARE(spy.size(), expectedLoadFinishedCount);
    QCOMPARE(page.url(), url);
    QCOMPARE(page.requestedUrl(), url);
    QCOMPARE(baseUrlSync(&page), url);

    // Load empty url
    page.load(QUrl());
    expectedLoadFinishedCount++;

    QTRY_COMPARE(spy.size(), expectedLoadFinishedCount);
    QCOMPARE(page.url(), aboutBlank);
    QCOMPARE(page.requestedUrl(), QUrl());
    QCOMPARE(baseUrlSync(&page), aboutBlank);
}

void tst_QWebEnginePage::setUrlToInvalid()
{
    QEXPECT_FAIL("", "Unsupported: QtWebEngine doesn't adjust invalid URLs.", Abort);
    QVERIFY(false);

    QWebEnginePage page;

    const QUrl invalidUrl("http:/example.com");
    QVERIFY(!invalidUrl.isEmpty());
    QVERIFY(invalidUrl != QUrl());

    // QWebEnginePage will do its best to accept the URL, possible converting it to a valid equivalent URL.
    const QUrl validUrl("http://example.com/");
    page.setUrl(invalidUrl);
    QCOMPARE(page.url(), validUrl);
    QCOMPARE(page.requestedUrl(), validUrl);
    QCOMPARE(baseUrlSync(&page), validUrl);

    // QUrls equivalent to QUrl() will be treated as such.
    const QUrl aboutBlank("about:blank");
    const QUrl anotherInvalidUrl("1http://bugs.webkit.org");
    QVERIFY(!anotherInvalidUrl.isEmpty()); // and they are not necessarily empty.
    QVERIFY(!anotherInvalidUrl.isValid());
    QCOMPARE(anotherInvalidUrl.toEncoded(), QUrl().toEncoded());

    page.setUrl(anotherInvalidUrl);
    QCOMPARE(page.url(), aboutBlank);
    QCOMPARE(page.requestedUrl().toEncoded(), anotherInvalidUrl.toEncoded());
    QCOMPARE(baseUrlSync(&page), aboutBlank);
}

void tst_QWebEnginePage::setUrlToBadDomain()
{
    // Failing to load a URL should still emit a urlChanged signal.
    //
    // This test is based on the scenario in QTBUG-48995 where the second setUrl
    // call first triggers an unexpected additional urlChanged signal with the
    // original url before the expected signal with the new url.

    // RFC 2606 says the .invalid TLD should be invalid.
    const QUrl url1 = QStringLiteral("http://this.is.definitely.invalid/");
    const QUrl url2 = QStringLiteral("http://this.is.also.invalid/");
    QWebEnginePage page;
    QSignalSpy urlSpy(&page, &QWebEnginePage::urlChanged);
    QSignalSpy titleSpy(&page, &QWebEnginePage::titleChanged);
    QSignalSpy loadSpy(&page, &QWebEnginePage::loadFinished);

    page.setUrl(url1);

    QTRY_COMPARE(urlSpy.size(), 1);
    QTRY_COMPARE_WITH_TIMEOUT(titleSpy.size(), 1, 20000);
    QTRY_COMPARE(loadSpy.size(), 1);

    QCOMPARE(urlSpy.takeFirst().value(0).toUrl(), url1);
    QCOMPARE(titleSpy.takeFirst().value(0).toString(), url1.host());
    QCOMPARE(loadSpy.takeFirst().value(0).toBool(), false);

    QCOMPARE(page.url(), url1);
    QCOMPARE(page.title(), url1.host());

    page.setUrl(url2);

    QTRY_COMPARE(urlSpy.size(), 1);
    QTRY_COMPARE_WITH_TIMEOUT(titleSpy.size(), 1, 20000);
    QTRY_COMPARE(loadSpy.size(), 1);

    QCOMPARE(urlSpy.takeFirst().value(0).toUrl(), url2);
    QCOMPARE(titleSpy.takeFirst().value(0).toString(), url2.host());
    QCOMPARE(loadSpy.takeFirst().value(0).toBool(), false);

    QCOMPARE(page.url(), url2);
    QCOMPARE(page.title(), url2.host());
}

void tst_QWebEnginePage::setUrlToBadPort()
{
    // Failing to load a URL should still emit a urlChanged signal.

    // Ports 244-245 are hopefully unbound (marked unassigned in RFC1700).
    const QUrl url1 = QStringLiteral("http://127.0.0.1:244/");
    const QUrl url2 = QStringLiteral("http://127.0.0.1:245/");
    QWebEnginePage page;
    QSignalSpy urlSpy(&page, &QWebEnginePage::urlChanged);
    QSignalSpy titleSpy(&page, &QWebEnginePage::titleChanged);
    QSignalSpy loadSpy(&page, &QWebEnginePage::loadFinished);

    page.setUrl(url1);

    QTRY_COMPARE(urlSpy.size(), 1);
    QTRY_COMPARE(titleSpy.size(), 2);
    QTRY_COMPARE(loadSpy.size(), 1);

    QCOMPARE(urlSpy.takeFirst().value(0).toUrl(), url1);
    QCOMPARE(titleSpy.takeFirst().value(0).toString(), url1.authority());
    QCOMPARE(titleSpy.takeFirst().value(0).toString(), url1.host());
    QCOMPARE(loadSpy.takeFirst().value(0).toBool(), false);

    QCOMPARE(page.url(), url1);
    QCOMPARE(page.title(), url1.host());

    page.setUrl(url2);

    QTRY_COMPARE(urlSpy.size(), 1);
    QTRY_COMPARE(titleSpy.size(), 2);
    QTRY_COMPARE(loadSpy.size(), 1);

    QCOMPARE(urlSpy.takeFirst().value(0).toUrl(), url2);
    QCOMPARE(titleSpy.takeFirst().value(0).toString(), url2.authority());
    QCOMPARE(titleSpy.takeFirst().value(0).toString(), url2.host());
    QCOMPARE(loadSpy.takeFirst().value(0).toBool(), false);

    QCOMPARE(page.url(), url2);
    QCOMPARE(page.title(), url2.host());
}

static QStringList collectHistoryUrls(QWebEngineHistory *history)
{
    QStringList urls;
    const QList<QWebEngineHistoryItem> items = history->items();
    for (const QWebEngineHistoryItem &i : items)
        urls << i.url().toString();
    return urls;
}

void tst_QWebEnginePage::setUrlHistory()
{
    const QUrl aboutBlank("about:blank");
    QUrl url;
    int expectedLoadFinishedCount = 0;
    QSignalSpy spy(m_page, SIGNAL(loadFinished(bool)));

    QCOMPARE(m_page->history()->count(), 0);

    m_page->setUrl(QUrl());
    expectedLoadFinishedCount++;
    QTRY_COMPARE(spy.size(), expectedLoadFinishedCount);
    QCOMPARE(m_page->url(), aboutBlank);
    QCOMPARE(m_page->requestedUrl(), QUrl());
    // Chromium stores navigation entry for every successful loads. The load of the empty page is committed and stored as about:blank.
    QCOMPARE(collectHistoryUrls(m_page->history()), QStringList() << aboutBlank.toString());

    url = QUrl("http://url.invalid/");
    m_page->setUrl(url);
    expectedLoadFinishedCount++;
    QTRY_COMPARE_WITH_TIMEOUT(spy.size(), expectedLoadFinishedCount, 20000);
    // When error page is disabled in case of LoadFail the entry of the unavailable page is not stored.
    // We expect the url of the previously loaded page here.
    QCOMPARE(m_page->url(), aboutBlank);
    QCOMPARE(m_page->requestedUrl(), QUrl());
    // Since the entry of the unavailable page is not stored it will not available in the history.
    QCOMPARE(collectHistoryUrls(m_page->history()), QStringList() << aboutBlank.toString());

    url = QUrl("qrc:/resources/test1.html");
    m_page->setUrl(url);
    expectedLoadFinishedCount++;
    QTRY_COMPARE(spy.size(), expectedLoadFinishedCount);
    QCOMPARE(m_page->url(), url);
    QCOMPARE(m_page->requestedUrl(), url);
    QCOMPARE(collectHistoryUrls(m_page->history()), QStringList() << aboutBlank.toString() << QStringLiteral("qrc:/resources/test1.html"));

    m_page->setUrl(QUrl());
    expectedLoadFinishedCount++;
    QTRY_COMPARE(spy.size(), expectedLoadFinishedCount);
    QCOMPARE(m_page->url(), aboutBlank);
    QCOMPARE(m_page->requestedUrl(), QUrl());
    // Chromium stores navigation entry for every successful loads. The load of the empty page is committed and stored as about:blank.
    QCOMPARE(collectHistoryUrls(m_page->history()), QStringList()
                                                        << aboutBlank.toString()
                                                        << QStringLiteral("qrc:/resources/test1.html")
                                                        << aboutBlank.toString());

    url = QUrl("qrc:/resources/test1.html");
    m_page->setUrl(url);
    expectedLoadFinishedCount++;
    QTRY_COMPARE(spy.size(), expectedLoadFinishedCount);
    QCOMPARE(m_page->url(), url);
    QCOMPARE(m_page->requestedUrl(), url);
    // The history count DOES change since the about:blank is in the list.
    QCOMPARE(collectHistoryUrls(m_page->history()), QStringList()
                                                        << aboutBlank.toString()
                                                        << QStringLiteral("qrc:/resources/test1.html")
                                                        << aboutBlank.toString()
                                                        << QStringLiteral("qrc:/resources/test1.html"));

    url = QUrl("qrc:/resources/test2.html");
    m_page->setUrl(url);
    expectedLoadFinishedCount++;
    QTRY_COMPARE(spy.size(), expectedLoadFinishedCount);
    QCOMPARE(m_page->url(), url);
    QCOMPARE(m_page->requestedUrl(), url);
    QCOMPARE(collectHistoryUrls(m_page->history()), QStringList()
                                                        << aboutBlank.toString()
                                                        << QStringLiteral("qrc:/resources/test1.html")
                                                        << aboutBlank.toString()
                                                        << QStringLiteral("qrc:/resources/test1.html")
                                                        << QStringLiteral("qrc:/resources/test2.html"));
}

void tst_QWebEnginePage::setUrlUsingStateObject()
{
    QUrl url;
    QSignalSpy urlChangedSpy(m_page, SIGNAL(urlChanged(QUrl)));
    QSignalSpy loadFinishedSpy(m_page, SIGNAL(loadFinished(bool)));
    int expectedUrlChangeCount = 0;

    QCOMPARE(m_page->history()->count(), 0);

    url = QUrl("qrc:/resources/test1.html");
    m_page->setUrl(url);
    expectedUrlChangeCount++;
    QTRY_COMPARE(urlChangedSpy.size(), expectedUrlChangeCount);
    QCOMPARE(m_page->url(), url);
    QTRY_COMPARE(loadFinishedSpy.size(), 1);
    QCOMPARE(m_page->url(), url);
    QCOMPARE(m_page->history()->count(), 1);

    evaluateJavaScriptSync(m_page, "window.history.pushState(null, 'push', 'navigate/to/here')");
    expectedUrlChangeCount++;
    QTRY_COMPARE(urlChangedSpy.size(), expectedUrlChangeCount);
    QCOMPARE(m_page->url(), QUrl("qrc:/resources/navigate/to/here"));
    QCOMPARE(m_page->history()->count(), 2);
    QVERIFY(m_page->history()->canGoBack());

    evaluateJavaScriptSync(m_page, "window.history.replaceState(null, 'replace', 'another/location')");
    expectedUrlChangeCount++;
    QTRY_COMPARE(urlChangedSpy.size(), expectedUrlChangeCount);
    QCOMPARE(m_page->url(), QUrl("qrc:/resources/navigate/to/another/location"));
    QCOMPARE(m_page->history()->count(), 2);
    QVERIFY(!m_page->history()->canGoForward());
    QVERIFY(m_page->history()->canGoBack());

    evaluateJavaScriptSync(m_page, "window.history.back()");
    expectedUrlChangeCount++;
    QTRY_COMPARE(urlChangedSpy.size(), expectedUrlChangeCount);
    QCOMPARE(m_page->url(), QUrl("qrc:/resources/test1.html"));
    QVERIFY(m_page->history()->canGoForward());
    QVERIFY(!m_page->history()->canGoBack());
}

static inline QUrl extractBaseUrl(const QUrl& url)
{
    return url.resolved(QUrl());
}

void tst_QWebEnginePage::setUrlThenLoads_data()
{
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<QUrl>("baseUrl");

    QTest::newRow("resource file") << QUrl("qrc:/resources/test1.html") << extractBaseUrl(QUrl("qrc:/resources/test1.html"));
    QTest::newRow("base specified in HTML") << QUrl("data:text/html,<head><base href=\"http://different.base/\"></head>") << QUrl("http://different.base/");
}

void tst_QWebEnginePage::setUrlThenLoads()
{
    QFETCH(QUrl, url);
    QFETCH(QUrl, baseUrl);
    QSignalSpy urlChangedSpy(m_page, SIGNAL(urlChanged(QUrl)));
    QSignalSpy startedSpy(m_page, SIGNAL(loadStarted()));
    QSignalSpy finishedSpy(m_page, SIGNAL(loadFinished(bool)));

    m_page->setUrl(url);
    QTRY_COMPARE(startedSpy.size(), 1);
    QTRY_COMPARE(urlChangedSpy.size(), 1);
    QTRY_COMPARE(finishedSpy.size(), 1);
    QVERIFY(finishedSpy.at(0).first().toBool());
    QCOMPARE(m_page->url(), url);
    QCOMPARE(m_page->requestedUrl(), url);
    QCOMPARE(baseUrlSync(m_page), baseUrl);

    const QUrl urlToLoad1("qrc:/resources/test2.html");
    const QUrl urlToLoad2("qrc:/resources/test1.html");

    m_page->load(urlToLoad1);
    QTRY_COMPARE(m_page->url(), urlToLoad1);
    QTRY_COMPARE(m_page->requestedUrl(), urlToLoad1);
    // baseUrlSync spins an event loop and this sometimes return the next result.
    // QCOMPARE(baseUrlSync(m_page), baseUrl);
    QTRY_COMPARE(startedSpy.size(), 2);

    // After first URL changed.
    QTRY_COMPARE(urlChangedSpy.size(), 2);
    QTRY_COMPARE(finishedSpy.size(), 2);
    QVERIFY(finishedSpy.at(1).first().toBool());
    QCOMPARE(m_page->url(), urlToLoad1);
    QCOMPARE(m_page->requestedUrl(), urlToLoad1);
    QCOMPARE(baseUrlSync(m_page), extractBaseUrl(urlToLoad1));

    m_page->load(urlToLoad2);
    QCOMPARE(m_page->url(), urlToLoad1);
    QCOMPARE(m_page->requestedUrl(), urlToLoad2);
    QCOMPARE(baseUrlSync(m_page), extractBaseUrl(urlToLoad1));
    QTRY_COMPARE(startedSpy.size(), 3);

    // After second URL changed.
    QTRY_COMPARE(urlChangedSpy.size(), 3);
    QTRY_COMPARE(finishedSpy.size(), 3);
    QVERIFY(finishedSpy.at(2).first().toBool());
    QCOMPARE(m_page->url(), urlToLoad2);
    QCOMPARE(m_page->requestedUrl(), urlToLoad2);
    QCOMPARE(baseUrlSync(m_page), extractBaseUrl(urlToLoad2));
}

class URLSetter : public QObject {
    Q_OBJECT

public:
    enum Signal {
        LoadStarted,
        LoadFinished,
    };

    enum Type {
        UseLoad,
        UseSetUrl
    };

    URLSetter(QWebEnginePage*, Signal, Type, const QUrl&);

public Q_SLOTS:
    void execute();

Q_SIGNALS:
    void finished();

private:
    QWebEnginePage* m_page;
    QUrl m_url;
    Type m_type;
};

Q_DECLARE_METATYPE(URLSetter::Signal)
Q_DECLARE_METATYPE(URLSetter::Type)

URLSetter::URLSetter(QWebEnginePage* page, Signal signal, URLSetter::Type type, const QUrl& url)
    : m_page(page), m_url(url), m_type(type)
{
    if (signal == LoadStarted)
        connect(m_page, SIGNAL(loadStarted()), SLOT(execute()));
    else if (signal == LoadFinished)
        connect(m_page, SIGNAL(loadFinished(bool)), SLOT(execute()));
}

void URLSetter::execute()
{
    // We track only the first emission.
    m_page->disconnect(this);
    connect(m_page, SIGNAL(loadFinished(bool)), SIGNAL(finished()));
    if (m_type == URLSetter::UseLoad)
        m_page->load(m_url);
    else
        m_page->setUrl(m_url);
}

void tst_QWebEnginePage::loadInSignalHandlers_data()
{
    QTest::addColumn<URLSetter::Type>("type");
    QTest::addColumn<URLSetter::Signal>("signal");
    QTest::addColumn<QUrl>("url");

    const QUrl validUrl("qrc:/resources/test2.html");
    const QUrl invalidUrl("qrc:/invalid");

    QTest::newRow("call load() in loadStarted() after valid url") << URLSetter::UseLoad << URLSetter::LoadStarted << validUrl;
    QTest::newRow("call load() in loadStarted() after invalid url") << URLSetter::UseLoad << URLSetter::LoadStarted << invalidUrl;
    QTest::newRow("call load() in loadFinished() after valid url") << URLSetter::UseLoad << URLSetter::LoadFinished << validUrl;
    QTest::newRow("call load() in loadFinished() after invalid url") << URLSetter::UseLoad << URLSetter::LoadFinished << invalidUrl;

    QTest::newRow("call setUrl() in loadStarted() after valid url") << URLSetter::UseSetUrl << URLSetter::LoadStarted << validUrl;
    QTest::newRow("call setUrl() in loadStarted() after invalid url") << URLSetter::UseSetUrl << URLSetter::LoadStarted << invalidUrl;
    QTest::newRow("call setUrl() in loadFinished() after valid url") << URLSetter::UseSetUrl << URLSetter::LoadFinished << validUrl;
    QTest::newRow("call setUrl() in loadFinished() after invalid url") << URLSetter::UseSetUrl << URLSetter::LoadFinished << invalidUrl;
}

void tst_QWebEnginePage::loadInSignalHandlers()
{
    QFETCH(URLSetter::Type, type);
    QFETCH(URLSetter::Signal, signal);
    QFETCH(QUrl, url);

    const QUrl urlForSetter("qrc:/resources/test1.html");
    URLSetter setter(m_page, signal, type, urlForSetter);
    QSignalSpy spy(&setter, &URLSetter::finished);
    m_page->load(url);
    QTRY_VERIFY_WITH_TIMEOUT(spy.size() >= 1, 20000);
    QCOMPARE(m_page->url(), urlForSetter);
}

void tst_QWebEnginePage::loadFromQrc()
{
    QWebEnginePage page;
    QSignalSpy spy(&page, &QWebEnginePage::loadFinished);

    // Standard case.
    page.load(QStringLiteral("qrc:///resources/foo.txt"));
    QTRY_COMPARE(spy.size(), 1);
    QCOMPARE(spy.takeFirst().value(0).toBool(), true);
    QCOMPARE(toPlainTextSync(&page), QStringLiteral("foo\n"));

    // Query and fragment parts are ignored.
    page.load(QStringLiteral("qrc:///resources/bar.txt?foo=1#bar"));
    QTRY_COMPARE(spy.size(), 1);
    QCOMPARE(spy.takeFirst().value(0).toBool(), true);
    QCOMPARE(toPlainTextSync(&page), QStringLiteral("bar\n"));

    // Literal spaces are OK.
    page.load(QStringLiteral("qrc:///resources/path with spaces.txt"));
    QTRY_COMPARE(spy.size(), 1);
    QCOMPARE(spy.takeFirst().value(0).toBool(), true);
    QCOMPARE(toPlainTextSync(&page), QStringLiteral("contents with spaces\n"));

    // Escaped spaces are OK too.
    page.load(QStringLiteral("qrc:///resources/path%20with%20spaces.txt"));
    QTRY_COMPARE(spy.size(), 1);
    QCOMPARE(spy.takeFirst().value(0).toBool(), true);
    QCOMPARE(toPlainTextSync(&page), QStringLiteral("contents with spaces\n"));

    // Resource not found, loading fails.
    page.load(QStringLiteral("qrc:///nope"));
    QTRY_COMPARE_WITH_TIMEOUT(spy.size(), 1, 10000);
    QCOMPARE(spy.takeFirst().value(0).toBool(), false);
}

#if QT_CONFIG(webengine_webchannel)
void tst_QWebEnginePage::restoreHistory()
{
    QWebChannel channel;
    QWebEnginePage page;
    page.setWebChannel(&channel);

    QWebEngineScript script;
    script.setName(QStringLiteral("script"));
    page.scripts().insert(script);

    QSignalSpy spy(&page, SIGNAL(loadFinished(bool)));
    page.load(QUrl(QStringLiteral("qrc:/resources/test1.html")));
    QTRY_COMPARE(spy.size(), 1);

    QCOMPARE(page.webChannel(), &channel);
    QVERIFY(page.scripts().contains(script));

    QByteArray data;
    QDataStream out(&data, QIODevice::ReadWrite);
    out << *page.history();
    QDataStream in(&data, QIODevice::ReadOnly);
    in >> *page.history();
    QTRY_COMPARE(spy.size(), 2);

    QCOMPARE(page.webChannel(), &channel);
    QVERIFY(page.scripts().contains(script));
}
#endif

void tst_QWebEnginePage::toPlainTextLoadFinishedRace_data()
{
    QTest::addColumn<bool>("enableErrorPage");
    QTest::newRow("disableErrorPage") << false;
    QTest::newRow("enableErrorPage") << true;
}

void tst_QWebEnginePage::toPlainTextLoadFinishedRace()
{
    QFETCH(bool, enableErrorPage);

    QScopedPointer<QWebEnginePage> page(new QWebEnginePage);
    page->settings()->setAttribute(QWebEngineSettings::ErrorPageEnabled, enableErrorPage);
    QSignalSpy spy(page.data(), SIGNAL(loadFinished(bool)));

    page->load(QUrl("data:text/plain,foobarbaz"));
    QTRY_VERIFY(spy.size() == 1);
    QCOMPARE(toPlainTextSync(page.data()), QString("foobarbaz"));

    page->load(QUrl("http://fail.invalid/"));
    QTRY_COMPARE_WITH_TIMEOUT(spy.size(), 2, 20000);
    QString s = toPlainTextSync(page.data());
    QVERIFY(s.contains("foobarbaz") == !enableErrorPage);

    page->load(QUrl("data:text/plain,lalala"));
    QTRY_COMPARE(spy.size(), 3);
    QTRY_COMPARE(toPlainTextSync(page.data()), QString("lalala"));
    page.reset();
    QCOMPARE(spy.size(), 3);
}

void tst_QWebEnginePage::setZoomFactor()
{
    TestBasePage page, page2;

    QCOMPARE(page.zoomFactor(), 1.0);
    page.setZoomFactor(2.5);
    QCOMPARE(page.zoomFactor(), 2.5);

    const QUrl url1("qrc:/resources/test1.html"), url2(QUrl("qrc:/resources/test2.html"));

    page.load(url1);
    QTRY_COMPARE(page.loadSpy.size(), 1);
    QVERIFY(page.loadSpy.at(0).first().toBool());
    QCOMPARE(page.zoomFactor(), 2.5);

    page.setZoomFactor(5.5); // max accepted zoom: kMaximumPageZoomFactor = 5.0
    QCOMPARE(page.zoomFactor(), 2.5);

    page.setZoomFactor(0.1); // min accepted zoom: kMinimumPageZoomFactor = 0.25
    QCOMPARE(page.zoomFactor(), 2.5);

    // try loading different url and check new values after load
    page.loadSpy.clear();
    for (auto &&p : {
            qMakePair(&page, 2.5), // navigating away to different url should keep zoom
            qMakePair(&page2, 1.0), // same url navigation in diffent page shouldn't be affected
        }) {
        auto &&page = *p.first; auto zoomFactor = p.second;
        page.load(url2);
        QTRY_COMPARE(page.loadSpy.size(), 1);
        QVERIFY(page.loadSpy.last().first().toBool());
        QCOMPARE(page.zoomFactor(), zoomFactor);
    }

    // should have no influence on first page
    page2.setZoomFactor(3.5);
    for (auto &&p : { qMakePair(&page, 2.5), qMakePair(&page2, 3.5), })
        QCOMPARE(p.first->zoomFactor(), p.second);
}

void tst_QWebEnginePage::mouseButtonTranslation()
{
    QWebEngineView view;

    QSignalSpy spy(&view, SIGNAL(loadFinished(bool)));
    view.setHtml(QStringLiteral(
                      "<html><head><script>\
                           var lastEvent = { 'button' : -1 }; \
                           function saveLastEvent(event) { console.log(event); lastEvent = event; }; \
                      </script></head>\
                      <body>\
                      <div style=\"height:600px;\" onmousedown=\"saveLastEvent(event)\">\
                      </div>\
                      </body></html>"));
    view.resize(640, 480);
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));
    QTRY_VERIFY(spy.size() == 1);

    QVERIFY(view.focusProxy() != nullptr);

    const QPoint mousePos = view.rect().center();
    QMouseEvent evpres(QEvent::MouseButtonPress, mousePos, view.mapToGlobal(mousePos),
                       Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QGuiApplication::sendEvent(view.focusProxy(), &evpres);

    QTRY_COMPARE(evaluateJavaScriptSync(view.page(), "lastEvent.button").toInt(), 0);
    QCOMPARE(evaluateJavaScriptSync(view.page(), "lastEvent.buttons").toInt(), 1);

    QMouseEvent evpres2(QEvent::MouseButtonPress, mousePos, view.mapToGlobal(mousePos),
                        Qt::RightButton, Qt::LeftButton | Qt::RightButton, Qt::NoModifier);
    QGuiApplication::sendEvent(view.focusProxy(), &evpres2);

    QTRY_COMPARE(evaluateJavaScriptSync(view.page(), "lastEvent.button").toInt(), 2);
    QCOMPARE(evaluateJavaScriptSync(view.page(), "lastEvent.buttons").toInt(), 3);
}

void tst_QWebEnginePage::mouseMovementProperties()
{
    QWebEngineView view;
    ConsolePage page;
    view.setPage(&page);
    view.resize(640, 480);
    QTest::mouseMove(&view, QPoint(10, 10));
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    QSignalSpy loadFinishedSpy(&page, SIGNAL(loadFinished(bool)));
    page.setHtml(QStringLiteral(
                        "<html><head><script>\
                            function onMouseMove(event) { console.log(event.movementX + \", \" + event.movementY); }; \
                        </script></head>\
                        <body>\
                            <div style=\"height:600px;\" onmousemove=\"onMouseMove(event)\">\
                        </div>\
                        </body></html>"));
    loadFinishedSpy.wait();

    QTest::mouseMove(&view, QPoint(20, 20));
    QTRY_COMPARE(page.messages.size(), 1);

    QTest::mouseMove(&view, QPoint(30, 30));
    QTRY_COMPARE(page.messages.size(), 2);
    QTRY_COMPARE(page.messages[1], QString("10, 10"));

    QTest::mouseMove(&view, QPoint(20, 20));
    QTRY_COMPARE(page.messages.size(), 3);
    QTRY_COMPARE(page.messages[2], QString("-10, -10"));
}

void tst_QWebEnginePage::viewSource()
{
    TestPage page;
    QSignalSpy loadFinishedSpy(&page, SIGNAL(loadFinished(bool)));
    QSignalSpy windowCreatedSpy(&page, SIGNAL(windowCreated()));
    const QUrl url("qrc:/resources/test1.html");

    page.load(url);
    QTRY_COMPARE(loadFinishedSpy.size(), 1);
    QCOMPARE(page.title(), QStringLiteral("Test page 1"));
    QVERIFY(page.action(QWebEnginePage::ViewSource)->isEnabled());

    page.triggerAction(QWebEnginePage::ViewSource);
    QTRY_COMPARE(windowCreatedSpy.size(), 1);
    QCOMPARE(page.createdWindows.size(), 1);

    QTRY_COMPARE(page.createdWindows[0]->url().toString(), QStringLiteral("view-source:%1").arg(url.toString()));
    // The requested URL should not be about:blank if the qrc scheme is supported
    QTRY_COMPARE(page.createdWindows[0]->requestedUrl(), url);
    QTRY_COMPARE(page.createdWindows[0]->title(), QStringLiteral("view-source:%1").arg(url.toString()));
    QVERIFY(!page.createdWindows[0]->action(QWebEnginePage::ViewSource)->isEnabled());
}

void tst_QWebEnginePage::viewSourceURL_data()
{
    QTest::addColumn<QUrl>("userInputUrl");
    QTest::addColumn<bool>("loadSucceed");
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<QUrl>("requestedUrl");
    QTest::addColumn<QString>("title");

    QTest::newRow("view-source:") << QUrl("view-source:") << true << QUrl("view-source:") << QUrl("about:blank") << QString("view-source:");
    QTest::newRow("view-source:about:blank") << QUrl("view-source:about:blank") << true << QUrl("view-source:about:blank") << QUrl("about:blank") << QString("view-source:about:blank");

    QString localFilePath =
            QString("%1/resources/test1.html").arg(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath());
    QUrl testLocalUrl = QUrl(QString("view-source:%1").arg(QUrl::fromLocalFile(localFilePath).toString()));
    QUrl testLocalUrlWithoutScheme = QUrl(QString("view-source:%1").arg(localFilePath));
    QTest::newRow(testLocalUrl.toString().toStdString().c_str()) << testLocalUrl << true << testLocalUrl << QUrl::fromLocalFile(localFilePath) << QString("test1.html");
    QTest::newRow(testLocalUrlWithoutScheme.toString().toStdString().c_str()) << testLocalUrlWithoutScheme << true << testLocalUrl << QUrl::fromLocalFile(localFilePath) << QString("test1.html");

    QString resourcePath = QLatin1String("qrc:/resources/test1.html");
    QUrl testResourceUrl = QUrl(QString("view-source:%1").arg(resourcePath));
    QTest::newRow(testResourceUrl.toString().toStdString().c_str()) << testResourceUrl << true << testResourceUrl << QUrl(resourcePath) << testResourceUrl.toString();

    QTest::newRow("view-source:http://non.existent") << QUrl("view-source:non.existent") << false << QUrl("view-source:http://non.existent/") << QUrl("http://non.existent/") << QString("non.existent");
    QTest::newRow("view-source:non.existent") << QUrl("view-source:non.existent") << false << QUrl("view-source:http://non.existent/") << QUrl("http://non.existent/") << QString("non.existent");
}

void tst_QWebEnginePage::viewSourceURL()
{
    if (!QDir(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()).exists())
        W_QSKIP(QString("This test requires access to resources found in '%1'")
                        .arg(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath())
                        .toLatin1()
                        .constData(),
                SkipAll);

    QFETCH(QUrl, userInputUrl);
    QFETCH(bool, loadSucceed);
    QFETCH(QUrl, url);
    QFETCH(QUrl, requestedUrl);
    QFETCH(QString, title);

    QWebEnginePage page;
    QSignalSpy loadFinishedSpy(&page, SIGNAL(loadFinished(bool)));

    page.load(userInputUrl);
    QTRY_COMPARE_WITH_TIMEOUT(loadFinishedSpy.size(), 1, 12000);
    QList<QVariant> arguments = loadFinishedSpy.takeFirst();

    QCOMPARE(arguments.at(0).toBool(), loadSucceed);
    QCOMPARE(page.url(), url);
    QCOMPARE(page.requestedUrl(), requestedUrl);
    QCOMPARE(page.title(), title);
    QVERIFY(!page.action(QWebEnginePage::ViewSource)->isEnabled());
}

void tst_QWebEnginePage::viewSourceCredentials()
{
    TestPage page;
    QSignalSpy loadFinishedSpy(&page, SIGNAL(loadFinished(bool)));
    QSignalSpy windowCreatedSpy(&page, SIGNAL(windowCreated()));
    QUrl url("http://user:passwd@httpbin.org/basic-auth/user/passwd");

    // Test explicit view-source URL with credentials
    page.load(QUrl(QString("view-source:" + url.toString())));
    if (!loadFinishedSpy.wait(10000) || !loadFinishedSpy.at(0).at(0).toBool())
        QSKIP("Couldn't load page from network, skipping test.");

    QCOMPARE(page.url().toString(), QString("view-source:" + url.toDisplayString(QUrl::RemoveUserInfo)));
    QCOMPARE(page.requestedUrl(), url);
    QCOMPARE(page.title(), QString("view-source:" + url.toDisplayString(QUrl::RemoveScheme | QUrl::RemoveUserInfo).remove(0, 2)));
    loadFinishedSpy.clear();
    windowCreatedSpy.clear();

    // Test ViewSource web action on URL with credentials
    page.load(url);
    if (!loadFinishedSpy.wait(10000) || !loadFinishedSpy.at(0).at(0).toBool())
        QSKIP("Couldn't load page from network, skipping test.");
    QVERIFY(page.action(QWebEnginePage::ViewSource)->isEnabled());

    page.triggerAction(QWebEnginePage::ViewSource);
    QTRY_COMPARE(windowCreatedSpy.size(), 1);
    QCOMPARE(page.createdWindows.size(), 1);

    QTRY_COMPARE(page.createdWindows[0]->url().toString(), QString("view-source:" + url.toDisplayString(QUrl::RemoveUserInfo)));
    QTRY_COMPARE(page.createdWindows[0]->requestedUrl(), url);
    QTRY_COMPARE(page.createdWindows[0]->title(), QString("view-source:" + url.toDisplayString(QUrl::RemoveScheme | QUrl::RemoveUserInfo).remove(0, 2)));
}

Q_DECLARE_METATYPE(QNetworkProxy::ProxyType);

void tst_QWebEnginePage::proxyConfigWithUnexpectedHostPortPair()
{
    // Chromium expects a proxy of type NoProxy to not have a host or port set.

    QNetworkProxy proxy;
    proxy.setType(QNetworkProxy::NoProxy);
    proxy.setHostName(QStringLiteral("127.0.0.1"));
    proxy.setPort(244);
    QNetworkProxy::setApplicationProxy(proxy);

    QSignalSpy loadFinishedSpy(m_page, SIGNAL(loadFinished(bool)));
    m_page->load(QStringLiteral("http://127.0.0.1:245/"));
    QTRY_COMPARE(loadFinishedSpy.size(), 1);
}

void tst_QWebEnginePage::registerProtocolHandler_data()
{
    QTest::addColumn<bool>("permission");
    QTest::newRow("accept") << true;
    QTest::newRow("reject") << false;
}

void tst_QWebEnginePage::registerProtocolHandler()
{
    QFETCH(bool, permission);

    HttpServer server;
    int mailRequestCount = 0;
    connect(&server, &HttpServer::newRequest, [&](HttpReqRep *rr) {
        if (rr->requestMethod() == "GET" && rr->requestPath() == "/") {
            rr->setResponseBody(QByteArrayLiteral("<html><body><a id=\"link\" href=\"mailto:foo@bar.com\">some text here</a></body></html>"));
            rr->sendResponse();
        } else if (rr->requestMethod() == "GET" && rr->requestPath() == "/mail?uri=mailto%3Afoo%40bar.com") {
            mailRequestCount++;
            rr->sendResponse();
        }
    });
    QVERIFY(server.start());

    QWebEngineProfile profile(QStringLiteral("registerProtocolHandler%1").arg(QTest::currentDataTag()));
    QWebEnginePage page(&profile, nullptr);
    QSignalSpy loadSpy(&page, &QWebEnginePage::loadFinished);
    QSignalSpy permissionSpy(&page, &QWebEnginePage::registerProtocolHandlerRequested);

    page.setUrl(server.url("/"));
    QTRY_COMPARE(loadSpy.size(), 1);
    QCOMPARE(loadSpy.takeFirst().value(0).toBool(), true);

    QString callFormat = QStringLiteral("window.navigator.registerProtocolHandler(\"%1\", \"%2\", \"%3\")");
    QString scheme = QStringLiteral("mailto");
    QString url = server.url("/mail").toString() + QStringLiteral("?uri=%s");
    QString title;
    QString call = callFormat.arg(scheme).arg(url).arg(title);
    page.runJavaScript(call);

    QTRY_COMPARE(permissionSpy.size(), 1);
    auto request = permissionSpy.takeFirst().value(0).value<QWebEngineRegisterProtocolHandlerRequest>();
    QCOMPARE(request.origin(), QUrl(url));
    QCOMPARE(request.scheme(), scheme);
    if (permission)
        request.accept();
    else
        request.reject();

    page.runJavaScript(QStringLiteral("document.getElementById(\"link\").click()"));

    QTRY_COMPARE(loadSpy.size(), 1);
    QCOMPARE(loadSpy.takeFirst().value(0).toBool(), permission);
    QCOMPARE(mailRequestCount, permission ? 1 : 0);
    QVERIFY(server.stop());
}

void tst_QWebEnginePage::dataURLFragment()
{
    m_view->resize(800, 600);
    m_view->show();
    QSignalSpy loadFinishedSpy(m_page, SIGNAL(loadFinished(bool)));
    QSignalSpy urlChangedSpy(m_page, SIGNAL(urlChanged(QUrl)));

    m_page->setHtml("<html><body>"
                    "<a id='link' href='#anchor'>anchor</a>"
                    "</body></html>", QUrl("http://test.qt.io/mytest.html"));
    QTRY_COMPARE(loadFinishedSpy.size(), 1);

    QTest::mouseClick(m_view->focusProxy(), Qt::LeftButton, {}, elementCenter(m_page, "link"));
    QVERIFY(urlChangedSpy.wait());
    QCOMPARE(m_page->url(), QUrl("http://test.qt.io/mytest.html#anchor"));
}

void tst_QWebEnginePage::devTools()
{
    QWebEngineProfile profile;
    QWebEnginePage inspectedPage1(&profile);
    QWebEnginePage inspectedPage2(&profile);
    QWebEnginePage devToolsPage(&profile);
    QSignalSpy spy(&devToolsPage, &QWebEnginePage::loadFinished);

    inspectedPage1.setDevToolsPage(&devToolsPage);

    QCOMPARE(inspectedPage1.devToolsPage(), &devToolsPage);
    QCOMPARE(inspectedPage1.inspectedPage(), nullptr);
    QCOMPARE(inspectedPage2.devToolsPage(), nullptr);
    QCOMPARE(inspectedPage2.inspectedPage(), nullptr);
    QCOMPARE(devToolsPage.devToolsPage(), nullptr);
    QCOMPARE(devToolsPage.inspectedPage(), &inspectedPage1);

    QTRY_COMPARE_WITH_TIMEOUT(spy.size(), 1, 90000);
    QVERIFY(spy.takeFirst().value(0).toBool());

    devToolsPage.setInspectedPage(&inspectedPage2);

    QCOMPARE(inspectedPage1.devToolsPage(), nullptr);
    QCOMPARE(inspectedPage1.inspectedPage(), nullptr);
    QCOMPARE(inspectedPage2.devToolsPage(), &devToolsPage);
    QCOMPARE(inspectedPage2.inspectedPage(), nullptr);
    QCOMPARE(devToolsPage.devToolsPage(), nullptr);
    QCOMPARE(devToolsPage.inspectedPage(), &inspectedPage2);

    QTRY_COMPARE_WITH_TIMEOUT(spy.size(), 1, 90000);
    QVERIFY(spy.takeFirst().value(0).toBool());

    devToolsPage.setInspectedPage(nullptr);

    QCOMPARE(inspectedPage1.devToolsPage(), nullptr);
    QCOMPARE(inspectedPage1.inspectedPage(), nullptr);
    QCOMPARE(inspectedPage2.devToolsPage(), nullptr);
    QCOMPARE(inspectedPage2.inspectedPage(), nullptr);
    QCOMPARE(devToolsPage.devToolsPage(), nullptr);
    QCOMPARE(devToolsPage.inspectedPage(), nullptr);

    QVERIFY(!inspectedPage1.devToolsId().isEmpty());
}

void tst_QWebEnginePage::openLinkInDifferentProfile()
{
    QWebEnginePage *targetPage = nullptr;
    QWebEngineProfile profile1, profile2;
    profile1.installUrlSchemeHandler("echo", new EchoingUrlSchemeHandler(&profile1));
    profile2.installUrlSchemeHandler("echo", new EchoingUrlSchemeHandler(&profile2));
    QWebEnginePage page1(&profile1), page2(&profile2);
    connect(&page1, &QWebEnginePage::newWindowRequested, [&](QWebEngineNewWindowRequest &request) {
        request.openIn(targetPage);
    });
    QWebEngineView view;
    view.resize(500, 500);
    view.setPage(&page1);
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));
    QSignalSpy spy1(&page1, &QWebEnginePage::loadFinished), spy2(&page2, &QWebEnginePage::loadFinished);
    page1.setHtml("<html><body>"
                  "<a id='link' href='hello'>link</a>"
                  "</body></html>", QUrl("echo:/"));
    QTRY_COMPARE(spy1.size(), 1);
    QVERIFY(spy1.takeFirst().value(0).toBool());
    targetPage = &page2;
    QTest::mouseClick(view.focusProxy(), Qt::MiddleButton, {}, elementCenter(&page1, "link"));
    QTRY_COMPARE(spy2.size(), 1);
    QVERIFY(spy2.takeFirst().value(0).toBool());
}

// What does createWindow do?
enum class OpenLinkInNewPageDecision {
    // Returns nullptr,
    ReturnNull,
    // Returns this,
    ReturnSelf,
    // Returns page != this
    ReturnOther,
};

// What causes createWindow to be called?
enum class OpenLinkInNewPageCause {
    // User clicks on a link with target=_blank.
    TargetBlank,
    // User clicks with MiddleButton.
    MiddleClick,
};

// What happens after createWindow?
enum class OpenLinkInNewPageEffect {
    // The navigation request disappears into the ether.
    Blocked,
    // The navigation request becomes a navigation in the original page.
    LoadInSelf,
    // The navigation request becomes a navigation in a different page.
    LoadInOther,
};

Q_DECLARE_METATYPE(OpenLinkInNewPageCause)
Q_DECLARE_METATYPE(OpenLinkInNewPageDecision)
Q_DECLARE_METATYPE(OpenLinkInNewPageEffect)

void tst_QWebEnginePage::openLinkInNewPage_data()
{
    using Decision = OpenLinkInNewPageDecision;
    using Cause = OpenLinkInNewPageCause;
    using Effect = OpenLinkInNewPageEffect;

    QTest::addColumn<Decision>("decision");
    QTest::addColumn<Cause>("cause");
    QTest::addColumn<Effect>("effect");

    // Note that the meaning of returning nullptr from createWindow is not
    // consistent between the TargetBlank and MiddleClick scenarios.
    //
    // With TargetBlank, the open-in-new-page disposition comes from the HTML
    // target attribute; something the user is probably not aware of. Returning
    // nullptr is interpreted as a decision by the app to block an unwanted
    // popup.
    //
    // With MiddleClick, the open-in-new-page disposition comes from the user's
    // explicit intent. Returning nullptr is then interpreted as a failure by
    // the app to fulfill this intent, which we try to compensate by ignoring
    // the disposition and performing the navigation request normally.

    QTest::newRow("BlockPopup")     << Decision::ReturnNull  << Cause::TargetBlank << Effect::Blocked;
    QTest::newRow("IgnoreIntent")   << Decision::ReturnNull  << Cause::MiddleClick << Effect::Blocked;
    QTest::newRow("OverridePopup")  << Decision::ReturnSelf  << Cause::TargetBlank << Effect::LoadInSelf;
    QTest::newRow("OverrideIntent") << Decision::ReturnSelf  << Cause::MiddleClick << Effect::LoadInSelf;
    QTest::newRow("AcceptPopup")    << Decision::ReturnOther << Cause::TargetBlank << Effect::LoadInOther;
    QTest::newRow("AcceptIntent")   << Decision::ReturnOther << Cause::MiddleClick << Effect::LoadInOther;
}

void tst_QWebEnginePage::openLinkInNewPage()
{
    using Decision = OpenLinkInNewPageDecision;
    using Cause = OpenLinkInNewPageCause;
    using Effect = OpenLinkInNewPageEffect;

    class Page : public QWebEnginePage
    {
    public:
        Page *targetPage = nullptr;
        QSignalSpy spy{this, &QWebEnginePage::loadFinished};
        Page(QWebEngineProfile *profile) : QWebEnginePage(profile) {}
    private:
        QWebEnginePage *createWindow(WebWindowType) override { return targetPage; }
    };

    class View : public QWebEngineView
    {
    public:
        View(Page *page)
        {
            resize(500, 500);
            setPage(page);
        }
    };

    QFETCH(Decision, decision);
    QFETCH(Cause, cause);
    QFETCH(Effect, effect);

    QWebEngineProfile profile;
    profile.installUrlSchemeHandler("echo", new EchoingUrlSchemeHandler(&profile));
    Page page1(&profile);
    Page page2(&profile);
    View view1(&page1);
    View view2(&page2);

    view1.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view1));

    page1.setHtml("<html><body>"
                  "<a id='link' href='hello' target='_blank'>link</a>"
                  "</body></html>", QUrl("echo:/"));
    QTRY_COMPARE(page1.spy.size(), 1);
    QVERIFY(page1.spy.takeFirst().value(0).toBool());

    switch (decision) {
    case Decision::ReturnNull:
        page1.targetPage = nullptr;
        break;
    case Decision::ReturnSelf:
        page1.targetPage = &page1;
        break;
    case Decision::ReturnOther:
        page1.targetPage = &page2;
        break;
    }

    Qt::MouseButton button = Qt::NoButton;
    switch (cause) {
    case Cause::TargetBlank:
        button = Qt::LeftButton;
        break;
    case Cause::MiddleClick:
        button = Qt::MiddleButton;
        break;
    }
    QTest::mouseClick(view1.focusProxy(), button, {}, elementCenter(&page1, "link"));

    switch (effect) {
    case Effect::Blocked:
        // Test nothing new loaded
        QTest::qWait(500);
        QCOMPARE(page1.spy.size(), 0);
        QCOMPARE(page2.spy.size(), 0);
        break;
    case Effect::LoadInSelf:
        QTRY_COMPARE(page1.spy.size(), 1);
        QVERIFY(page1.spy.takeFirst().value(0).toBool());
        QCOMPARE(page2.spy.size(), 0);
        if (decision == Decision::ReturnSelf && cause == Cause::TargetBlank)
            // History was discarded due to AddNewContents
            QCOMPARE(page1.history()->count(), 1);
        else
            QCOMPARE(page1.history()->count(), 2);
        QCOMPARE(page2.history()->count(), 0);
        break;
    case Effect::LoadInOther:
        QTRY_COMPARE(page2.spy.size(), 1);
        QVERIFY(page2.spy.takeFirst().value(0).toBool());
        QCOMPARE(page1.spy.size(), 0);
        QCOMPARE(page1.history()->count(), 1);
        QCOMPARE(page2.history()->count(), 1);
        break;
    }
}

void tst_QWebEnginePage::triggerActionWithoutMenu()
{
    // Calling triggerAction should not crash even when for
    // context-menu-specific actions without a context menu.
    QWebEngineProfile profile;
    QWebEnginePage page(&profile);
    page.triggerAction(QWebEnginePage::DownloadLinkToDisk);
}

void tst_QWebEnginePage::dynamicFrame()
{
    QWebEnginePage page;
    page.settings()->setAttribute(QWebEngineSettings::ErrorPageEnabled, false);
    QSignalSpy spy(&page, &QWebEnginePage::loadFinished);
    page.load(QStringLiteral("qrc:/resources/dynamicFrame.html"));
    QTRY_COMPARE(spy.size(), 1);
    QCOMPARE(toPlainTextSync(&page).trimmed(), QStringLiteral("foo"));
}

struct NotificationPage : ConsolePage {
    Q_OBJECT
    const QWebEnginePage::PermissionPolicy policy;

public:
    NotificationPage(QWebEnginePage::PermissionPolicy ppolicy) : policy(ppolicy) {
        connect(this, &QWebEnginePage::loadFinished, [load = spyLoad.ref()] (bool result) mutable { load(result); });

        connect(this, &QWebEnginePage::featurePermissionRequested,
                [this] (const QUrl &origin, QWebEnginePage::Feature feature) {
            if (feature != QWebEnginePage::Notifications)
                return;
            if (spyRequest.wasCalled())
                QFAIL("request executed twise!");
            setFeaturePermission(origin, feature, policy);
            spyRequest.ref()(origin);
        });

        load(QStringLiteral("qrc:///shared/notification.html"));
    }

    CallbackSpy<bool> spyLoad;
    CallbackSpy<QUrl> spyRequest;

    QString getPermission() { return evaluateJavaScriptSync(this, "getPermission()").toString(); }
    void requestPermission() { runJavaScript("requestPermission()"); }
    void resetPermission() { runJavaScript("resetPermission()"); }
    void sendNotification(const QString &title, const QString &body) {
        runJavaScript("sendNotification('" + title + "', '" + body + "')");
    }
};

void tst_QWebEnginePage::notificationPermission_data()
{
    QTest::addColumn<bool>("setOnInit");
    QTest::addColumn<QWebEnginePage::PermissionPolicy>("policy");
    QTest::addColumn<QString>("permission");
    QTest::newRow("denyOnInit")  << true  << QWebEnginePage::PermissionDeniedByUser << "denied";
    QTest::newRow("deny")        << false << QWebEnginePage::PermissionDeniedByUser << "denied";
    QTest::newRow("grant")       << false << QWebEnginePage::PermissionGrantedByUser << "granted";
    QTest::newRow("grantOnInit") << true  << QWebEnginePage::PermissionGrantedByUser << "granted";
}

void tst_QWebEnginePage::notificationPermission()
{
    QFETCH(bool, setOnInit);
    QFETCH(QWebEnginePage::PermissionPolicy, policy);
    QFETCH(QString, permission);

    QWebEngineProfile otr;
    QWebEnginePage page(&otr, nullptr);

    QUrl baseUrl("https://www.example.com/somepage.html");

    bool permissionRequested = false, errorState = false;
    connect(&page, &QWebEnginePage::featurePermissionRequested, &page, [&] (const QUrl &o, QWebEnginePage::Feature f) {
        if (f != QWebEnginePage::Notifications)
            return;
        if (permissionRequested || o != baseUrl.url(QUrl::RemoveFilename)) {
            qWarning() << "Unexpected case. Can't proceed." << setOnInit << permissionRequested << o;
            errorState = true;
            return;
        }
        permissionRequested = true;
        page.setFeaturePermission(o, f, policy);
    });

    if (setOnInit)
        page.setFeaturePermission(baseUrl, QWebEnginePage::Notifications, policy);

    QSignalSpy spy(&page, &QWebEnginePage::loadFinished);
    page.setHtml(QString("<html><body>Test</body></html>"), baseUrl);
    QTRY_COMPARE(spy.size(), 1);

    QCOMPARE(evaluateJavaScriptSync(&page, QStringLiteral("Notification.permission")), setOnInit ? permission : QLatin1String("default"));

    if (!setOnInit) {
        page.setFeaturePermission(baseUrl, QWebEnginePage::Notifications, policy);
        QTRY_COMPARE(evaluateJavaScriptSync(&page, QStringLiteral("Notification.permission")), permission);
    }

    auto js = QStringLiteral("var permission; Notification.requestPermission().then(p => { permission = p })");
    evaluateJavaScriptSync(&page, js);
    QTRY_COMPARE(evaluateJavaScriptSync(&page, "permission").toString(), permission);
    // permission is not 'remembered' from api standpoint, hence is not suppressed on explicit call from JS
    QVERIFY(permissionRequested);
    QVERIFY(!errorState);
}

void tst_QWebEnginePage::sendNotification()
{
    NotificationPage page(QWebEnginePage::PermissionGrantedByUser);
    QVERIFY(page.spyLoad.waitForResult());

    page.resetPermission();
    page.requestPermission();
    auto origin = page.spyRequest.waitForResult();
    QVERIFY(page.spyRequest.wasCalled());
    QCOMPARE(page.getPermission(), "granted");

    std::unique_ptr<QWebEngineNotification> activeNotification;
    CallbackSpy<bool> presenter;
    page.profile()->setNotificationPresenter(
                [&] (std::unique_ptr<QWebEngineNotification> notification)
                {
                    activeNotification = std::move(notification);
                    presenter(true);
                });

    QString title("Title"), message("Message");
    page.sendNotification(title, message);

    presenter.waitForResult();
    QVERIFY(presenter.wasCalled());
    QVERIFY(activeNotification);
    QCOMPARE(activeNotification->title(), title);
    QCOMPARE(activeNotification->message(), message);
    QCOMPARE(activeNotification->origin(), origin);
    QCOMPARE(activeNotification->direction(), Qt::RightToLeft);
    QCOMPARE(activeNotification->language(), "de");
    QCOMPARE(activeNotification->tag(), "tst");

    activeNotification->show();
    QTRY_VERIFY2(page.messages.contains("onshow"), page.messages.join("\n").toLatin1().constData());
    activeNotification->click();
    QTRY_VERIFY2(page.messages.contains("onclick"), page.messages.join("\n").toLatin1().constData());
    activeNotification->close();
    QTRY_VERIFY2(page.messages.contains("onclose"), page.messages.join("\n").toLatin1().constData());
}

void tst_QWebEnginePage::contentsSize()
{
    m_view->resize(800, 600);
    m_view->show();

    QSignalSpy loadSpy(m_page, &QWebEnginePage::loadFinished);
    QSignalSpy contentsSizeChangedSpy(m_page, &QWebEnginePage::contentsSizeChanged);

    m_view->setHtml(QString("<html><body style=\"width: 1600px; height: 1200px;\"><p>hi</p></body></html>"));

    QTRY_COMPARE(loadSpy.size(), 1);
    QTRY_COMPARE(contentsSizeChangedSpy.size(), 1);

    // Verify the page's contents size is not limited by the view's size.
    QCOMPARE(m_page->contentsSize().width(), 1608);
    QCOMPARE(m_page->contentsSize().height(), 1216);

    // Verify resizing the view does not affect the contents size.
    m_view->resize(2400, 1800);
    QCOMPARE(m_page->contentsSize().width(), 1608);
    QCOMPARE(m_page->contentsSize().height(), 1216);

    // Verify resizing the view does not affect the contents size.
    m_view->resize(1600, 1200);
    QCOMPARE(m_page->contentsSize().width(), 1608);
    QCOMPARE(m_page->contentsSize().height(), 1216);
}

void tst_QWebEnginePage::setLifecycleState()
{
    qRegisterMetaType<QWebEnginePage::LifecycleState>("LifecycleState");

    QWebEngineProfile profile;
    QWebEnginePage page(&profile);
    QSignalSpy loadSpy(&page, &QWebEnginePage::loadFinished);
    QSignalSpy lifecycleSpy(&page, &QWebEnginePage::lifecycleStateChanged);
    QSignalSpy visibleSpy(&page, &QWebEnginePage::visibleChanged);

    page.load(QStringLiteral("qrc:/resources/lifecycle.html"));
    QTRY_COMPARE(loadSpy.size(), 1);
    QCOMPARE(loadSpy.takeFirst().value(0), QVariant(true));
    QCOMPARE(lifecycleSpy.size(), 0);
    QCOMPARE(page.lifecycleState(), QWebEnginePage::LifecycleState::Active);
    QCOMPARE(visibleSpy.size(), 0);
    QCOMPARE(page.isVisible(), false);
    QCOMPARE(evaluateJavaScriptSync(&page, "document.wasDiscarded"), QVariant(false));
    QCOMPARE(evaluateJavaScriptSync(&page, "frozenness"), QVariant(0));

    // Active -> Frozen
    page.setLifecycleState(QWebEnginePage::LifecycleState::Frozen);
    QCOMPARE(lifecycleSpy.size(), 1);
    QCOMPARE(lifecycleSpy.takeFirst().value(0), QVariant::fromValue(QWebEnginePage::LifecycleState::Frozen));
    QCOMPARE(page.lifecycleState(), QWebEnginePage::LifecycleState::Frozen);
    QCOMPARE(visibleSpy.size(), 0);
    QCOMPARE(page.isVisible(), false);
    QCOMPARE(evaluateJavaScriptSync(&page, "document.wasDiscarded"), QVariant(false));
    QCOMPARE(evaluateJavaScriptSync(&page, "frozenness"), QVariant(1));

    // Frozen -> Active
    page.setLifecycleState(QWebEnginePage::LifecycleState::Active);
    QCOMPARE(lifecycleSpy.size(), 1);
    QCOMPARE(lifecycleSpy.takeFirst().value(0), QVariant::fromValue(QWebEnginePage::LifecycleState::Active));
    QCOMPARE(page.lifecycleState(), QWebEnginePage::LifecycleState::Active);
    QCOMPARE(visibleSpy.size(), 0);
    QCOMPARE(page.isVisible(), false);
    QCOMPARE(evaluateJavaScriptSync(&page, "document.wasDiscarded"), QVariant(false));
    QCOMPARE(evaluateJavaScriptSync(&page, "frozenness"), QVariant(0));

    // Active -> Discarded
    page.setLifecycleState(QWebEnginePage::LifecycleState::Discarded);
    QCOMPARE(lifecycleSpy.size(), 1);
    QCOMPARE(lifecycleSpy.takeFirst().value(0), QVariant::fromValue(QWebEnginePage::LifecycleState::Discarded));
    QCOMPARE(page.lifecycleState(), QWebEnginePage::LifecycleState::Discarded);
    QCOMPARE(visibleSpy.size(), 0);
    QCOMPARE(page.isVisible(), false);
    QTest::ignoreMessage(QtWarningMsg, "runJavaScript: disabled in Discarded state");
    QCOMPARE(evaluateJavaScriptSync(&page, "document.wasDiscarded"), QVariant());
    QTest::ignoreMessage(QtWarningMsg, "runJavaScript: disabled in Discarded state");
    QCOMPARE(evaluateJavaScriptSync(&page, "frozenness"), QVariant());
    QCOMPARE(loadSpy.size(), 0);

    // Discarded -> Frozen (illegal!)
    QTest::ignoreMessage(QtWarningMsg,
                         "setLifecycleState: failed to transition from Discarded to Frozen state: "
                         "illegal transition");
    page.setLifecycleState(QWebEnginePage::LifecycleState::Frozen);
    QCOMPARE(lifecycleSpy.size(), 0);
    QCOMPARE(page.lifecycleState(), QWebEnginePage::LifecycleState::Discarded);

    // Discarded -> Active
    page.setLifecycleState(QWebEnginePage::LifecycleState::Active);
    QTRY_COMPARE(loadSpy.size(), 1);
    QCOMPARE(loadSpy.takeFirst().value(0), QVariant(true));
    QCOMPARE(lifecycleSpy.size(), 1);
    QCOMPARE(lifecycleSpy.takeFirst().value(0), QVariant::fromValue(QWebEnginePage::LifecycleState::Active));
    QCOMPARE(page.lifecycleState(), QWebEnginePage::LifecycleState::Active);
    QCOMPARE(visibleSpy.size(), 0);
    QCOMPARE(page.isVisible(), false);
    QCOMPARE(evaluateJavaScriptSync(&page, "document.wasDiscarded"), QVariant(true));
    QCOMPARE(evaluateJavaScriptSync(&page, "frozenness"), QVariant(0));

    // Active -> Frozen -> Discarded -> Active
    page.setLifecycleState(QWebEnginePage::LifecycleState::Frozen);
    page.setLifecycleState(QWebEnginePage::LifecycleState::Discarded);
    page.setLifecycleState(QWebEnginePage::LifecycleState::Active);
    QCOMPARE(lifecycleSpy.size(), 3);
    QCOMPARE(lifecycleSpy.takeFirst().value(0), QVariant::fromValue(QWebEnginePage::LifecycleState::Frozen));
    QCOMPARE(lifecycleSpy.takeFirst().value(0), QVariant::fromValue(QWebEnginePage::LifecycleState::Discarded));
    QCOMPARE(lifecycleSpy.takeFirst().value(0), QVariant::fromValue(QWebEnginePage::LifecycleState::Active));
    QCOMPARE(page.lifecycleState(), QWebEnginePage::LifecycleState::Active);
    QCOMPARE(visibleSpy.size(), 0);
    QCOMPARE(page.isVisible(), false);
    QTRY_COMPARE(loadSpy.size(), 1);
    QCOMPARE(loadSpy.takeFirst().value(0), QVariant(true));
    QCOMPARE(evaluateJavaScriptSync(&page, "document.wasDiscarded"), QVariant(true));
    QCOMPARE(evaluateJavaScriptSync(&page, "frozenness"), QVariant(0));

    // Reload clears document.wasDiscarded
    page.triggerAction(QWebEnginePage::Reload);
    QTRY_COMPARE(loadSpy.size(), 1);
    QCOMPARE(loadSpy.takeFirst().value(0), QVariant(true));
    QCOMPARE(evaluateJavaScriptSync(&page, "document.wasDiscarded"), QVariant(false));
}

void tst_QWebEnginePage::setVisible()
{
    qRegisterMetaType<QWebEnginePage::LifecycleState>("LifecycleState");

    QWebEngineProfile profile;
    QWebEnginePage page(&profile);
    QSignalSpy loadSpy(&page, &QWebEnginePage::loadFinished);
    QSignalSpy lifecycleSpy(&page, &QWebEnginePage::lifecycleStateChanged);
    QSignalSpy visibleSpy(&page, &QWebEnginePage::visibleChanged);

    page.load(QStringLiteral("about:blank"));
    QTRY_COMPARE(loadSpy.size(), 1);
    QCOMPARE(loadSpy.takeFirst().value(0), QVariant(true));
    QCOMPARE(lifecycleSpy.size(), 0);
    QCOMPARE(page.lifecycleState(), QWebEnginePage::LifecycleState::Active);
    QCOMPARE(visibleSpy.size(), 0);
    QCOMPARE(page.isVisible(), false);

    // hidden -> visible
    page.setVisible(true);
    QCOMPARE(lifecycleSpy.size(), 0);
    QCOMPARE(page.lifecycleState(), QWebEnginePage::LifecycleState::Active);
    QCOMPARE(visibleSpy.size(), 1);
    QCOMPARE(visibleSpy.takeFirst().value(0), QVariant(true));
    QCOMPARE(page.isVisible(), true);

    // Active -> Frozen (illegal)
    QTest::ignoreMessage(
            QtWarningMsg,
            "setLifecycleState: failed to transition from Active to Frozen state: page is visible");
    page.setLifecycleState(QWebEnginePage::LifecycleState::Frozen);
    QCOMPARE(lifecycleSpy.size(), 0);

    // visible -> hidden
    page.setVisible(false);
    QCOMPARE(lifecycleSpy.size(), 0);
    QCOMPARE(page.lifecycleState(), QWebEnginePage::LifecycleState::Active);
    QCOMPARE(visibleSpy.size(), 1);
    QCOMPARE(visibleSpy.takeFirst().value(0), QVariant(false));
    QCOMPARE(page.isVisible(), false);

    // Active -> Frozen
    page.setLifecycleState(QWebEnginePage::LifecycleState::Frozen);
    QCOMPARE(lifecycleSpy.size(), 1);
    QCOMPARE(lifecycleSpy.takeFirst().value(0), QVariant::fromValue(QWebEnginePage::LifecycleState::Frozen));
    QCOMPARE(page.lifecycleState(), QWebEnginePage::LifecycleState::Frozen);

    // hidden -> visible (triggers Frozen -> Active)
    page.setVisible(true);
    QCOMPARE(lifecycleSpy.size(), 1);
    QCOMPARE(lifecycleSpy.takeFirst().value(0), QVariant::fromValue(QWebEnginePage::LifecycleState::Active));
    QCOMPARE(page.lifecycleState(), QWebEnginePage::LifecycleState::Active);
    QCOMPARE(visibleSpy.size(), 1);
    QCOMPARE(visibleSpy.takeFirst().value(0), QVariant(true));
    QCOMPARE(page.isVisible(), true);

    // Active -> Discarded (illegal)
    QTest::ignoreMessage(QtWarningMsg,
                         "setLifecycleState: failed to transition from Active to Discarded state: "
                         "page is visible");
    page.setLifecycleState(QWebEnginePage::LifecycleState::Discarded);
    QCOMPARE(lifecycleSpy.size(), 0);

    // visible -> hidden
    page.setVisible(false);
    QCOMPARE(lifecycleSpy.size(), 0);
    QCOMPARE(page.lifecycleState(), QWebEnginePage::LifecycleState::Active);
    QCOMPARE(visibleSpy.size(), 1);
    QCOMPARE(visibleSpy.takeFirst().value(0), QVariant(false));
    QCOMPARE(page.isVisible(), false);

    // Active -> Discarded
    page.setLifecycleState(QWebEnginePage::LifecycleState::Discarded);
    QCOMPARE(lifecycleSpy.size(), 1);
    QCOMPARE(lifecycleSpy.takeFirst().value(0), QVariant::fromValue(QWebEnginePage::LifecycleState::Discarded));
    QCOMPARE(page.lifecycleState(), QWebEnginePage::LifecycleState::Discarded);

    // hidden -> visible (triggers Discarded -> Active)
    page.setVisible(true);
    QCOMPARE(lifecycleSpy.size(), 1);
    QCOMPARE(lifecycleSpy.takeFirst().value(0), QVariant::fromValue(QWebEnginePage::LifecycleState::Active));
    QCOMPARE(page.lifecycleState(), QWebEnginePage::LifecycleState::Active);
    QCOMPARE(visibleSpy.size(), 1);
    QCOMPARE(visibleSpy.takeFirst().value(0), QVariant(true));
    QCOMPARE(page.isVisible(), true);
    QTRY_COMPARE(loadSpy.size(), 1);
    QCOMPARE(loadSpy.takeFirst().value(0), QVariant(true));
}

void tst_QWebEnginePage::discardPreservesProperties()
{
    QWebEngineProfile profile;
    QWebEnginePage page(&profile);
    QSignalSpy loadSpy(&page, &QWebEnginePage::loadFinished);

    page.load(QStringLiteral("about:blank"));
    QTRY_COMPARE(loadSpy.size(), 1);
    QCOMPARE(loadSpy.takeFirst().value(0), QVariant(true));

    // Change as many properties as possible to non-default values
    bool audioMuted = true;
    QVERIFY(page.isAudioMuted() != audioMuted);
    page.setAudioMuted(audioMuted);
    QColor backgroundColor = Qt::black;
    QVERIFY(page.backgroundColor() != backgroundColor);
    page.setBackgroundColor(backgroundColor);
    qreal zoomFactor = 2;
    QVERIFY(page.zoomFactor() != zoomFactor);
    page.setZoomFactor(zoomFactor);
#if QT_CONFIG(webengine_webchannel)
    QWebChannel *webChannel = new QWebChannel(&page);
    page.setWebChannel(webChannel);
#endif

    // Take snapshot of the rest
    QSizeF contentsSize = page.contentsSize();
    QIcon icon = page.icon();
    QUrl iconUrl = page.iconUrl();
    QUrl requestedUrl = page.requestedUrl();
    QString title = page.title();
    QUrl url = page.url();

    // History should be preserved too
    int historyCount = page.history()->count();
    QCOMPARE(historyCount, 1);
    int historyIndex = page.history()->currentItemIndex();
    QCOMPARE(historyIndex, 0);
    QWebEngineHistoryItem historyItem = page.history()->currentItem();
    QVERIFY(historyItem.isValid());

    // Discard + undiscard
    page.setLifecycleState(QWebEnginePage::LifecycleState::Discarded);
    page.setLifecycleState(QWebEnginePage::LifecycleState::Active);
    QTRY_COMPARE(loadSpy.size(), 1);
    QCOMPARE(loadSpy.takeFirst().value(0), QVariant(true));

    // Property changes should be preserved
    QCOMPARE(page.isAudioMuted(), audioMuted);
    QCOMPARE(page.backgroundColor(), backgroundColor);
    QCOMPARE(page.contentsSize(), contentsSize);
    QCOMPARE(page.icon(), icon);
    QCOMPARE(page.iconUrl(), iconUrl);
    QCOMPARE(page.requestedUrl(), requestedUrl);
    QCOMPARE(page.title(), title);
    QCOMPARE(page.url(), url);
    QCOMPARE(page.zoomFactor(), zoomFactor);
#if QT_CONFIG(webengine_webchannel)
    QCOMPARE(page.webChannel(), webChannel);
#endif
    QCOMPARE(page.history()->count(), historyCount);
    QCOMPARE(page.history()->currentItemIndex(), historyIndex);
    QCOMPARE(page.history()->currentItem().url(), historyItem.url());
    QCOMPARE(page.history()->currentItem().originalUrl(), historyItem.originalUrl());
    QCOMPARE(page.history()->currentItem().title(), historyItem.title());
}

void tst_QWebEnginePage::discardBeforeInitialization()
{
    QWebEngineProfile profile;
    QWebEnginePage page(&profile);
    page.setLifecycleState(QWebEnginePage::LifecycleState::Discarded);
    // The call is ignored
    QCOMPARE(page.lifecycleState(), QWebEnginePage::LifecycleState::Active);
}

void tst_QWebEnginePage::automaticUndiscard()
{
    QWebEngineProfile profile;
    QWebEnginePage page(&profile);
    QSignalSpy loadSpy(&page, &QWebEnginePage::loadFinished);

    page.load(QStringLiteral("about:blank"));
    QTRY_COMPARE(loadSpy.size(), 1);
    QCOMPARE(loadSpy.takeFirst().value(0), QVariant(true));

    // setUrl
    page.setLifecycleState(QWebEnginePage::LifecycleState::Discarded);
    page.setUrl(QStringLiteral("qrc:/resources/lifecycle.html"));
    QCOMPARE(page.lifecycleState(), QWebEnginePage::LifecycleState::Active);

    // setContent
    page.setLifecycleState(QWebEnginePage::LifecycleState::Discarded);
    page.setContent(QByteArrayLiteral("foo"));
    QCOMPARE(page.lifecycleState(), QWebEnginePage::LifecycleState::Active);
}

void tst_QWebEnginePage::setLifecycleStateWithDevTools()
{
    QWebEngineProfile profile;
    QWebEnginePage inspectedPage(&profile);
    QWebEnginePage devToolsPage(&profile);
    QSignalSpy devToolsSpy(&devToolsPage, &QWebEnginePage::loadFinished);
    QSignalSpy inspectedSpy(&inspectedPage, &QWebEnginePage::loadFinished);

    // Ensure pages are initialized
    inspectedPage.load(QStringLiteral("about:blank"));
    devToolsPage.load(QStringLiteral("about:blank"));
    QTRY_COMPARE_WITH_TIMEOUT(inspectedSpy.size(), 1, 90000);
    QCOMPARE(inspectedSpy.takeFirst().value(0), QVariant(true));
    QTRY_COMPARE_WITH_TIMEOUT(devToolsSpy.size(), 1, 90000);
    QCOMPARE(devToolsSpy.takeFirst().value(0), QVariant(true));

    // Open DevTools with Frozen inspectedPage
    inspectedPage.setLifecycleState(QWebEnginePage::LifecycleState::Frozen);
    inspectedPage.setDevToolsPage(&devToolsPage);
    QCOMPARE(inspectedPage.lifecycleState(), QWebEnginePage::LifecycleState::Active);
    QTRY_COMPARE_WITH_TIMEOUT(devToolsSpy.size(), 1, 90000);
    QCOMPARE(devToolsSpy.takeFirst().value(0), QVariant(true));
    inspectedPage.setDevToolsPage(nullptr);

    // Open DevTools with Discarded inspectedPage
    inspectedPage.setLifecycleState(QWebEnginePage::LifecycleState::Discarded);
    inspectedPage.setDevToolsPage(&devToolsPage);
    QCOMPARE(inspectedPage.lifecycleState(), QWebEnginePage::LifecycleState::Active);
    QTRY_COMPARE_WITH_TIMEOUT(devToolsSpy.size(), 1, 90000);
    QCOMPARE(devToolsSpy.takeFirst().value(0), QVariant(true));
    QTRY_COMPARE(inspectedSpy.size(), 1);
    QCOMPARE(inspectedSpy.takeFirst().value(0), QVariant(true));
    inspectedPage.setDevToolsPage(nullptr);

    // Open DevTools with Frozen devToolsPage
    devToolsPage.setLifecycleState(QWebEnginePage::LifecycleState::Frozen);
    devToolsPage.setInspectedPage(&inspectedPage);
    QCOMPARE(devToolsPage.lifecycleState(), QWebEnginePage::LifecycleState::Active);
    QTRY_COMPARE_WITH_TIMEOUT(devToolsSpy.size(), 1, 90000);
    QCOMPARE(devToolsSpy.takeFirst().value(0), QVariant(true));
    devToolsPage.setInspectedPage(nullptr);

    // Open DevTools with Discarded devToolsPage
    devToolsPage.setLifecycleState(QWebEnginePage::LifecycleState::Discarded);
    devToolsPage.setInspectedPage(&inspectedPage);
    QCOMPARE(devToolsPage.lifecycleState(), QWebEnginePage::LifecycleState::Active);
    QTRY_COMPARE_WITH_TIMEOUT(devToolsSpy.size(), 1, 90000);
    QCOMPARE(devToolsSpy.takeFirst().value(0), QVariant(true));
    // keep DevTools open

    // Try to change state while DevTools are open
    QTest::ignoreMessage(
            QtWarningMsg,
            "setLifecycleState: failed to transition from Active to Frozen state: DevTools open");
    inspectedPage.setLifecycleState(QWebEnginePage::LifecycleState::Frozen);
    QCOMPARE(inspectedPage.lifecycleState(), QWebEnginePage::LifecycleState::Active);
    QTest::ignoreMessage(QtWarningMsg,
                         "setLifecycleState: failed to transition from Active to Discarded state: "
                         "DevTools open");
    inspectedPage.setLifecycleState(QWebEnginePage::LifecycleState::Discarded);
    QCOMPARE(inspectedPage.lifecycleState(), QWebEnginePage::LifecycleState::Active);
    QTest::ignoreMessage(
            QtWarningMsg,
            "setLifecycleState: failed to transition from Active to Frozen state: DevTools open");
    devToolsPage.setLifecycleState(QWebEnginePage::LifecycleState::Frozen);
    QCOMPARE(devToolsPage.lifecycleState(), QWebEnginePage::LifecycleState::Active);
    QTest::ignoreMessage(QtWarningMsg,
                         "setLifecycleState: failed to transition from Active to Discarded state: "
                         "DevTools open");
    devToolsPage.setLifecycleState(QWebEnginePage::LifecycleState::Discarded);
    QCOMPARE(devToolsPage.lifecycleState(), QWebEnginePage::LifecycleState::Active);
}

void tst_QWebEnginePage::discardPreservesCommittedLoad()
{
    QWebEngineProfile profile;
    QWebEnginePage page(&profile);
    QSignalSpy loadStartedSpy(&page, &QWebEnginePage::loadStarted);
    QSignalSpy loadFinishedSpy(&page, &QWebEnginePage::loadFinished);
    QSignalSpy urlChangedSpy(&page, &QWebEnginePage::urlChanged);
    QSignalSpy titleChangedSpy(&page, &QWebEnginePage::titleChanged);

    QString url = QStringLiteral("qrc:/resources/lifecycle.html");
    page.setUrl(url);
    QTRY_COMPARE(loadStartedSpy.size(), 1);
    loadStartedSpy.clear();
    QTRY_COMPARE(loadFinishedSpy.size(), 1);
    QCOMPARE(loadFinishedSpy.takeFirst().value(0), QVariant(true));
    QCOMPARE(urlChangedSpy.size(), 1);
    QCOMPARE(urlChangedSpy.takeFirst().value(0), QVariant(QUrl(url)));
    QCOMPARE(page.url(), url);
    QCOMPARE(titleChangedSpy.size(), 2);
    QCOMPARE(titleChangedSpy.takeFirst().value(0), QVariant(url));
    QString title = QStringLiteral("Lifecycle");
    QCOMPARE(titleChangedSpy.takeFirst().value(0), QVariant(title));
    QCOMPARE(page.title(), title);

    page.setLifecycleState(QWebEnginePage::LifecycleState::Discarded);
    QCOMPARE(loadStartedSpy.size(), 0);
    QCOMPARE(loadFinishedSpy.size(), 0);
    QCOMPARE(urlChangedSpy.size(), 0);
    QCOMPARE(page.url(), QUrl(url));
    QCOMPARE(titleChangedSpy.size(), 0);
    QCOMPARE(page.title(), title);

    page.setLifecycleState(QWebEnginePage::LifecycleState::Active);
    QTRY_COMPARE(loadStartedSpy.size(), 1);
    loadStartedSpy.clear();
    QTRY_COMPARE(loadFinishedSpy.size(), 1);
    QCOMPARE(loadFinishedSpy.takeFirst().value(0), QVariant(true));
    QCOMPARE(urlChangedSpy.size(), 0);
    QCOMPARE(page.url(), url);
    QCOMPARE(titleChangedSpy.size(), 0);
    QCOMPARE(page.title(), title);
}

void tst_QWebEnginePage::discardAbortsPendingLoad()
{
    QWebEngineProfile profile;
    QWebEnginePage page(&profile);
    QSignalSpy loadStartedSpy(&page, &QWebEnginePage::loadStarted);
    QSignalSpy loadFinishedSpy(&page, &QWebEnginePage::loadFinished);
    QSignalSpy urlChangedSpy(&page, &QWebEnginePage::urlChanged);
    QSignalSpy titleChangedSpy(&page, &QWebEnginePage::titleChanged);

    connect(&page, &QWebEnginePage::loadStarted,
            [&]() { page.setLifecycleState(QWebEnginePage::LifecycleState::Discarded); });
    QUrl url = QStringLiteral("qrc:/resources/lifecycle.html");
    page.setUrl(url);
    QTRY_COMPARE(loadStartedSpy.size(), 1);
    loadStartedSpy.clear();
    QTRY_COMPARE(loadFinishedSpy.size(), 1);
    QCOMPARE(loadFinishedSpy.takeFirst().value(0), QVariant(false));
    QCOMPARE(urlChangedSpy.size(), 2);
    QCOMPARE(urlChangedSpy.takeFirst().value(0), QVariant(url));
    QCOMPARE(urlChangedSpy.takeFirst().value(0), QVariant(QUrl()));
    QCOMPARE(titleChangedSpy.size(), 0);
    QCOMPARE(page.url(), QUrl());
    QCOMPARE(page.title(), QString());

    page.setLifecycleState(QWebEnginePage::LifecycleState::Active);
    QCOMPARE(loadStartedSpy.size(), 0);
    QCOMPARE(loadFinishedSpy.size(), 0);
    QCOMPARE(urlChangedSpy.size(), 0);
    QCOMPARE(page.url(), QUrl());
    QCOMPARE(page.title(), QString());
}

void tst_QWebEnginePage::discardAbortsPendingLoadAndPreservesCommittedLoad()
{
    QWebEngineProfile profile;
    QWebEnginePage page(&profile);
    QSignalSpy loadStartedSpy(&page, &QWebEnginePage::loadStarted);
    QSignalSpy loadFinishedSpy(&page, &QWebEnginePage::loadFinished);
    QSignalSpy urlChangedSpy(&page, &QWebEnginePage::urlChanged);
    QSignalSpy titleChangedSpy(&page, &QWebEnginePage::titleChanged);

    QString url1 = QStringLiteral("qrc:/resources/lifecycle.html");
    page.setUrl(url1);
    QTRY_COMPARE(loadStartedSpy.size(), 1);
    loadStartedSpy.clear();
    QTRY_COMPARE(loadFinishedSpy.size(), 1);
    QCOMPARE(loadFinishedSpy.takeFirst().value(0), QVariant(true));
    QCOMPARE(urlChangedSpy.size(), 1);
    QCOMPARE(urlChangedSpy.takeFirst().value(0), QVariant(QUrl(url1)));
    QCOMPARE(page.url(), url1);
    QCOMPARE(titleChangedSpy.size(), 2);
    QCOMPARE(titleChangedSpy.takeFirst().value(0), QVariant(url1));
    QString title = QStringLiteral("Lifecycle");
    QCOMPARE(titleChangedSpy.takeFirst().value(0), QVariant(title));
    QCOMPARE(page.title(), title);

    connect(&page, &QWebEnginePage::loadStarted,
            [&]() { page.setLifecycleState(QWebEnginePage::LifecycleState::Discarded); });
    QString url2 = QStringLiteral("about:blank");
    page.setUrl(url2);
    QTRY_COMPARE(loadStartedSpy.size(), 1);
    loadStartedSpy.clear();
    QTRY_COMPARE(loadFinishedSpy.size(), 1);
    QCOMPARE(loadFinishedSpy.takeFirst().value(0), QVariant(false));
    QCOMPARE(urlChangedSpy.size(), 2);
    QCOMPARE(urlChangedSpy.takeFirst().value(0), QVariant(QUrl(url2)));
    QCOMPARE(urlChangedSpy.takeFirst().value(0), QVariant(QUrl(url1)));
    QCOMPARE(titleChangedSpy.size(), 0);
    QCOMPARE(page.url(), url1);
    QCOMPARE(page.title(), title);

    page.setLifecycleState(QWebEnginePage::LifecycleState::Active);
    QCOMPARE(loadStartedSpy.size(), 0);
    QCOMPARE(loadFinishedSpy.size(), 0);
    QCOMPARE(urlChangedSpy.size(), 0);
    QCOMPARE(page.url(), url1);
    QCOMPARE(page.title(), title);
}

void tst_QWebEnginePage::recommendedState()
{
    qRegisterMetaType<QWebEnginePage::LifecycleState>("LifecycleState");

    QWebEngineProfile profile;
    QWebEnginePage page(&profile);

    struct Event {
        enum { StateChange, RecommendationChange } key;
        QWebEnginePage::LifecycleState value;
    };
    std::vector<Event> events;
    connect(&page, &QWebEnginePage::lifecycleStateChanged, [&](QWebEnginePage::LifecycleState state) {
        events.push_back(Event { Event::StateChange, state });
    });
    connect(&page, &QWebEnginePage::recommendedStateChanged, [&](QWebEnginePage::LifecycleState state) {
        events.push_back(Event { Event::RecommendationChange, state });
    });

    page.load(QStringLiteral("qrc:/resources/lifecycle.html"));
    QTRY_COMPARE(events.size(), 1u);
    QCOMPARE(events[0].key, Event::RecommendationChange);
    QCOMPARE(events[0].value, QWebEnginePage::LifecycleState::Frozen);
    events.clear();
    QCOMPARE(page.recommendedState(), QWebEnginePage::LifecycleState::Frozen);

    page.setVisible(true);
    QTRY_COMPARE(events.size(), 1u);
    QCOMPARE(events[0].key, Event::RecommendationChange);
    QCOMPARE(events[0].value, QWebEnginePage::LifecycleState::Active);
    events.clear();
    QCOMPARE(page.recommendedState(), QWebEnginePage::LifecycleState::Active);

    page.setVisible(false);
    QTRY_COMPARE(events.size(), 1u);
    QCOMPARE(events[0].key, Event::RecommendationChange);
    QCOMPARE(events[0].value, QWebEnginePage::LifecycleState::Frozen);
    events.clear();
    QCOMPARE(page.recommendedState(), QWebEnginePage::LifecycleState::Frozen);

    page.triggerAction(QWebEnginePage::Reload);
    QTRY_COMPARE(events.size(), 2u);
    QCOMPARE(events[0].key, Event::RecommendationChange);
    QCOMPARE(events[0].value, QWebEnginePage::LifecycleState::Active);
    QCOMPARE(events[1].key, Event::RecommendationChange);
    QCOMPARE(events[1].value, QWebEnginePage::LifecycleState::Frozen);
    events.clear();
    QCOMPARE(page.recommendedState(), QWebEnginePage::LifecycleState::Frozen);

    QWebEnginePage devTools;
    page.setDevToolsPage(&devTools);
    QTRY_COMPARE(events.size(), 1u);
    QCOMPARE(events[0].key, Event::RecommendationChange);
    QCOMPARE(events[0].value, QWebEnginePage::LifecycleState::Active);
    events.clear();
    QCOMPARE(page.recommendedState(), QWebEnginePage::LifecycleState::Active);

    page.setDevToolsPage(nullptr);
    QTRY_COMPARE(events.size(), 1u);
    QCOMPARE(events[0].key, Event::RecommendationChange);
    QCOMPARE(events[0].value, QWebEnginePage::LifecycleState::Frozen);
    events.clear();
    QCOMPARE(page.recommendedState(), QWebEnginePage::LifecycleState::Frozen);

    page.setLifecycleState(QWebEnginePage::LifecycleState::Frozen);
    QTRY_COMPARE(events.size(), 2u);
    QCOMPARE(events[0].key, Event::StateChange);
    QCOMPARE(events[0].value, QWebEnginePage::LifecycleState::Frozen);
    QCOMPARE(events[1].key, Event::RecommendationChange);
    QCOMPARE(events[1].value, QWebEnginePage::LifecycleState::Discarded);
    events.clear();
    QCOMPARE(page.recommendedState(), QWebEnginePage::LifecycleState::Discarded);

    page.setLifecycleState(QWebEnginePage::LifecycleState::Discarded);
    QTRY_COMPARE(events.size(), 1u);
    QCOMPARE(events[0].key, Event::StateChange);
    QCOMPARE(events[0].value, QWebEnginePage::LifecycleState::Discarded);
    events.clear();
    QCOMPARE(page.recommendedState(), QWebEnginePage::LifecycleState::Discarded);
}

void tst_QWebEnginePage::recommendedStateAuto()
{
    qRegisterMetaType<QWebEnginePage::LifecycleState>("LifecycleState");

    QWebEngineProfile profile;
    QWebEnginePage page(&profile);
    QSignalSpy lifecycleSpy(&page, &QWebEnginePage::lifecycleStateChanged);
    connect(&page, &QWebEnginePage::recommendedStateChanged, &page, &QWebEnginePage::setLifecycleState);

    page.load(QStringLiteral("qrc:/resources/lifecycle.html"));
    QTRY_COMPARE(lifecycleSpy.size(), 2);
    QCOMPARE(lifecycleSpy.takeFirst().value(0), QVariant::fromValue(QWebEnginePage::LifecycleState::Frozen));
    QCOMPARE(lifecycleSpy.takeFirst().value(0), QVariant::fromValue(QWebEnginePage::LifecycleState::Discarded));

    page.setVisible(true);
    QTRY_COMPARE(lifecycleSpy.size(), 1);
    QCOMPARE(lifecycleSpy.takeFirst().value(0), QVariant::fromValue(QWebEnginePage::LifecycleState::Active));

    page.setVisible(false);
    QTRY_COMPARE(lifecycleSpy.size(), 2);
    QCOMPARE(lifecycleSpy.takeFirst().value(0), QVariant::fromValue(QWebEnginePage::LifecycleState::Frozen));
    QCOMPARE(lifecycleSpy.takeFirst().value(0), QVariant::fromValue(QWebEnginePage::LifecycleState::Discarded));

    page.triggerAction(QWebEnginePage::Reload);
    QTRY_COMPARE(lifecycleSpy.size(), 3);
    QCOMPARE(lifecycleSpy.takeFirst().value(0), QVariant::fromValue(QWebEnginePage::LifecycleState::Active));
    QCOMPARE(lifecycleSpy.takeFirst().value(0), QVariant::fromValue(QWebEnginePage::LifecycleState::Frozen));
    QCOMPARE(lifecycleSpy.takeFirst().value(0), QVariant::fromValue(QWebEnginePage::LifecycleState::Discarded));

    QWebEnginePage devTools;
    page.setDevToolsPage(&devTools);
    QTRY_COMPARE(lifecycleSpy.size(), 1);
    QCOMPARE(lifecycleSpy.takeFirst().value(0), QVariant::fromValue(QWebEnginePage::LifecycleState::Active));

    page.setDevToolsPage(nullptr);
    QTRY_COMPARE(lifecycleSpy.size(), 2);
    QCOMPARE(lifecycleSpy.takeFirst().value(0), QVariant::fromValue(QWebEnginePage::LifecycleState::Frozen));
    QCOMPARE(lifecycleSpy.takeFirst().value(0), QVariant::fromValue(QWebEnginePage::LifecycleState::Discarded));
}

void tst_QWebEnginePage::setLifecycleStateAndReload()
{
    qRegisterMetaType<QWebEnginePage::LifecycleState>("LifecycleState");

    QWebEngineProfile profile;
    QWebEnginePage page(&profile);
    QSignalSpy loadSpy(&page, &QWebEnginePage::loadFinished);
    QSignalSpy lifecycleSpy(&page, &QWebEnginePage::lifecycleStateChanged);

    page.load(QStringLiteral("qrc:/resources/lifecycle.html"));
    QTRY_COMPARE(loadSpy.size(), 1);
    QCOMPARE(loadSpy.takeFirst().value(0), QVariant(true));
    QCOMPARE(lifecycleSpy.size(), 0);
    QCOMPARE(page.lifecycleState(), QWebEnginePage::LifecycleState::Active);

    page.setLifecycleState(QWebEnginePage::LifecycleState::Frozen);
    QCOMPARE(page.lifecycleState(), QWebEnginePage::LifecycleState::Frozen);
    QCOMPARE(lifecycleSpy.size(), 1);
    QCOMPARE(lifecycleSpy.takeFirst().value(0), QVariant::fromValue(QWebEnginePage::LifecycleState::Frozen));

    page.triggerAction(QWebEnginePage::Reload);
    QCOMPARE(page.lifecycleState(), QWebEnginePage::LifecycleState::Active);
    QCOMPARE(lifecycleSpy.size(), 1);
    QCOMPARE(lifecycleSpy.takeFirst().value(0), QVariant::fromValue(QWebEnginePage::LifecycleState::Active));
    QTRY_COMPARE(loadSpy.size(), 1);
    QCOMPARE(loadSpy.takeFirst().value(0), QVariant(true));

    page.setLifecycleState(QWebEnginePage::LifecycleState::Discarded);
    QCOMPARE(page.lifecycleState(), QWebEnginePage::LifecycleState::Discarded);
    QCOMPARE(lifecycleSpy.size(), 1);
    QCOMPARE(lifecycleSpy.takeFirst().value(0), QVariant::fromValue(QWebEnginePage::LifecycleState::Discarded));

    page.triggerAction(QWebEnginePage::Reload);
    QCOMPARE(page.lifecycleState(), QWebEnginePage::LifecycleState::Active);
    QCOMPARE(lifecycleSpy.size(), 1);
    QCOMPARE(lifecycleSpy.takeFirst().value(0), QVariant::fromValue(QWebEnginePage::LifecycleState::Active));
    QTRY_COMPARE(loadSpy.size(), 1);
    QCOMPARE(loadSpy.takeFirst().value(0), QVariant(true));
}

void tst_QWebEnginePage::editActionsWithExplicitFocus()
{
    QWebEngineView view;
    QWebEnginePage *page = view.page();
    view.settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, false);

    QSignalSpy loadFinishedSpy(page, &QWebEnginePage::loadFinished);
    QSignalSpy selectionChangedSpy(page, &QWebEnginePage::selectionChanged);
    QSignalSpy actionChangedSpy(page->action(QWebEnginePage::SelectAll), &QAction::changed);

    // The view is hidden and no focus on the page. Edit actions should be disabled.
    QVERIFY(!view.isVisible());
    QVERIFY(!page->action(QWebEnginePage::SelectAll)->isEnabled());

    page->setHtml(QString("<html><body><div>foo bar</div></body></html>"));
    QTRY_COMPARE(loadFinishedSpy.size(), 1);

    // Still no focus because focus on navigation is disabled. Edit actions don't do anything (should not crash).
    QVERIFY(!page->action(QWebEnginePage::SelectAll)->isEnabled());
    view.page()->triggerAction(QWebEnginePage::SelectAll);
    QCOMPARE(selectionChangedSpy.size(), 0);
    QCOMPARE(page->hasSelection(), false);

    // Focus content by focusing window from JavaScript. Edit actions should be enabled and functional.
    evaluateJavaScriptSync(page, "window.focus();");
    QTRY_COMPARE(actionChangedSpy.size(), 1);
    QVERIFY(page->action(QWebEnginePage::SelectAll)->isEnabled());
    view.page()->triggerAction(QWebEnginePage::SelectAll);
    QTRY_COMPARE(selectionChangedSpy.size(), 1);
    QCOMPARE(page->hasSelection(), true);
    QCOMPARE(page->selectedText(), QStringLiteral("foo bar"));
}

void tst_QWebEnginePage::editActionsWithInitialFocus()
{
    QWebEngineView view;
    QWebEnginePage *page = view.page();
    view.settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, true);

    QSignalSpy loadFinishedSpy(page, &QWebEnginePage::loadFinished);
    QSignalSpy selectionChangedSpy(page, &QWebEnginePage::selectionChanged);
    QSignalSpy actionChangedSpy(page->action(QWebEnginePage::SelectAll), &QAction::changed);

    // The view is hidden and no focus on the page. Edit actions should be disabled.
    QVERIFY(!view.isVisible());
    QVERIFY(!page->action(QWebEnginePage::SelectAll)->isEnabled());

    page->setHtml(QString("<html><body><div>foo bar</div></body></html>"));
    QTRY_COMPARE(loadFinishedSpy.size(), 1);

    // Content gets initial focus.
    QTRY_COMPARE(actionChangedSpy.size(), 1);
    QVERIFY(page->action(QWebEnginePage::SelectAll)->isEnabled());
    view.page()->triggerAction(QWebEnginePage::SelectAll);
    QTRY_COMPARE(selectionChangedSpy.size(), 1);
    QCOMPARE(page->hasSelection(), true);
    QCOMPARE(page->selectedText(), QStringLiteral("foo bar"));
}

void tst_QWebEnginePage::editActionsWithFocusOnIframe()
{
    QWebEngineView view;
    QWebEnginePage *page = view.page();
    view.settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, false);

    QSignalSpy loadFinishedSpy(page, &QWebEnginePage::loadFinished);
    QSignalSpy selectionChangedSpy(page, &QWebEnginePage::selectionChanged);
    QSignalSpy actionChangedSpy(page->action(QWebEnginePage::SelectAll), &QAction::changed);

    // The view is hidden and no focus on the page. Edit actions should be disabled.
    QVERIFY(!view.isVisible());
    QVERIFY(!page->action(QWebEnginePage::SelectAll)->isEnabled());

    page->load(QUrl("qrc:///resources/iframe2.html"));
    QTRY_COMPARE(loadFinishedSpy.size(), 1);
    QVERIFY(!page->action(QWebEnginePage::SelectAll)->isEnabled());

    // Focusing an iframe.
    evaluateJavaScriptSync(page, "document.getElementsByTagName('iframe')[0].contentWindow.focus()");
    QTRY_COMPARE(actionChangedSpy.size(), 1);
    QVERIFY(page->action(QWebEnginePage::SelectAll)->isEnabled());
    view.page()->triggerAction(QWebEnginePage::SelectAll);
    QTRY_COMPARE(selectionChangedSpy.size(), 1);
    QCOMPARE(page->hasSelection(), true);
    QCOMPARE(page->selectedText(), QStringLiteral("inner"));
}

void tst_QWebEnginePage::editActionsWithoutSelection()
{
    QWebEngineView view;
    QWebEnginePage *page = view.page();
    view.settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, true);

    QSignalSpy loadFinishedSpy(page, &QWebEnginePage::loadFinished);
    QSignalSpy selectionChangedSpy(page, &QWebEnginePage::selectionChanged);
    QSignalSpy actionChangedSpy(page->action(QWebEnginePage::SelectAll), &QAction::changed);

    page->setHtml(QString("<html><body><div>foo bar</div></body></html>"));
    QTRY_COMPARE(loadFinishedSpy.size(), 1);
    QTRY_COMPARE(actionChangedSpy.size(), 1);

    QVERIFY(!page->action(QWebEnginePage::Cut)->isEnabled());
    QVERIFY(!page->action(QWebEnginePage::Copy)->isEnabled());
    QVERIFY(page->action(QWebEnginePage::Paste)->isEnabled());
    QVERIFY(page->action(QWebEnginePage::Undo)->isEnabled());
    QVERIFY(page->action(QWebEnginePage::Redo)->isEnabled());
    QVERIFY(page->action(QWebEnginePage::SelectAll)->isEnabled());
    QVERIFY(page->action(QWebEnginePage::PasteAndMatchStyle)->isEnabled());
    QVERIFY(!page->action(QWebEnginePage::Unselect)->isEnabled());

    page->triggerAction(QWebEnginePage::SelectAll);
    QTRY_COMPARE(selectionChangedSpy.size(), 1);
    QCOMPARE(page->hasSelection(), true);
    QCOMPARE(page->selectedText(), QStringLiteral("foo bar"));

    QVERIFY(page->action(QWebEnginePage::Cut)->isEnabled());
    QVERIFY(page->action(QWebEnginePage::Copy)->isEnabled());
    QVERIFY(page->action(QWebEnginePage::Paste)->isEnabled());
    QVERIFY(page->action(QWebEnginePage::Undo)->isEnabled());
    QVERIFY(page->action(QWebEnginePage::Redo)->isEnabled());
    QVERIFY(page->action(QWebEnginePage::SelectAll)->isEnabled());
    QVERIFY(page->action(QWebEnginePage::PasteAndMatchStyle)->isEnabled());
    QVERIFY(page->action(QWebEnginePage::Unselect)->isEnabled());
}

struct PageWithNewWindowHandler : QWebEnginePage
{
    QScopedPointer<PageWithNewWindowHandler> newPage;
    bool handleInSignal;
    QWebEngineProfile *targetProfile = nullptr;
    QSignalSpy loadSpy { this, &QWebEnginePage::loadFinished };
    PageWithNewWindowHandler(QWebEngineProfile *p, bool inSignal = false, QWebEngineProfile *tp = nullptr)
        : QWebEnginePage(p), handleInSignal(inSignal), targetProfile(tp) {
        if (handleInSignal)
            connect(this, &QWebEnginePage::newWindowRequested, this, [this] (QWebEngineNewWindowRequest &r) {
                newPage.reset(new PageWithNewWindowHandler(targetProfile ? targetProfile : profile(), handleInSignal));
                newPage->acceptAsNewWindow(r);
            });
    }
    QWebEnginePage *createWindow(WebWindowType) override {
        if (handleInSignal)
            return nullptr;
        newPage.reset(new PageWithNewWindowHandler(targetProfile ? targetProfile : profile(), handleInSignal));
        return newPage.get();
    }
};

void tst_QWebEnginePage::customUserAgentInNewTab()
{
    HttpServer server;
    QByteArray lastUserAgent;
    connect(&server, &HttpServer::newRequest, [&](HttpReqRep *rr) {
        QCOMPARE(rr->requestMethod(), "GET");
        lastUserAgent = rr->requestHeader("user-agent");
        rr->setResponseBody(QByteArrayLiteral("<html><body>Test</body></html>"));
        rr->sendResponse();
    });
    QVERIFY(server.start());

    QString expectedUserAgent("custom 1");
    QWebEngineProfile profile;
    profile.setHttpUserAgent(expectedUserAgent);

    PageWithNewWindowHandler page(&profile);
    QWebEngineView view; view.resize(500, 500); view.setPage(&page); view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    // First check we can get the user-agent passed through normally
    page.setHtml(QString("<html><body><a id='link' target='_blank' href='") +
                 server.url("/test1").toEncoded() +
                 QString("'>link</a></body></html>"));
    QTRY_COMPARE(page.loadSpy.size(), 1);
    QVERIFY(page.loadSpy.takeFirst().value(0).toBool());
    QCOMPARE(evaluateJavaScriptSync(&page, QStringLiteral("navigator.userAgent")).toString(), expectedUserAgent);
    QTest::mouseClick(view.focusProxy(), Qt::LeftButton, {}, elementCenter(&page, "link"));
    QTRY_VERIFY(page.newPage);
    QTRY_COMPARE(page.newPage->loadSpy.size(), 1);
    QTRY_VERIFY(!lastUserAgent.isEmpty());
    QCOMPARE(lastUserAgent, expectedUserAgent);
    QCOMPARE(evaluateJavaScriptSync(page.newPage.get(), QStringLiteral("navigator.userAgent")).toString(), expectedUserAgent);

    // Now check we can get the new user-agent of the profile
    page.newPage.reset();
    expectedUserAgent = "custom 2";
    profile.setHttpUserAgent(expectedUserAgent);
    page.loadSpy.clear();
    lastUserAgent = { };
    page.setHtml(QString("<html><body><a id='link' target='_blank' href='") +
                 server.url("/test2").toEncoded() +
                 QString("'>link</a></body></html>"));
    QTRY_COMPARE(page.loadSpy.size(), 1);
    QVERIFY(page.loadSpy.takeFirst().value(0).toBool());
    QTest::mouseClick(view.focusProxy(), Qt::LeftButton, {}, elementCenter(&page, "link"));
    QTRY_VERIFY(page.newPage);
    QTRY_COMPARE(page.newPage->loadSpy.size(), 1);
    QTRY_VERIFY(!lastUserAgent.isEmpty());
    QCOMPARE(lastUserAgent, expectedUserAgent);
    QCOMPARE(evaluateJavaScriptSync(&page, QStringLiteral("navigator.userAgent")).toString(), expectedUserAgent);
    QCOMPARE(evaluateJavaScriptSync(page.newPage.get(), QStringLiteral("navigator.userAgent")).toString(), expectedUserAgent);
}

void tst_QWebEnginePage::openNewTabInDifferentProfile_data()
{
    QTest::addColumn<bool>("handleInSignal");
    QTest::addRow("handleInSignal") << true;
    QTest::addRow("handleInOverride") << false;
}

void tst_QWebEnginePage::openNewTabInDifferentProfile()
{
    QFETCH(bool, handleInSignal);

    HttpServer server;
    QStringList receivedRequests;
    connect(&server, &HttpServer::newRequest, [&] (HttpReqRep *r) {
        receivedRequests.append(r->requestPath());
        r->setResponseBody("DUMMY");
        r->sendResponse();
    });
    QVERIFY(server.start());

    QWebEngineProfile profile1, profile2;
    PageWithNewWindowHandler page(&profile1, handleInSignal, &profile2);
    QWebEngineView view; view.setPage(&page); view.resize(320, 240); view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    page.setHtml(QString("<html><body><a id='link' target='_blank' href='%1'>link</a></body></html>").arg(server.url("/first.html").toEncoded()));
    QTRY_COMPARE(page.loadSpy.size(), 1);
    QVERIFY(page.loadSpy.takeFirst().value(0).toBool());

    QTest::mouseClick(view.focusProxy(), Qt::LeftButton, {}, elementCenter(&page, "link"));
    QTRY_VERIFY(page.newPage);
    QVERIFY(page.profile() == &profile1);
    QVERIFY(page.newPage->profile() == &profile2);
    // not load should occur or requests to server issued since web_contents is not expected to be adopted from other profile
    QTRY_LOOP_IMPL(page.newPage->loadSpy.size() != 0, 1000, 100);
    QVERIFY2(receivedRequests.isEmpty(), qPrintable(receivedRequests.join(", ")));
}

void tst_QWebEnginePage::renderProcessCrashed()
{
    using Status = QWebEnginePage::RenderProcessTerminationStatus;
    QWebEngineProfile profile;
    QWebEnginePage page(&profile);
    bool done = false;
    Status status;
    connect(&page, &QWebEnginePage::renderProcessTerminated, [&](Status newStatus) {
        status = newStatus;
        done = true;
    });
    page.load(QUrl("chrome://crash"));
    QTRY_VERIFY_WITH_TIMEOUT(done, 20000);
    // The status depends on whether stack traces are enabled. With
    // --disable-in-process-stack-traces we get an AbnormalTerminationStatus,
    // otherwise a CrashedTerminationStatus.
    QVERIFY(status == QWebEnginePage::CrashedTerminationStatus ||
            status == QWebEnginePage::AbnormalTerminationStatus);
}

void tst_QWebEnginePage::renderProcessPid()
{
    QCOMPARE(m_page->renderProcessPid(), 0);

    m_page->load(QUrl("about:blank"));
    QSignalSpy spyFinished(m_page, &QWebEnginePage::loadFinished);
    QVERIFY(spyFinished.wait());

    QVERIFY(m_page->renderProcessPid() > 1);

    bool crashed = false;
    connect(m_page, &QWebEnginePage::renderProcessTerminated, [&]() { crashed = true; });
    m_page->load(QUrl("chrome://crash"));
    QTRY_VERIFY_WITH_TIMEOUT(crashed, 20000);

    QCOMPARE(m_page->renderProcessPid(), 0);
}

class FileSelectionTestPage : public QWebEnginePage {
public:
    FileSelectionTestPage() : m_tempDir(QDir::tempPath() + "/tst_qwebenginepage-XXXXXX") { }

    QStringList chooseFiles(FileSelectionMode mode, const QStringList &oldFiles, const QStringList &acceptedMimeTypes) override
    {
        Q_UNUSED(oldFiles);
        chosenFileSelectionMode = mode;
        chosenAcceptedMimeTypes = acceptedMimeTypes;
        return QStringList() << (m_tempDir.path() + "/file.txt");
    }

    QTemporaryDir m_tempDir;
    int chosenFileSelectionMode = -1;
    QStringList chosenAcceptedMimeTypes;
};

void tst_QWebEnginePage::testChooseFilesParameters_data()
{
    QTest::addColumn<QString>("uploadAttribute");
    QTest::addColumn<QString>("mimeTypeAttribute");
    QTest::addColumn<QWebEnginePage::FileSelectionMode>("expectedFileSelectionMode");
    QTest::addColumn<QStringList>("expectedMimeType");
    QStringList mimeTypes;

    QTest::addRow("Single file upload") << QString() << QString()
                                        << QWebEnginePage::FileSelectOpen << QStringList();
    QTest::addRow("Multiple file upload") << QString("multiple") << QString()
                                          << QWebEnginePage::FileSelectOpenMultiple << QStringList();
    QTest::addRow("Folder upload") << QString("multiple webkitdirectory") << QString()
                                   << QWebEnginePage::FileSelectUploadFolder << QStringList();
    QTest::addRow("Save file") << QString("") << QString()
                                   << QWebEnginePage::FileSelectSave << QStringList();
    mimeTypes = QStringList() << "audio/*";
    QTest::addRow("MIME type: audio") << QString() << QString("accept='%1'").arg(mimeTypes.join(','))
                                      << QWebEnginePage::FileSelectOpen << mimeTypes;
    mimeTypes = QStringList() << "video/*";
    QTest::addRow("MIME type: video") << QString() << QString("accept='%1'").arg(mimeTypes.join(','))
                                      << QWebEnginePage::FileSelectOpen << mimeTypes;
    mimeTypes = QStringList() << "image/*";
    QTest::addRow("MIME type: image") << QString() << QString("accept='%1'").arg(mimeTypes.join(','))
                                      << QWebEnginePage::FileSelectOpen << mimeTypes;
    mimeTypes = QStringList() << ".txt" << ".html";
    QTest::addRow("MIME type: custom") << QString() << QString("accept='%1'").arg(mimeTypes.join(','))
                                       << QWebEnginePage::FileSelectOpen << mimeTypes;
    mimeTypes = QStringList() << "audio/*" << "video/*" << "image/*" << ".txt" << ".html";
    QTest::addRow("Multiple MIME types") << QString() << QString("accept='%1'").arg(mimeTypes.join(','))
                                        << QWebEnginePage::FileSelectOpen << mimeTypes;
}

void tst_QWebEnginePage::testChooseFilesParameters()
{
    QFETCH(QString, uploadAttribute);
    QFETCH(QString, mimeTypeAttribute);
    QFETCH(QWebEnginePage::FileSelectionMode, expectedFileSelectionMode);
    QFETCH(QStringList, expectedMimeType);

    FileSelectionTestPage page;
    QSignalSpy spyFinished(&page, &QWebEnginePage::loadFinished);

    QWebEngineView view;
    view.resize(500, 500);
    view.setPage(&page);
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    if (expectedFileSelectionMode != QWebEnginePage::FileSelectSave) {
        page.setHtml(QString("<html><body>"
                             "<input id='filePicker' type='file' name='filePicker' %1 %2 />"
                             "</body></html>").arg(uploadAttribute, mimeTypeAttribute));
    } else {
        page.setHtml(QString("<html><body>"
                             "<button id='filePicker' value='trigger' "
                             "onclick='window.showSaveFilePicker()'"
                             "</body></html>"), QString("qrc:/"));
    }
    QVERIFY(spyFinished.wait());
    QTRY_COMPARE(spyFinished.size(), 1);

    evaluateJavaScriptSync(view.page(), "document.getElementById('filePicker').focus()");
    QTRY_COMPARE(evaluateJavaScriptSync(view.page(), "document.activeElement.id").toString(), QStringLiteral("filePicker"));
    QTest::keyClick(view.focusProxy(), Qt::Key_Enter);

    QTRY_COMPARE(page.chosenFileSelectionMode, expectedFileSelectionMode);
    QTRY_COMPARE(page.chosenAcceptedMimeTypes, expectedMimeType);
}

void tst_QWebEnginePage::fileSystemAccessDialog()
{
    FileSelectionTestPage page;
    QSignalSpy spyFinished(&page, &QWebEnginePage::loadFinished);

    QWebEngineView view;
    view.resize(500, 500);
    view.setPage(&page);
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    connect(&page, &QWebEnginePage::fileSystemAccessRequested,
            [](QWebEngineFileSystemAccessRequest request) {
                QCOMPARE(request.accessFlags(),
                         QWebEngineFileSystemAccessRequest::Read
                                 | QWebEngineFileSystemAccessRequest::Write);
                request.accept();
            });

    page.setHtml(QString("<html><head><script>"
                         "async function getTemporaryDir() {"
                         "  const newHandle = await window.showSaveFilePicker();"
                         "  const writable = await newHandle.createWritable();"
                         "  await writable.write(new Blob(['New value']));"
                         "  await writable.close();"
                         ""
                         "  const fileData = await newHandle.getFile();"
                         "  document.title = await fileData.text();"
                         "}"
                         "</script></head><body>"
                         "<button id='triggerDialog' value='trigger' "
                         "onclick='getTemporaryDir()'"
                         "</body></html>"),
                 QString("qrc:/"));
    QVERIFY(spyFinished.wait());
    QTRY_COMPARE(spyFinished.size(), 1);

    evaluateJavaScriptSync(view.page(), "document.getElementById('triggerDialog').focus()");
    QTRY_COMPARE(evaluateJavaScriptSync(view.page(), "document.activeElement.id").toString(),
                 QStringLiteral("triggerDialog"));
    QTest::keyClick(view.focusProxy(), Qt::Key_Enter);

    QTRY_COMPARE(page.title(), "New value");

    QTRY_COMPARE(page.chosenFileSelectionMode, QWebEnginePage::FileSelectSave);
    QTRY_COMPARE(page.chosenAcceptedMimeTypes, QStringList());
}

void tst_QWebEnginePage::backgroundColor()
{
    QWebEngineProfile profile;
    QWebEngineView view;
    QWebEnginePage *page = new QWebEnginePage(&profile, &view);

    view.resize(640, 480);
    view.setStyleSheet("background: yellow");
    view.show();
    QPoint center(view.size().width() / 2, view.size().height() / 2);

    QCOMPARE(page->backgroundColor(), Qt::white);
    QTRY_COMPARE(view.grab().toImage().pixelColor(center), Qt::white);

    page->setBackgroundColor(Qt::red);
    view.setPage(page);

    QCOMPARE(page->backgroundColor(), Qt::red);
    QTRY_COMPARE(view.grab().toImage().pixelColor(center), Qt::red);

    page->setHtml(QString("<html>"
                          "<head><style>html, body { margin:0; padding:0; }</style></head>"
                          "<body><div style=\"width:100%; height:10px; background-color:black\"/></body>"
                          "</html>"));
    QSignalSpy spyFinished(page, &QWebEnginePage::loadFinished);
    QVERIFY(spyFinished.wait());
    // Make sure the page is rendered and the test is not grabbing the color of the RenderWidgetHostViewQtDelegateWidget.
    QTRY_COMPARE(view.grab().toImage().pixelColor(QPoint(5, 5)), Qt::black);

    QCOMPARE(page->backgroundColor(), Qt::red);
    QCOMPARE(view.grab().toImage().pixelColor(center), Qt::red);

    page->setBackgroundColor(Qt::transparent);

    QCOMPARE(page->backgroundColor(), Qt::transparent);
    QTRY_COMPARE(view.grab().toImage().pixelColor(center), Qt::yellow);

    page->setBackgroundColor(Qt::green);

    QCOMPARE(page->backgroundColor(), Qt::green);
    QTRY_COMPARE(view.grab().toImage().pixelColor(center), Qt::green);
}

void tst_QWebEnginePage::audioMuted()
{
    QWebEngineProfile profile;
    QWebEnginePage page(&profile);
    QSignalSpy spy(&page, &QWebEnginePage::audioMutedChanged);

    QCOMPARE(page.isAudioMuted(), false);
    page.setAudioMuted(true);
    loadSync(&page, QUrl("about:blank"));
    QCOMPARE(page.isAudioMuted(), true);
    QCOMPARE(spy.size(), 1);
    QCOMPARE(spy[0][0], QVariant(true));
    page.setAudioMuted(false);
    QCOMPARE(page.isAudioMuted(), false);
    QCOMPARE(spy.size(), 2);
    QCOMPARE(spy[1][0], QVariant(false));
}

void tst_QWebEnginePage::closeContents()
{
    TestPage page;
    QSignalSpy spyFinished(&page, &QWebEnginePage::loadFinished);
    QSignalSpy windowCreatedSpy(&page, &TestPage::windowCreated);
    page.setUrl(QUrl("about:blank"));
    QTRY_COMPARE(spyFinished.size(), 1);
    page.runJavaScript("var dialog = window.open('', '', 'width=100, height=100');");
    QTRY_COMPARE(windowCreatedSpy.size(), 1);

    QWebEngineView *dialogView = new QWebEngineView;
    QWebEnginePage *dialogPage = page.createdWindows[0];
    dialogView->setPage(dialogPage);
    QCOMPARE(dialogPage->lifecycleState(), QWebEnginePage::LifecycleState::Active);

    // This should not crash.
    connect(dialogPage, &QWebEnginePage::windowCloseRequested, dialogView, &QWebEngineView::close);
    page.runJavaScript("dialog.close();");

    // QWebEngineView::closeEvent() sets the life cycle state to discarded.
    QTRY_COMPARE(dialogPage->lifecycleState(), QWebEnginePage::LifecycleState::Discarded);
    delete dialogView;
}

// Based on QTBUG-84011
void tst_QWebEnginePage::isSafeRedirect_data()
{
    QTest::addColumn<QUrl>("requestedUrl");
    QTest::addColumn<QUrl>("expectedUrl");
    QString fileScheme = "file://";

#ifdef Q_OS_WIN
    fileScheme += "/";
#endif

    QString tempDir(fileScheme + QDir::tempPath() + "/");
    QTest::newRow(qPrintable(tempDir)) << QUrl(tempDir) << QUrl(tempDir);
    QTest::newRow(qPrintable(tempDir + QString("/foo/bar"))) << QUrl(tempDir + "/foo/bar") << QUrl(tempDir + "/foo/bar");
    QTest::newRow("filesystem:http://foo.com/bar") << QUrl("filesystem:http://foo.com/bar") << QUrl("filesystem:http://foo.com/bar/");
}

void tst_QWebEnginePage::isSafeRedirect()
{
    QFETCH(QUrl, requestedUrl);
    QFETCH(QUrl, expectedUrl);

    TestPage page;
    QSignalSpy spy(&page, SIGNAL(loadFinished(bool)));
    page.setUrl(requestedUrl);
    QTRY_COMPARE_WITH_TIMEOUT(spy.size(), 1, 20000);
    QCOMPARE(page.url(), expectedUrl);
    spy.clear();
}

class LocalRemoteUrlSchemeHandler : public QWebEngineUrlSchemeHandler
{
public:
    LocalRemoteUrlSchemeHandler(QObject *parent = nullptr)
        : QWebEngineUrlSchemeHandler(parent)
    {
    }
    ~LocalRemoteUrlSchemeHandler() = default;

    void requestStarted(QWebEngineUrlRequestJob *job) override
    {
        QBuffer *buffer = new QBuffer(job);
        buffer->setData("<html><body><a href='remote://test.html' id='link'>Click link</a></body></html>");
        job->reply("text/html", buffer);
        loaded = true;
    }
    bool loaded = false;
};

void tst_QWebEnginePage::localToRemoteNavigation()
{
    LocalRemoteUrlSchemeHandler local;
    LocalRemoteUrlSchemeHandler remote;
    QWebEngineProfile profile;
    profile.installUrlSchemeHandler("local", &local);
    profile.installUrlSchemeHandler("remote", &remote);

    QWebEnginePage page(&profile);
    QSignalSpy loadSpy(&page, SIGNAL(loadFinished(bool)));
    QWebEngineView view;
    view.resize(640, 480);
    view.show();
    view.setPage(&page);
    page.setUrl(QUrl("local://test.html"));
    QVERIFY(QTest::qWaitForWindowExposed(&view));
    QTRY_COMPARE_WITH_TIMEOUT(loadSpy.size(), 1, 20000);
    QVERIFY(local.loaded);

    // Should navigate:
    QTest::mouseClick(view.focusProxy(), Qt::LeftButton, {}, elementCenter(&page, "link"));
    QTRY_COMPARE_WITH_TIMEOUT(loadSpy.size(), 2, 20000);
    QVERIFY(remote.loaded);
    local.loaded = false;
    remote.loaded = false;

    page.setUrl(QUrl("local://test.html"));
    QTRY_COMPARE_WITH_TIMEOUT(loadSpy.size(), 3, 20000);
    QVERIFY(local.loaded && !remote.loaded);

    // Should not navigate:
    page.runJavaScript(QStringLiteral("document.getElementById(\"link\").click()"));
    QTest::qWait(500);
    QVERIFY(!remote.loaded);
}

void tst_QWebEnginePage::clientHints()
{
    HttpServer server;
    connect(&server, &HttpServer::newRequest, [&] (HttpReqRep *r) {
        r->setResponseBody(r->requestHeader("Sec-Ch-Ua-Platform"));
        r->sendResponse();
    });
    QVERIFY(server.start());

    QWebEnginePage page;
    QSignalSpy loadSpy(&page, SIGNAL(loadFinished(bool)));

    page.setUrl(server.url());
    QTRY_COMPARE(loadSpy.size(), 1);
    QVERIFY(loadSpy.takeFirst().value(0).toBool());

    QString platform = toPlainTextSync(&page);
#ifdef Q_OS_LINUX
    QCOMPARE(platform.toLower(), "\"linux\"");
#elif defined (Q_OS_MACOS)
    QCOMPARE(platform.toLower(), "\"macos\"");
#elif defined (Q_OS_WIN)
    QCOMPARE(platform.toLower(), "\"windows\"");
#endif

}

void tst_QWebEnginePage::childFrameInput()
{
    HttpServer server;
    server.setHostDomain("localhost");

    // The cross-origin policy blocks scripting this frame with QWebEnginePage::runJavaScript.
    // Use console messages to validate events.
    QString innerHtml(
            "<html><head><style>body{height:1200px;width:1200px;}</style></head><body>test<script>"
            "  let lastX, lastY = 0;"
            "  document.onscroll = (e) => {"
            "    if (window.scrollY > lastY) console.log(\"Down\");"
            "    if (window.scrollY < lastY) console.log(\"Up\");"
            "    if (window.scrollX > lastX) console.log(\"Right\");"
            "    if (window.scrollX < lastX) console.log(\"Left\");"
            "    lastX = window.scrollX;"
            "    lastY = window.scrollY;"
            "  };"
            "  window.onload = () => {console.log('loaded');};"
            "</script></body></html>");

    QVERIFY(server.start());
    connect(&server, &HttpServer::newRequest, [&](HttpReqRep *rr) {
        if (rr->requestPath() == "/main.html") {
            // the Origin-Agent-Cluster header enables dedicated processes for origins
            rr->setResponseHeader("Origin-Agent-Cluster", "?1");
            // the same-site-cross-origin page forces to create the frame in a different process
            server.setHostDomain("sub.localhost");
            rr->setResponseBody(("<html><body>"
                                 "<iframe id=\"iframe\" width=90% height=90% src=\""
                                 + server.url().toString().toUtf8()
                                 + "inner.html\"></iframe>"
                                   "</body></html>"));
        }
        if (rr->requestPath() == "/inner.html")
            rr->setResponseBody(innerHtml.toUtf8());
        rr->sendResponse();
    });

    QWebEngineView view;
    ConsolePage page;
    view.setPage(&page);
    view.resize(640, 480);
    QSignalSpy loadSpy(&page, &QWebEnginePage::loadFinished);
    page.load(server.url("/main.html"));
    QTRY_COMPARE_WITH_TIMEOUT(loadSpy.size(), 1, 20000);

    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));
    QTRY_VERIFY(evaluateJavaScriptSync(&page, "window.originAgentCluster").toBool());

    // make sure the frame is loaded
    QTRY_COMPARE(page.messages.size(), 1);
    QTRY_COMPARE(page.messages[0], QString("loaded"));

    // focus
    evaluateJavaScriptSync(&page, "document.getElementById('iframe').contentWindow.focus()");
    QTRY_COMPARE(evaluateJavaScriptSync(&page, "document.activeElement.id").toString(),
                 QStringLiteral("iframe"));

    QPoint globalPos = view.windowHandle()->position();
    QPoint p = elementCenter(&page, QString("iframe"));

    // Even if the document is loaded, it is not necessarily drawn.
    // Hit-testing (in Viz) for pointer events will be flacky in this scenario.
    // Send keyClick events first so the target frame will be cached for wheel events.
    QTest::keyClick(view.focusProxy(), Qt::Key_Down);
    QTRY_COMPARE(page.messages.size(), 2);
    QTRY_COMPARE(page.messages[1], QString("Down"));

    QTest::keyClick(view.focusProxy(), Qt::Key_Up);
    QTRY_COMPARE(page.messages.size(), 3);
    QTRY_COMPARE(page.messages[2], QString("Up"));

    QTest::keyClick(view.focusProxy(), Qt::Key_Right);
    QTRY_COMPARE(page.messages.size(), 4);
    QTRY_COMPARE(page.messages[3], QString("Right"));

    QTest::keyClick(view.focusProxy(), Qt::Key_Left);
    QTRY_COMPARE(page.messages.size(), 5);
    QTRY_COMPARE(page.messages[4], QString("Left"));

    makeScroll(view.focusProxy(), p, globalPos, QPoint(0, -120));
    QTRY_COMPARE(page.messages.size(), 6);
    QTRY_COMPARE(page.messages[5], QString("Down"));

    makeScroll(view.focusProxy(), p, globalPos, QPoint(0, 120));
    QTRY_COMPARE(page.messages.size(), 7);
    QTRY_COMPARE(page.messages[6], QString("Up"));

    makeScroll(view.focusProxy(), p, globalPos, QPoint(-120, 0));
    QTRY_COMPARE(page.messages.size(), 8);
    QTRY_COMPARE(page.messages[7], QString("Right"));

    makeScroll(view.focusProxy(), p, globalPos, QPoint(120, 0));
    QTRY_COMPARE(page.messages.size(), 9);
    QTRY_COMPARE(page.messages[8], QString("Left"));
}

void tst_QWebEnginePage::openLinkInNewPageWithWebWindowType_data()
{
    QTest::addColumn<QWebEnginePage::WebWindowType>("webWindowType");
    QTest::addColumn<QString>("elementId");
    QTest::addColumn<Qt::MouseButton>("button");
    QTest::addColumn<Qt::KeyboardModifier>("keyboardModififer");
    QTest::newRow("webBrowserWindow")
            << QWebEnginePage::WebBrowserWindow << "link" << Qt::LeftButton << Qt::ShiftModifier;
    QTest::newRow("webBrowserTab")
            << QWebEnginePage::WebBrowserTab << "link" << Qt::LeftButton << Qt::NoModifier;
    QTest::newRow("webDialog") << QWebEnginePage::WebDialog << "openWindow" << Qt::LeftButton
                               << Qt::NoModifier;
    QTest::newRow("webBrowserBackgroundTab") << QWebEnginePage::WebBrowserBackgroundTab << "link"
                                             << Qt::MiddleButton << Qt::NoModifier;
}

class WebWindowTypeTestPage : public QWebEnginePage
{
    Q_OBJECT

public:
    WebWindowType windowType;

signals:
    void windowCreated();

private:
    QWebEnginePage *createWindow(WebWindowType type) override
    {
        windowType = type;
        emit windowCreated();
        return nullptr;
    }
};

void tst_QWebEnginePage::openLinkInNewPageWithWebWindowType()
{
    QFETCH(QWebEnginePage::WebWindowType, webWindowType);
    QFETCH(QString, elementId);
    QFETCH(Qt::MouseButton, button);
    QFETCH(Qt::KeyboardModifier, keyboardModififer);

    WebWindowTypeTestPage page;
    QSignalSpy loadFinishedSpy(&page, SIGNAL(loadFinished(bool)));
    QSignalSpy windowCreatedSpy(&page, &WebWindowTypeTestPage::windowCreated);
    QWebEngineView view(&page);
    view.resize(640, 480);
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    page.settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    page.settings()->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, true);
    QString html = "<html><body>"
                   "<a id='link' href='hello' target='_blank'>link</a>"
                   "<br><br>"
                   "<button id='openWindow' onclick='myFunction()'>Try it</button>"
                   "<script>"
                   "function myFunction() {"
                   " const myWindow = window.open('', '', 'width=300,height=300');"
                   "}"
                   "</script>"
                   "</body></html>";

    page.setHtml(html);
    QVERIFY(loadFinishedSpy.wait());

    QTest::mouseClick(view.focusProxy(), button, keyboardModififer,
                      elementCenter(&page, elementId));
    QVERIFY(windowCreatedSpy.wait());
    QCOMPARE(page.windowType, webWindowType);
}

class DoNothingInterceptor : public QWebEngineUrlRequestInterceptor
{
public:
    DoNothingInterceptor() { }

    void interceptRequest(QWebEngineUrlRequestInfo &) override
    {
        ran = true;
    }
    bool ran = false;
};

void tst_QWebEnginePage::keepInterceptorAfterNewWindowRequested()
{
    DoNothingInterceptor interceptor;
    QWebEnginePage page;
    page.setUrlRequestInterceptor(&interceptor);
    connect(&page, &QWebEnginePage::newWindowRequested, [&](QWebEngineNewWindowRequest &request) {
        request.openIn(&page);
    });
    QSignalSpy loadFinishedSpy(&page, SIGNAL(loadFinished(bool)));

    QWebEngineView view;
    view.resize(500, 500);
    view.setPage(&page);
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    page.setHtml("<html><body>"
                  "<a id='link' href='hello' target='_blank'>link</a>"
                  "</body></html>");
    QTRY_COMPARE(loadFinishedSpy.size(), 1);
    QVERIFY(loadFinishedSpy.takeFirst().value(0).toBool());
    QVERIFY(interceptor.ran);
    interceptor.ran = false;

    QTest::mouseClick(view.focusProxy(), Qt::LeftButton, {}, elementCenter(&page, "link"));
    QTRY_COMPARE(loadFinishedSpy.size(), 1);
    QVERIFY(loadFinishedSpy.takeFirst().value(0).toBool());
    QVERIFY(!interceptor.ran);

    page.setHtml("<html><body></body></html>");
    QTRY_COMPARE(loadFinishedSpy.size(), 1);
    QVERIFY(loadFinishedSpy.takeFirst().value(0).toBool());
    QVERIFY(interceptor.ran);
}

static QByteArrayList params = {QByteArrayLiteral("--use-fake-device-for-media-stream")};
W_QTEST_MAIN(tst_QWebEnginePage, params)

#include "tst_qwebenginepage.moc"
