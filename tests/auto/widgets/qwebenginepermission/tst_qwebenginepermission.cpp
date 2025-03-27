// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <util.h>

#include <QtTest/QtTest>
#include <QDir>
#include <QStringLiteral>
#include <QWebEngineDesktopMediaRequest>
#include <QWebEngineFrame>
#include <QWebEnginePage>
#include <QWebEnginePermission>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QWebEngineView>

using namespace Qt::StringLiterals;

class tst_QWebEnginePermission : public QObject
{
    Q_OBJECT

public:
    tst_QWebEnginePermission();
    ~tst_QWebEnginePermission();

public Q_SLOTS:
    void init();
    void cleanup();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void triggerFromJavascript_data();
    void triggerFromJavascript();
    void preGrant_data();
    void preGrant();
    void iframe_data();
    void iframe();

    void permissionPersistence_data();
    void permissionPersistence();

    void queryPermission_data();
    void queryPermission();
    void listPermissions();

    void clipboardReadWritePermissionInitialState_data();
    void clipboardReadWritePermissionInitialState();
    void clipboardReadWritePermission_data();
    void clipboardReadWritePermission();

private:
    std::unique_ptr<QWebEngineProfile> m_profile;
    QString m_profileName;
};

tst_QWebEnginePermission::tst_QWebEnginePermission()
    : m_profileName("tst_QWebEnginePermission")
{
}

tst_QWebEnginePermission::~tst_QWebEnginePermission()
{
}

void tst_QWebEnginePermission::initTestCase()
{
}

void tst_QWebEnginePermission::cleanupTestCase()
{
}

void tst_QWebEnginePermission::init()
{
    m_profile.reset(new QWebEngineProfile("tst_QWebEnginePermission"));
}

void tst_QWebEnginePermission::cleanup()
{
    if (m_profile && m_profile->persistentPermissionsPolicy()
            == QWebEngineProfile::PersistentPermissionsPolicy::StoreOnDisk) {
        QDir dir(m_profile->persistentStoragePath());
        dir.remove("permissions.json");

        // Set a persistent permission to force the creation of a permission.json
        // in test cases where it wouldn't be created otherwise
        m_profile->queryPermission(QUrl("https://google.com"),
            QWebEnginePermission::PermissionType::Notifications).grant();

        // This will trigger the writing of permissions to disk
        m_profile.reset();

        // Wait for the new permissions.json to be written to disk before deleting
        QTRY_VERIFY_WITH_TIMEOUT(dir.exists("permissions.json"), 5000);
        dir.remove("permissions.json");
    } else {
        m_profile.reset();
    }
}

static QString MediaAudioCapture_trigger =
    "navigator.mediaDevices.getUserMedia({ video: false, audio: true }).then(s => { data = s; done = true; })"
    ".catch(err => { skipReason = err.message; done = true; });"_L1;
static QString MediaAudioCapture_check =
    "return data != undefined;"_L1;

static QString MediaVideoCapture_trigger =
    "navigator.mediaDevices.getUserMedia({ video: true, audio: false }).then(s => { data = s; done = true; })"
    ".catch(err => { skipReason = err.message; done = true; });"_L1;
static QString MediaVideoCapture_check =
    "return data != undefined;"_L1;

static QString MediaAudioVideoCapture_trigger =
    "navigator.mediaDevices.getUserMedia({ video: true, audio: true }).then(s => { data = s; done = true; })"
    ".catch(err => { skipReason = err.message; done = true; });"_L1;
static QString MediaAudioVideoCapture_check =
    "return data != undefined;"_L1;

static QString DesktopVideoCapture_trigger =
    "navigator.mediaDevices.getDisplayMedia({ video: true, audio: false }).then(s => { data = s; done = true; })"
    ".catch(err => { skipReason = err.message; done = true; });"_L1;
static QString DesktopVideoCapture_check =
    "return data != undefined;"_L1;

static QString DesktopAudioVideoCapture_trigger =
    "navigator.mediaDevices.getDisplayMedia({ video: true, audio: true }).then(s => { data = s; done = true; })"
    ".catch(err => { skipReason = err.message; done = true; });"_L1;
static QString DesktopAudioVideoCapture_check =
    "return data != undefined;"_L1;

static QString MouseLock_trigger =
    "document.documentElement.requestPointerLock().then(() => { data = document.pointerLockElement(); done = true; }).catch(() => { done = true; });"_L1;
static QString MouseLock_check =
    "var ret = (data != undefined); document.exitPointerLock(); return ret;"_L1;

static QString Notifications_trigger =
    "Notification.requestPermission().then(p => { data = p; done = true; }).catch(() => { done = true; });"_L1;
static QString Notifications_check =
    "return data != undefined && Notification.permission === 'granted';"_L1;

static QString Geolocation_trigger =
    "success = function(p) { data = p; done = true; };"
    "failure = function(err) { if (err.code === 2) skipReason = 'Positioning is unavailable'; done = true; };"
    "navigator.geolocation.getCurrentPosition(success, failure);"_L1;
static QString Geolocation_check =
    "return data != undefined;"_L1;

static QString ClipboardReadWrite_trigger =
    "navigator.clipboard.readText().then(c => { data = c; done = true; }).catch(() => { done = true; });"_L1;
static QString ClipboardReadWrite_check =
    "return data != undefined;"_L1;

static QString LocalFontsAccess_trigger =
    "if (!window.queryLocalFonts) { skipReason = 'Local fonts access is not supported on this system'; done = true; }"
    "else { window.queryLocalFonts().then(f => { data = f; done = true; }); };"_L1;
static QString LocalFontsAccess_check =
    "return data.length != 0;"_L1;

static void commonTestData()
{
    QTest::addColumn<QWebEnginePermission::PermissionType>("permissionType");
    QTest::addColumn<QString>("triggerFunction");
    QTest::addColumn<QString>("testFunction");
    QTest::addColumn<QWebEngineProfile::PersistentPermissionsPolicy>("policy");

#define QWebEnginePermissionTestCase(pt) \
    QTest::newRow(#pt "_AskEveryTime")                                      \
        << QWebEnginePermission::PermissionType::pt                         \
        << pt ## _trigger << pt ## _check                                   \
        << QWebEngineProfile::PersistentPermissionsPolicy::AskEveryTime;    \
    QTest::newRow(#pt "_StoreInMemory")                                     \
        << QWebEnginePermission::PermissionType::pt                         \
        << pt ## _trigger << pt ## _check                                   \
        << QWebEngineProfile::PersistentPermissionsPolicy::StoreInMemory;   \
    QTest::newRow(#pt "_StoreOnDisk")                                       \
        << QWebEnginePermission::PermissionType::pt                         \
        << pt ## _trigger << pt ## _check                                   \
        << QWebEngineProfile::PersistentPermissionsPolicy::StoreOnDisk;

    QWebEnginePermissionTestCase(MediaAudioCapture);

    // Video capture tests don't work with offscreen
    if (QGuiApplication::platformName() != QLatin1String("offscreen")) {
        QWebEnginePermissionTestCase(MediaVideoCapture);
        QWebEnginePermissionTestCase(MediaAudioVideoCapture);
        QWebEnginePermissionTestCase(DesktopVideoCapture);
        QWebEnginePermissionTestCase(DesktopAudioVideoCapture);
    }
    // QWebEnginePermissionTestCase(MouseLock); // currently untestable
    QWebEnginePermissionTestCase(Notifications);
#ifndef Q_OS_MACOS
    QWebEnginePermissionTestCase(Geolocation);
#endif
    QWebEnginePermissionTestCase(ClipboardReadWrite);
    QWebEnginePermissionTestCase(LocalFontsAccess);

#undef QWebEnginePermissionTestCase
}

void tst_QWebEnginePermission::triggerFromJavascript_data()
{
    commonTestData();
}

void tst_QWebEnginePermission::triggerFromJavascript()
{
    QFETCH(QWebEnginePermission::PermissionType, permissionType);
    QFETCH(QString, triggerFunction);
    QFETCH(QString, testFunction);
    QFETCH(QWebEngineProfile::PersistentPermissionsPolicy, policy);

    QWebEngineView view;
    QWebEnginePage page(m_profile.get(), &view);
    m_profile->setPersistentPermissionsPolicy(policy);
    view.setPage(&page);

    page.settings()->setAttribute(QWebEngineSettings::ScreenCaptureEnabled, true);
    page.settings()->setAttribute(QWebEngineSettings::JavascriptCanAccessClipboard, true);
    connect(&page, &QWebEnginePage::desktopMediaRequested, &page, [&](const QWebEngineDesktopMediaRequest &request) {
        request.selectScreen(request.screensModel()->index(0));
    });

    bool grant = true;
    QWebEnginePermission permission;
    connect(&page, &QWebEnginePage::permissionRequested, &page, [&](QWebEnginePermission p) {
        QCOMPARE(p.permissionType(), permissionType);
        grant ? p.grant() : p.deny();
        permission = p;
    });

    QSignalSpy spy(&page, &QWebEnginePage::loadFinished);
    page.load(QUrl("qrc:///resources/index.html"));
    QTRY_COMPARE(spy.size(), 1);

    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    evaluateJavaScriptSync(&page, "triggerFunc = function() {"_L1 + triggerFunction + "}"_L1);
    evaluateJavaScriptSync(&page, "testFunc = function() {"_L1 + testFunction + "done = true;" + "}"_L1);

    // Access to some pf the APIs requires recent user interaction
    QTest::mouseClick(view.focusProxy(), Qt::LeftButton, {}, QPoint{100, 100});

    QTRY_VERIFY_WITH_TIMEOUT(evaluateJavaScriptSync(&page, QStringLiteral("done")).toBool(), 5000);
    if (evaluateJavaScriptSync(&page, QStringLiteral("skipReason")).toBool()) {
        // Catch expected failures and skip test
        QSKIP(("Skipping test. Reason: " + evaluateJavaScriptSync(&page, QStringLiteral("skipReason")).toString()).toStdString().c_str());
    }
    qWarning() << evaluateJavaScriptSync(&page, QStringLiteral("data"));

    QVERIFY(evaluateJavaScriptSync(&page, QStringLiteral("testFunc()")).toBool());
    QCOMPARE(permission.state(), QWebEnginePermission::State::Granted);

    // Now reset the permission, and try denying it
    permission.reset();
    QCOMPARE(permission.state(), QWebEnginePermission::State::Ask);
    evaluateJavaScriptSync(&page, "done = false; data = undefined"_L1);
    grant = false;

    QTest::mouseClick(view.focusProxy(), Qt::LeftButton, {}, QPoint{100, 100});

    QTRY_VERIFY_WITH_TIMEOUT(evaluateJavaScriptSync(&page, QStringLiteral("done")).toBool(), 5000);
    QCOMPARE(evaluateJavaScriptSync(&page, QStringLiteral("testFunc()")).toBool(), false);
    QCOMPARE(permission.state(), QWebEnginePermission::State::Denied);
}

void tst_QWebEnginePermission::preGrant_data()
{
    commonTestData();
}

void tst_QWebEnginePermission::preGrant()
{
    QFETCH(QWebEnginePermission::PermissionType, permissionType);
    QFETCH(QString, triggerFunction);
    QFETCH(QString, testFunction);
    QFETCH(QWebEngineProfile::PersistentPermissionsPolicy, policy);

    QWebEngineView view;
    QWebEnginePage page(m_profile.get(), &view);
    m_profile->setPersistentPermissionsPolicy(policy);
    view.setPage(&page);

    QSignalSpy loadSpy(&page, &QWebEnginePage::loadFinished);
    page.load(QUrl("qrc:///resources/index.html"));
    QTRY_COMPARE(loadSpy.size(), 1);

    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    page.settings()->setAttribute(QWebEngineSettings::ScreenCaptureEnabled, true);
    page.settings()->setAttribute(QWebEngineSettings::JavascriptCanAccessClipboard, true);
    connect(&page, &QWebEnginePage::desktopMediaRequested, &page, [&](const QWebEngineDesktopMediaRequest &request) {
        request.selectScreen(request.screensModel()->index(0));
    });

    QWebEnginePermission permission = m_profile->queryPermission(page.url(), permissionType);
    QVERIFY(permission.state() == QWebEnginePermission::State::Ask);
    permission.grant();

    evaluateJavaScriptSync(&page, "triggerFunc = function() {"_L1 + triggerFunction + "}"_L1);
    evaluateJavaScriptSync(&page, "testFunc = function() {"_L1 + testFunction + "done = true;" + "}"_L1);

    QSignalSpy spy(&page, &QWebEnginePage::permissionRequested);

    QTest::mouseClick(view.focusProxy(), Qt::LeftButton, {}, QPoint{100, 100});
    QTRY_VERIFY_WITH_TIMEOUT(evaluateJavaScriptSync(&page, QStringLiteral("done")).toBool(), 5000);
    if (evaluateJavaScriptSync(&page, QStringLiteral("skipReason")).toBool()) {
        // No media devices, or no geolocation plugin
        QSKIP(("Skipping test. Reason: " + evaluateJavaScriptSync(&page, QStringLiteral("skipReason")).toString()).toStdString().c_str());
    }
    QVERIFY(evaluateJavaScriptSync(&page, QStringLiteral("testFunc()")).toBool());

    // The permissionRequested signal must NOT fire
    QCOMPARE(spy.size(), 0);
}

void tst_QWebEnginePermission::iframe_data()
{
    commonTestData();
}

void tst_QWebEnginePermission::iframe()
{
    QFETCH(QWebEnginePermission::PermissionType, permissionType);
    QFETCH(QString, triggerFunction);
    QFETCH(QString, testFunction);
    QFETCH(QWebEngineProfile::PersistentPermissionsPolicy, policy);

    QWebEngineView view;
    QWebEnginePage page(m_profile.get(), &view);
    m_profile->setPersistentPermissionsPolicy(policy);
    view.setPage(&page);

    page.settings()->setAttribute(QWebEngineSettings::ScreenCaptureEnabled, true);
    page.settings()->setAttribute(QWebEngineSettings::JavascriptCanAccessClipboard, true);
    connect(&page, &QWebEnginePage::desktopMediaRequested, &page, [&](const QWebEngineDesktopMediaRequest &request) {
        request.selectScreen(request.screensModel()->index(0));
    });

    bool grant = true;
    QWebEnginePermission permission;
    connect(&page, &QWebEnginePage::permissionRequested, &page, [&](QWebEnginePermission p) {
        grant ? p.grant() : p.deny();
        permission = p;
    });

    QSignalSpy loadSpy(&page, &QWebEnginePage::loadFinished);
    page.load(QUrl("qrc:///resources/iframe.html"));
    QTRY_COMPARE(loadSpy.size(), 1);

    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    auto maybeFrame = page.findFrameByName("frame");
    QVERIFY(maybeFrame);
    QWebEngineFrame &frame = maybeFrame.value();

    evaluateJavaScriptSync(&frame, "triggerFunc = function() {"_L1 + triggerFunction + "}"_L1);
    evaluateJavaScriptSync(&frame, "testFunc = function() {"_L1 + testFunction + "done = true;" + "}"_L1);

    QTest::mouseClick(view.focusProxy(), Qt::LeftButton, {}, QPoint{100, 100});

    QTRY_VERIFY_WITH_TIMEOUT(evaluateJavaScriptSync(&frame, QStringLiteral("done")).toBool(), 10000);
    if (evaluateJavaScriptSync(&frame, QStringLiteral("skipReason")).toBool()) {
        // Catch expected failures and skip test
        QSKIP(("Skipping test. Reason: " + evaluateJavaScriptSync(&frame, QStringLiteral("skipReason")).toString()).toStdString().c_str());
    }

    QVERIFY(evaluateJavaScriptSync(&frame, QStringLiteral("testFunc()")).toBool());
    QCOMPARE(permission.state(), QWebEnginePermission::State::Granted);

    // Now reset the permission, and try denying it
    permission.reset();
    QCOMPARE(permission.state(), QWebEnginePermission::State::Ask);
    evaluateJavaScriptSync(&frame, "done = false; data = undefined"_L1);
    grant = false;

    // Only test non-persistent permissions past this point
    if (QWebEnginePermission::isPersistent(permissionType)
            && policy != QWebEngineProfile::PersistentPermissionsPolicy::AskEveryTime)
        return;

    // Perform a cross-origin navigation and then go back to check if the permission has been cleared
    // We don't need a valid URL to trigger the cross-origin logic.
    evaluateJavaScriptSync(&page, "document.getElementsByName('frame')[0].src = 'http://bad-url.bad-url'"_L1);
    QTRY_VERIFY_WITH_TIMEOUT(frame.url() != QUrl("qrc:///resources/index.html"_L1), 10000);
    evaluateJavaScriptSync(&page, "document.getElementsByName('frame')[0].src = 'qrc:///resources/index.html'"_L1);
    QTRY_VERIFY_WITH_TIMEOUT(frame.url() == QUrl("qrc:///resources/index.html"_L1), 10000);

    QCOMPARE(permission.state(), QWebEnginePermission::State::Ask);
}

void tst_QWebEnginePermission::permissionPersistence_data()
{
    QTest::addColumn<QWebEngineProfile::PersistentPermissionsPolicy>("policy");
    QTest::addColumn<bool>("granted");

    QTest::newRow("noPersistenceDeny")      << QWebEngineProfile::PersistentPermissionsPolicy::AskEveryTime  << false;
    QTest::newRow("noPersistenceGrant")     << QWebEngineProfile::PersistentPermissionsPolicy::AskEveryTime  << true;
    QTest::newRow("memoryPersistenceDeny")  << QWebEngineProfile::PersistentPermissionsPolicy::StoreInMemory << false;
    QTest::newRow("memoryPersistenceGrant") << QWebEngineProfile::PersistentPermissionsPolicy::StoreInMemory << true;
    QTest::newRow("diskPersistenceDeny")    << QWebEngineProfile::PersistentPermissionsPolicy::StoreOnDisk   << false;
    QTest::newRow("diskPersistenceGrant")   << QWebEngineProfile::PersistentPermissionsPolicy::StoreOnDisk   << true;
}

void tst_QWebEnginePermission::permissionPersistence()
{
    QFETCH(QWebEngineProfile::PersistentPermissionsPolicy, policy);
    QFETCH(bool, granted);

    m_profile->setPersistentPermissionsPolicy(policy);

    std::unique_ptr<QWebEnginePage> page(new QWebEnginePage(m_profile.get()));
    std::unique_ptr<QSignalSpy> loadSpy(new QSignalSpy(page.get(), &QWebEnginePage::loadFinished));
    QDir storageDir = QDir(m_profile->persistentStoragePath());

    page->load(QUrl("qrc:///resources/index.html"_L1));
    QTRY_COMPARE(loadSpy->size(), 1);

    QVariant variant = granted ? "granted" : "denied";
    QVariant defaultVariant = "default";

    QWebEnginePermission permissionObject = m_profile->queryPermission(
        QUrl("qrc:///resources/index.html"_L1), QWebEnginePermission::PermissionType::Notifications);
    if (granted)
        permissionObject.grant();
    else
        permissionObject.deny();
    QCOMPARE(evaluateJavaScriptSync(page.get(), "Notification.permission"), variant);

    page.reset();
    m_profile.reset();
    loadSpy.reset();

    bool expectSame = false;
    if (policy == QWebEngineProfile::PersistentPermissionsPolicy::StoreOnDisk) {
        expectSame = true;

        // File is written asynchronously, wait for it to be created
        QTRY_COMPARE(storageDir.exists("permissions.json"), true);
    }

    m_profile.reset(new QWebEngineProfile(m_profileName));
    m_profile->setPersistentPermissionsPolicy(policy);

    page.reset(new QWebEnginePage(m_profile.get()));
    loadSpy.reset(new QSignalSpy(page.get(), &QWebEnginePage::loadFinished));
    page->load(QUrl("qrc:///resources/index.html"_L1));
    QTRY_COMPARE(loadSpy->size(), 1);
    QTRY_COMPARE(evaluateJavaScriptSync(page.get(), "Notification.permission"),
        expectSame ? variant : defaultVariant);

    // Re-acquire the permission, since deleting the Profile makes it invalid
    permissionObject = m_profile->queryPermission(QUrl("qrc:///resources/index.html"_L1), QWebEnginePermission::PermissionType::Notifications);
    permissionObject.reset();
    QCOMPARE(evaluateJavaScriptSync(page.get(), "Notification.permission"), defaultVariant);
}

void tst_QWebEnginePermission::queryPermission_data()
{
    QTest::addColumn<QWebEnginePermission::PermissionType>("permissionType");
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<bool>("expectedValid");

    QTest::newRow("badUrl")
        << QWebEnginePermission::PermissionType::Notifications << QUrl("//:bad-url"_L1)                << false;
    QTest::newRow("badFeature")
        << QWebEnginePermission::PermissionType::Unsupported   << QUrl("qrc:/resources/index.html"_L1) << false;
    QTest::newRow("transientFeature")
        << QWebEnginePermission::PermissionType::MouseLock     << QUrl("qrc:/resources/index.html"_L1) << true;
    QTest::newRow("good")
        << QWebEnginePermission::PermissionType::Notifications << QUrl("qrc:/resources/index.html"_L1) << true;
}

void tst_QWebEnginePermission::queryPermission()
{
    QFETCH(QWebEnginePermission::PermissionType, permissionType);
    QFETCH(QUrl, url);
    QFETCH(bool, expectedValid);

    // In-memory is the default for otr profiles
    m_profile.reset(new QWebEngineProfile());
    QVERIFY(m_profile->persistentPermissionsPolicy() == QWebEngineProfile::PersistentPermissionsPolicy::StoreInMemory);

    QWebEnginePermission permission = m_profile->queryPermission(url, permissionType);
    bool valid = permission.isValid();
    QCOMPARE(valid, expectedValid);
    if (!valid)
        QCOMPARE(permission.state(), QWebEnginePermission::State::Invalid);

    // Verify that we can grant a valid permission, and we can't grant an invalid one...
    permission.grant();
    QCOMPARE(permission.state(), valid ? QWebEnginePermission::State::Granted : QWebEnginePermission::State::Invalid);

    // ...and that doing so twice doesn't mess up the state...
    permission.grant();
    QCOMPARE(permission.state(), valid ? QWebEnginePermission::State::Granted : QWebEnginePermission::State::Invalid);

    // ...and that the same thing applies to denying them...
    permission.deny();
    QCOMPARE(permission.state(), valid ? QWebEnginePermission::State::Denied : QWebEnginePermission::State::Invalid);
    permission.deny();
    QCOMPARE(permission.state(), valid ? QWebEnginePermission::State::Denied : QWebEnginePermission::State::Invalid);

    // ...and that resetting works
    permission.reset();
    QCOMPARE(permission.state(), valid ? QWebEnginePermission::State::Ask : QWebEnginePermission::State::Invalid);
    permission.reset();
    QCOMPARE(permission.state(), valid ? QWebEnginePermission::State::Ask : QWebEnginePermission::State::Invalid);
}

void tst_QWebEnginePermission::listPermissions()
{
    // In-memory is the default for otr profiles
    m_profile.reset(new QWebEngineProfile());
    QVERIFY(m_profile->persistentPermissionsPolicy() == QWebEngineProfile::PersistentPermissionsPolicy::StoreInMemory);

    QUrl commonUrl = QUrl(QStringLiteral("https://www.bing.com/maps"));
    QWebEnginePermission::PermissionType commonType = QWebEnginePermission::PermissionType::Notifications;

    // First, set several permissions at once
    m_profile->queryPermission(commonUrl, QWebEnginePermission::PermissionType::Geolocation).deny();
    m_profile->queryPermission(commonUrl, QWebEnginePermission::PermissionType::Unsupported).grant(); // Invalid
    m_profile->queryPermission(commonUrl, commonType).grant();
    m_profile->queryPermission(QUrl(QStringLiteral("https://www.google.com/translate")), commonType).grant();

    QList<QWebEnginePermission> permissionsListAll = m_profile->listAllPermissions();
    QList<QWebEnginePermission> permissionsListUrl = m_profile->listPermissionsForOrigin(commonUrl);
    QList<QWebEnginePermission> permissionsListFeature = m_profile->listPermissionsForPermissionType(commonType);

    // Order of returned permissions is not guaranteed, so we must iterate until we find the one we need
    auto findInList = [](QList<QWebEnginePermission> list, const QUrl &url,
        QWebEnginePermission::PermissionType permissionType, QWebEnginePermission::State state)
    {
        bool found = false;
        for (auto &permission : list) {
            if (permission.origin().adjusted(QUrl::RemovePath) == url.adjusted(QUrl::RemovePath)
                    && permission.permissionType() == permissionType && permission.state() == state) {
                found = true;
                break;
            }
        }
        return found;
    };

    // Check full list
    QVERIFY(permissionsListAll.size() == 3);
    QVERIFY(findInList(permissionsListAll, commonUrl, QWebEnginePermission::PermissionType::Geolocation, QWebEnginePermission::State::Denied));
    QVERIFY(findInList(permissionsListAll, commonUrl, commonType, QWebEnginePermission::State::Granted));
    QVERIFY(findInList(permissionsListAll, QUrl(QStringLiteral("https://www.google.com")), commonType, QWebEnginePermission::State::Granted));

    // Check list filtered by URL
    QVERIFY(permissionsListUrl.size() == 2);
    QVERIFY(findInList(permissionsListUrl, commonUrl, QWebEnginePermission::PermissionType::Geolocation, QWebEnginePermission::State::Denied));
    QVERIFY(findInList(permissionsListAll, commonUrl, commonType, QWebEnginePermission::State::Granted));

    // Check list filtered by feature
    QVERIFY(permissionsListFeature.size() == 2);
    QVERIFY(findInList(permissionsListAll, commonUrl, commonType, QWebEnginePermission::State::Granted));
    QVERIFY(findInList(permissionsListAll, QUrl(QStringLiteral("https://www.google.com")), commonType, QWebEnginePermission::State::Granted));
}

static QString clipboardPermissionQuery(QString variableName, QString permissionName)
{
    return QString("var %1; navigator.permissions.query({ name:'%2' }).then((p) => { %1 = p.state; "
                   "});")
            .arg(variableName)
            .arg(permissionName);
}

void tst_QWebEnginePermission::clipboardReadWritePermissionInitialState_data()
{
    QTest::addColumn<bool>("canAccessClipboard");
    QTest::addColumn<bool>("canPaste");
    QTest::addColumn<QString>("readPermission");
    QTest::addColumn<QString>("writePermission");
    QTest::newRow("access and paste should grant both") << true << true << "granted" << "granted";
    QTest::newRow("paste only should prompt for both") << false << true << "prompt" << "prompt";
    QTest::newRow("access only should grant for write only")
            << true << false << "prompt" << "granted";
    QTest::newRow("no access or paste should prompt for both")
            << false << false << "prompt" << "prompt";
}

void tst_QWebEnginePermission::clipboardReadWritePermissionInitialState()
{
    QFETCH(bool, canAccessClipboard);
    QFETCH(bool, canPaste);
    QFETCH(QString, readPermission);
    QFETCH(QString, writePermission);

    m_profile->setPersistentPermissionsPolicy(QWebEngineProfile::PersistentPermissionsPolicy::AskEveryTime);
    QWebEngineView view(m_profile.get());
    QWebEnginePage &page = *view.page();
    view.settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, true);
    page.settings()->setAttribute(QWebEngineSettings::JavascriptCanAccessClipboard,
                                  canAccessClipboard);
    page.settings()->setAttribute(QWebEngineSettings::JavascriptCanPaste, canPaste);

    QSignalSpy spy(&page, &QWebEnginePage::loadFinished);
    QUrl baseUrl("https://www.example.com/somepage.html");
    page.setHtml(QString("<html><body>Test</body></html>"), baseUrl);
    QTRY_COMPARE(spy.size(), 1);

    evaluateJavaScriptSync(&page, clipboardPermissionQuery("readPermission", "clipboard-read"));
    QCOMPARE(evaluateJavaScriptSync(&page, QStringLiteral("readPermission")), readPermission);
    evaluateJavaScriptSync(&page, clipboardPermissionQuery("writePermission", "clipboard-write"));
    QCOMPARE(evaluateJavaScriptSync(&page, QStringLiteral("writePermission")), writePermission);
}

void tst_QWebEnginePermission::clipboardReadWritePermission_data()
{
    QTest::addColumn<bool>("canAccessClipboard");
    QTest::addColumn<QWebEnginePermission::State>("initialPolicy");
    QTest::addColumn<QString>("initialPermission");
    QTest::addColumn<QString>("finalPermission");

    QTest::newRow("noAccessGrant")
            << false << QWebEnginePermission::State::Granted << "granted" << "granted";
    QTest::newRow("noAccessDeny")
            << false << QWebEnginePermission::State::Denied  << "denied"  << "denied";
    QTest::newRow("noAccessAsk")
            << false << QWebEnginePermission::State::Ask     << "prompt"  << "granted";

    // All policies are ignored and overridden by setting JsCanAccessClipboard and JsCanPaste to
    // true
    QTest::newRow("accessGrant")
            << true << QWebEnginePermission::State::Granted << "granted"  << "granted";
    QTest::newRow("accessDeny")
            << true << QWebEnginePermission::State::Denied  << "granted"  << "granted";
    QTest::newRow("accessAsk")
            << true << QWebEnginePermission::State::Ask     << "granted"  << "granted";
}

void tst_QWebEnginePermission::clipboardReadWritePermission()
{
    QFETCH(bool, canAccessClipboard);
    QFETCH(QWebEnginePermission::State, initialPolicy);
    QFETCH(QString, initialPermission);
    QFETCH(QString, finalPermission);

    m_profile->setPersistentPermissionsPolicy(QWebEngineProfile::PersistentPermissionsPolicy::AskEveryTime);
    QWebEngineView view(m_profile.get());
    QWebEnginePage &page = *view.page();
    view.settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, true);
    page.settings()->setAttribute(QWebEngineSettings::JavascriptCanAccessClipboard,
                                  canAccessClipboard);
    page.settings()->setAttribute(QWebEngineSettings::JavascriptCanPaste, true);

    QUrl baseUrl("https://www.example.com/somepage.html");

    int permissionRequestCount = 0;
    bool errorState = false;

    // This should only fire in the noAccessAsk case. The other NoAccess cases will remember the initial permission,
    // and the Access cases will auto-grant because JavascriptCanPaste and JavascriptCanAccessClipboard are set.
    connect(&page, &QWebEnginePage::permissionRequested, &page,
            [&](QWebEnginePermission permission) {
                if (permission.permissionType() != QWebEnginePermission::PermissionType::ClipboardReadWrite)
                    return;
                if (permission.origin() != baseUrl.url(QUrl::RemoveFilename)) {
                    qWarning() << "Unexpected case. Can't proceed." << permission.origin();
                    errorState = true;
                    return;
                }
                permissionRequestCount++;
                // Deliberately set to the opposite state; we want to force a fail when this triggers
                if (initialPolicy == QWebEnginePermission::State::Granted)
                    permission.deny();
                else
                    permission.grant();
            });

    QWebEnginePermission permissionObject = m_profile->queryPermission(baseUrl, QWebEnginePermission::PermissionType::ClipboardReadWrite);
    switch (initialPolicy) {
    case QWebEnginePermission::State::Granted:
        permissionObject.grant();
        break;
    case QWebEnginePermission::State::Denied:
        permissionObject.deny();
        break;
    case QWebEnginePermission::State::Ask:
        permissionObject.reset();
        break;
    case QWebEnginePermission::State::Invalid:
        break;
    }

    QSignalSpy spy(&page, &QWebEnginePage::loadFinished);
    page.setHtml(QString("<html><body>Test</body></html>"), baseUrl);
    QTRY_COMPARE(spy.size(), 1);

    evaluateJavaScriptSync(&page, clipboardPermissionQuery("readPermission", "clipboard-read"));
    QCOMPARE(evaluateJavaScriptSync(&page, QStringLiteral("readPermission")), initialPermission);
    evaluateJavaScriptSync(&page, clipboardPermissionQuery("writePermission", "clipboard-write"));
    QCOMPARE(evaluateJavaScriptSync(&page, QStringLiteral("writePermission")), initialPermission);

    auto triggerRequest = [&page](QString variableName, QString apiCall)
    {
        auto js = QString("var %1; navigator.clipboard.%2.then((v) => { %1 = 'granted' }, (v) => { %1 = "
                "'denied' });")
            .arg(variableName)
            .arg(apiCall);
        evaluateJavaScriptSync(&page, js);
    };

    // Permission is remembered, and shouldn't trigger a new request when called from JS
    triggerRequest("readState", "readText()");
    QTRY_COMPARE(evaluateJavaScriptSync(&page, "readState"), finalPermission);
    triggerRequest("writeState", "writeText('foo')");
    QTRY_COMPARE(evaluateJavaScriptSync(&page, "writeState"), finalPermission);

    if (initialPermission != finalPermission) {
        QCOMPARE(permissionRequestCount, 1);
    } else {
        QCOMPARE(permissionRequestCount, 0);
    }

    QVERIFY(!errorState);
}

QTEST_MAIN(tst_QWebEnginePermission)
#include "tst_qwebenginepermission.moc"
