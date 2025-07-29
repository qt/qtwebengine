// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <util.h>
#include <QtTest/QtTest>
#include <QTemporaryDir>
#include <QtWebEngineCore/qwebengineprofile.h>
#include <QtWebEngineCore/qwebenginepage.h>
#include <QtWebEngineCore/qwebenginedownloadrequest.h>
#include <QtWebEngineCore/qwebengineextensionmanager.h>
#include <QtWebEngineCore/qwebengineextensioninfo.h>
#include <QtWebEngineWidgets/qwebengineview.h>
#include <QtWebEngineCore/qwebengineprofilebuilder.h>

#include <QDir>

using namespace Qt::StringLiterals;

class tst_QWebEngineExtension : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void cleanup();
    void cleanupTestCase();
    void init();
    void initTestCase();

private Q_SLOTS:
    void installExtension();
    void uninstallExtension();
    void loadExtension();
    void unloadExtension();
    void extensionSetEnabled();
    void reloadExtension();
    void installFailures();
    void uninstallOutsideFromProfileDir();
    void loadFailures();
    void actionPopupUrl();
    void loadInIncognito();
    void installInIncognito();
    void loadInstalledExtensions();
    void serviceWorkerMessaging();

private:
    int installedFiles();
    int extensionCount();
    QWebEngineExtensionInfo loadExtensionSync(const QString &path);
    void unloadExtensionSync(const QWebEngineExtensionInfo &extension);
    QWebEngineExtensionInfo installExtensionSync(const QString &path);
    void uninstallExtensionSync(const QWebEngineExtensionInfo &extension);
    QString resourcesPath();
    QDir extensionsInstallDir();

    QWebEnginePage *m_page;
    QWebEngineProfile *m_profile;
    QWebEngineExtensionManager *m_manager;
    QString m_resourcesPath;
};

int tst_QWebEngineExtension::installedFiles()
{
    return QDir(m_manager->installPath())
            .entryInfoList(QDir::AllEntries | QDir::NoDot | QDir::NoDotDot)
            .size();
}

int tst_QWebEngineExtension::extensionCount()
{
    return m_manager->extensions().size();
}

QWebEngineExtensionInfo tst_QWebEngineExtension::loadExtensionSync(const QString &path)
{
    QSignalSpy spy(m_manager, SIGNAL(extensionLoadFinished(QWebEngineExtensionInfo)));
    m_manager->loadExtension(path);
    spy.wait();
    if (spy.size() != 1) {
        qWarning("Did not receive loadFinished signal!");
        return {};
    }
    return spy.takeFirst().at(0).value<QWebEngineExtensionInfo>();
}

void tst_QWebEngineExtension::unloadExtensionSync(const QWebEngineExtensionInfo &extension)
{
    QSignalSpy spy(m_manager, SIGNAL(extensionUnloadFinished(QWebEngineExtensionInfo)));
    m_manager->unloadExtension(extension);
    QTRY_COMPARE(spy.size(), 1);
}

QWebEngineExtensionInfo tst_QWebEngineExtension::installExtensionSync(const QString &path)
{
    QSignalSpy spy(m_manager, SIGNAL(extensionInstallFinished(QWebEngineExtensionInfo)));
    m_manager->installExtension(path);
    spy.wait();
    if (spy.size() != 1) {
        qWarning("Did not receive installFinished signal!");
        return {};
    }
    return spy.takeFirst().at(0).value<QWebEngineExtensionInfo>();
}

void tst_QWebEngineExtension::uninstallExtensionSync(const QWebEngineExtensionInfo &extension)
{
    QSignalSpy spy(m_manager, SIGNAL(extensionUninstallFinished(QWebEngineExtensionInfo)));
    m_manager->uninstallExtension(extension);
    QTRY_COMPARE(spy.size(), 1);
}

QString tst_QWebEngineExtension::resourcesPath()
{
    return m_resourcesPath;
}

QDir tst_QWebEngineExtension::extensionsInstallDir()
{
    QString path = m_manager->installPath();
    return QDir(path);
}

void tst_QWebEngineExtension::cleanup()
{
    QVERIFY(QDir(m_manager->installPath()).removeRecursively());
    QCOMPARE(installedFiles(), 0);
    for (auto extension : m_manager->extensions())
        m_manager->unloadExtension(extension);
}

void tst_QWebEngineExtension::cleanupTestCase()
{
    delete m_page;
}

void tst_QWebEngineExtension::init() { }

void tst_QWebEngineExtension::initTestCase()
{
    QTemporaryDir tempDir(QDir::tempPath() + u"tst_QWebEngineExtension-XXXXXX");
    QWebEngineProfileBuilder profileBuilder;
    profileBuilder.setPersistentStoragePath(tempDir.path());
    m_profile = profileBuilder.createProfile("Test");
    m_page = new QWebEnginePage(m_profile);
    m_manager = m_profile->extensionManager();

    m_resourcesPath = QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()
            + u"/resources/"_s;
}

void tst_QWebEngineExtension::installExtension()
{
    int lastExtensionCount = extensionCount();
    QWebEngineExtensionInfo packedExtension =
            installExtensionSync(resourcesPath() + u"packed_ext.zip");
    QVERIFY2(packedExtension.isLoaded(), qPrintable(packedExtension.error()));
    QVERIFY2(packedExtension.isInstalled(), qPrintable(packedExtension.error()));
    QCOMPARE(installedFiles(), 1);
    QCOMPARE(extensionCount(), ++lastExtensionCount);

    QWebEngineExtensionInfo unpackedExtension =
            installExtensionSync(resourcesPath() + u"unpacked_ext");
    QVERIFY2(unpackedExtension.isLoaded(), qPrintable(unpackedExtension.error()));
    QVERIFY2(unpackedExtension.isInstalled(), qPrintable(unpackedExtension.error()));
    QCOMPARE(installedFiles(), 2);
    QCOMPARE(extensionCount(), ++lastExtensionCount);
}

void tst_QWebEngineExtension::uninstallExtension()
{
    QCOMPARE(installedFiles(), 0);
    int lastExtensionCount = extensionCount();
    QWebEngineExtensionInfo packedExtension =
            installExtensionSync(resourcesPath() + u"packed_ext.zip");
    uninstallExtensionSync(packedExtension);
    QCOMPARE(installedFiles(), 0);
    QCOMPARE(extensionCount(), lastExtensionCount);

    QWebEngineExtensionInfo unpackedExtension =
            installExtensionSync(resourcesPath() + u"unpacked_ext");
    uninstallExtensionSync(unpackedExtension);
    QCOMPARE(installedFiles(), 0);
    QCOMPARE(extensionCount(), lastExtensionCount);
}

void tst_QWebEngineExtension::loadExtension()
{
    int lastExtensionCount = extensionCount();
    QWebEngineExtensionInfo extension = loadExtensionSync(resourcesPath() + u"unpacked_ext");
    QVERIFY2(extension.isLoaded(), qPrintable(extension.error()));
    QVERIFY(!extension.isInstalled());
    QCOMPARE(extensionCount(), ++lastExtensionCount);
    QCOMPARE(installedFiles(), 0);
}

void tst_QWebEngineExtension::unloadExtension()
{
    int lastExtensionCount = extensionCount();
    QWebEngineExtensionInfo extension = loadExtensionSync(resourcesPath() + u"unpacked_ext");
    QVERIFY2(extension.isLoaded(), qPrintable(extension.error()));
    unloadExtensionSync(extension);
    QCOMPARE(extensionCount(), lastExtensionCount);
}

void tst_QWebEngineExtension::reloadExtension()
{
    QString path = resourcesPath() + u"unpacked_ext";
    int lastExtensionCount = extensionCount();
    QWebEngineExtensionInfo extension = loadExtensionSync(path);
    QVERIFY2(extension.isLoaded(), qPrintable(extension.error()));
    QCOMPARE(extensionCount(), ++lastExtensionCount);
    extension = loadExtensionSync(path);
    QVERIFY2(extension.isLoaded(), qPrintable(extension.error()));
    // Loading from the same path acts as a reload
    QCOMPARE(extensionCount(), lastExtensionCount);
}

void tst_QWebEngineExtension::extensionSetEnabled()
{
    QString contentScript = resourcesPath() + u"content_script_ext";
    QWebEngineExtensionInfo extension = loadExtensionSync(contentScript);
    QVERIFY2(extension.isLoaded(), qPrintable(extension.error()));
    QVERIFY(!extension.isEnabled());

    QSignalSpy loadSpy(m_page, SIGNAL(loadFinished(bool)));
    m_page->load(QUrl("qrc:///resources/index.html"));
    QTRY_COMPARE(loadSpy.size(), 1);
    QCOMPARE(evaluateJavaScriptSync(m_page, "document.body.childElementCount"), 0);
    m_manager->setExtensionEnabled(extension, true);
    QVERIFY(extension.isEnabled());
    m_page->triggerAction(QWebEnginePage::Reload);
    QTRY_COMPARE(loadSpy.size(), 2);
    QCOMPARE(evaluateJavaScriptSync(m_page, "document.body.childElementCount"), 1);

    m_manager->setExtensionEnabled(extension, false);
    QVERIFY(!extension.isEnabled());
    m_page->triggerAction(QWebEnginePage::Reload);
    QTRY_COMPARE(loadSpy.size(), 3);
    QCOMPARE(evaluateJavaScriptSync(m_page, "document.body.childElementCount"), 0);
}

void tst_QWebEngineExtension::installFailures()
{
    QCOMPARE(installedFiles(), 0);
    QWebEngineExtensionInfo extension =
            installExtensionSync(resourcesPath() + u"invalid_manifest_packed.zip");
    QVERIFY(!extension.isLoaded());
    QVERIFY(!extension.error().isEmpty());
    QTRY_COMPARE(installedFiles(), 0);

    extension = installExtensionSync(u"invalid_path"_s);
    QVERIFY(!extension.isLoaded());
    QVERIFY(!extension.error().isEmpty());
    QTRY_COMPARE(installedFiles(), 0);

    extension = installExtensionSync(resourcesPath() + u"non_existent.zip");
    QVERIFY(!extension.isLoaded());
    QVERIFY(!extension.error().isEmpty());
    QTRY_COMPARE(installedFiles(), 0);

    extension = installExtensionSync(resourcesPath() + u"invalid_manifest_ext");
    QVERIFY(!extension.isLoaded());
    QVERIFY(!extension.error().isEmpty());
    QTRY_COMPARE(installedFiles(), 0);
}

void tst_QWebEngineExtension::uninstallOutsideFromProfileDir()
{
    QString path = resourcesPath() + u"unpacked_ext";
    QVERIFY(QDir(path).exists());
    QWebEngineExtensionInfo extension = loadExtensionSync(path);
    QVERIFY2(extension.isLoaded(), qPrintable(extension.error()));
    QVERIFY(extension.error().isEmpty());
    QObject::connect(
            m_manager, &QWebEngineExtensionManager::extensionUninstallFinished,
            [](QWebEngineExtensionInfo extension) { QVERIFY(!extension.error().isEmpty()); });
    uninstallExtensionSync(extension);
    QVERIFY(QDir(path).exists());
}

void tst_QWebEngineExtension::loadFailures()
{
    int lastExtensionCount = extensionCount();
    QWebEngineExtensionInfo extension = loadExtensionSync(u"invalid_path"_s);
    QVERIFY(!extension.isLoaded());
    QVERIFY(!extension.error().isEmpty());
    QCOMPARE(extensionCount(), lastExtensionCount);

    extension = loadExtensionSync(resourcesPath());
    QVERIFY(!extension.isLoaded());
    QVERIFY(!extension.error().isEmpty());
    QCOMPARE(extensionCount(), lastExtensionCount);

    extension = loadExtensionSync(resourcesPath() + u"invalud_manifest");
    QVERIFY(!extension.isLoaded());
    QVERIFY(!extension.error().isEmpty());
    QCOMPARE(extensionCount(), lastExtensionCount);
}

void tst_QWebEngineExtension::actionPopupUrl()
{
    QWebEngineExtensionInfo extension = loadExtensionSync(resourcesPath() + u"unpacked_ext");
    QVERIFY2(extension.isLoaded(), qPrintable(extension.error()));
    QVERIFY(extension.actionPopupUrl().isEmpty());

    extension = loadExtensionSync(resourcesPath() + u"action_popup_ext");
    QVERIFY2(extension.isLoaded(), qPrintable(extension.error()));
    QVERIFY(!extension.actionPopupUrl().isEmpty());
}

void tst_QWebEngineExtension::loadInIncognito()
{
    QWebEngineProfile profile;
    QWebEnginePage page(&profile);
    QWebEngineExtensionManager *manager = profile.extensionManager();
    QSignalSpy spy(manager, SIGNAL(extensionLoadFinished(QWebEngineExtensionInfo)));
    manager->loadExtension(resourcesPath() + u"content_script_ext");
    QTRY_COMPARE(spy.size(), 1);
    auto extension = spy.takeFirst().at(0).value<QWebEngineExtensionInfo>();
    QVERIFY(!extension.isLoaded());
    QVERIFY(!extension.error().isEmpty());
}

void tst_QWebEngineExtension::installInIncognito()
{
    QWebEngineProfile profile;
    QWebEnginePage page(&profile);
    QWebEngineExtensionManager *manager = profile.extensionManager();
    QSignalSpy spy(manager, SIGNAL(extensionInstallFinished(QWebEngineExtensionInfo)));
    manager->installExtension(resourcesPath() + u"packed_ext.zip");
    QTRY_COMPARE(spy.size(), 1);
    auto extension = spy.takeFirst().at(0).value<QWebEngineExtensionInfo>();
    QVERIFY(!extension.isLoaded());
    QVERIFY(!extension.isInstalled());
    QVERIFY(!extension.error().isEmpty());
}

void tst_QWebEngineExtension::loadInstalledExtensions()
{
    QTemporaryDir tempDir;
    QWebEngineProfileBuilder profileBuilder;
    profileBuilder.setPersistentStoragePath(tempDir.path());
    QWebEngineProfile *profile = profileBuilder.createProfile("Test");
    QWebEngineExtensionManager *manager = profile->extensionManager();

    QSignalSpy spy(manager, SIGNAL(extensionInstallFinished(QWebEngineExtensionInfo)));
    manager->installExtension(resourcesPath() + u"packed_ext.zip");
    QTRY_COMPARE(spy.size(), 1);
    auto extension = spy.takeFirst().at(0).value<QWebEngineExtensionInfo>();
    QVERIFY2(extension.isLoaded(), qPrintable(extension.error()));

    int extensionCount = manager->extensions().size();

    // recreate the profile to verify installed extensions are loaded at start
    delete profile;
    profile = profileBuilder.createProfile("Test");
    auto manager2 = profile->extensionManager();
    QTRY_COMPARE(manager2->extensions().size(), extensionCount);
}

void tst_QWebEngineExtension::serviceWorkerMessaging()
{
    int lastExtensionCount = extensionCount();
    QWebEngineExtensionInfo extension = loadExtensionSync(resourcesPath() + u"service_worker_ext");
    QVERIFY(extension.isLoaded());
    m_manager->setExtensionEnabled(extension, true);
    QCOMPARE(extensionCount(), ++lastExtensionCount);
    QCOMPARE(installedFiles(), 0);

    QSignalSpy loadSpy(m_page, SIGNAL(loadFinished(bool)));
    m_page->load(QUrl("qrc:///resources/index.html"));
    QTRY_COMPARE(loadSpy.size(), 1);
    QTRY_COMPARE(evaluateJavaScriptSync(m_page, "document.body.childElementCount"), 1);
}

QTEST_MAIN(tst_QWebEngineExtension)
#include "tst_qwebengineextension.moc"
