// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>
#include <QTemporaryDir>
#include <QtWebEngineCore/qwebengineprofilebuilder.h>

#if QT_CONFIG(ssl)
#include <httpsserver.h>
#include <util.h>
#include <QtTest/private/qtesthelpers_p.h>
#include <QtWebEngineCore/qwebenginecertificateerror.h>
#include <QtWebEngineCore/qwebengineclientcertificatestore.h>
#include <QtWebEngineCore/qwebenginesettings.h>
#endif

class tst_QWebEngineProfileBuilder : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void offTheRecordProfile();
    void diskBasedProfile();
    void persistentStoragePath();
    void cachePath();
    void httpCacheType_data();
    void httpCacheType();
    void persistentCookiesPolicy_data();
    void persistentCookiesPolicy();
    void httpCacheSize();
    void persistentPermissionsPolicy_data();
    void persistentPermissionsPolicy();
#if QT_CONFIG(ssl)
    void additionalTrustedCertificates();
#endif
    void useSameDataPathForProfiles();
};

static QString StandardCacheLocation()
{
    static auto p = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    return p;
}
static QString StandardAppDataLocation()
{
    static auto p = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return p;
}

void tst_QWebEngineProfileBuilder::offTheRecordProfile()
{
    QScopedPointer<QWebEngineProfile> profile(
            QWebEngineProfileBuilder::createOffTheRecordProfile());
    QVERIFY(profile);
    QVERIFY(profile->isOffTheRecord());
    QCOMPARE(profile->storageName(), QString());
    QCOMPARE(profile->httpCacheType(), QWebEngineProfile::MemoryHttpCache);
    QCOMPARE(profile->persistentCookiesPolicy(), QWebEngineProfile::NoPersistentCookies);
    QCOMPARE(profile->cachePath(), QString());
    QCOMPARE(profile->persistentStoragePath(),
             StandardAppDataLocation() + QStringLiteral("/QtWebEngine/OffTheRecord"));
    QCOMPARE(profile->persistentPermissionsPolicy(),
             QWebEngineProfile::PersistentPermissionsPolicy::StoreInMemory);
}

void tst_QWebEngineProfileBuilder::diskBasedProfile()
{
    QWebEngineProfileBuilder profileBuilder;
    QScopedPointer<QWebEngineProfile> profile(profileBuilder.createProfile(QStringLiteral("Test")));
    QVERIFY(!profile.isNull());
    QVERIFY(!profile->isOffTheRecord());
    QCOMPARE(profile->storageName(), QStringLiteral("Test"));
    QCOMPARE(profile->httpCacheType(), QWebEngineProfile::DiskHttpCache);
    QCOMPARE(profile->persistentCookiesPolicy(), QWebEngineProfile::AllowPersistentCookies);
    QCOMPARE(profile->cachePath(), StandardCacheLocation() + QStringLiteral("/QtWebEngine/Test"));
    QCOMPARE(profile->persistentStoragePath(),
             StandardAppDataLocation() + QStringLiteral("/QtWebEngine/Test"));
    QCOMPARE(profile->persistentPermissionsPolicy(),
             QWebEngineProfile::PersistentPermissionsPolicy::StoreOnDisk);
}

void tst_QWebEngineProfileBuilder::persistentStoragePath()
{
    QTemporaryDir tempDir(QDir::tempPath() + "/tst_QWebEngineProfileBuilder-XXXXXX");

    QWebEngineProfileBuilder profileBuilder;
    profileBuilder.setPersistentStoragePath(tempDir.path());
    QScopedPointer<QWebEngineProfile> profile(profileBuilder.createProfile(QStringLiteral("Test")));
    QVERIFY(!profile.isNull());

    QCOMPARE(profile->persistentStoragePath(), tempDir.path());
    QVERIFY(!profile->isOffTheRecord());
    QCOMPARE(profile->storageName(), QStringLiteral("Test"));
    QCOMPARE(profile->httpCacheType(), QWebEngineProfile::DiskHttpCache);
    QCOMPARE(profile->persistentCookiesPolicy(), QWebEngineProfile::AllowPersistentCookies);
    QCOMPARE(profile->cachePath(), StandardCacheLocation() + QStringLiteral("/QtWebEngine/Test"));
    QCOMPARE(profile->persistentPermissionsPolicy(),
             QWebEngineProfile::PersistentPermissionsPolicy::StoreOnDisk);
}

void tst_QWebEngineProfileBuilder::cachePath()
{
    QTemporaryDir tempDir(QDir::tempPath() + "/tst_QWebEngineProfileBuilder-XXXXXX");

    QWebEngineProfileBuilder profileBuilder;
    profileBuilder.setCachePath(tempDir.path());
    QScopedPointer<QWebEngineProfile> profile(profileBuilder.createProfile(QStringLiteral("Test")));
    QVERIFY(!profile.isNull());

    QCOMPARE(profile->persistentStoragePath(),
             StandardAppDataLocation() + QStringLiteral("/QtWebEngine/Test"));
    QVERIFY(!profile->isOffTheRecord());
    QCOMPARE(profile->storageName(), QStringLiteral("Test"));
    QCOMPARE(profile->httpCacheType(), QWebEngineProfile::DiskHttpCache);
    QCOMPARE(profile->persistentCookiesPolicy(), QWebEngineProfile::AllowPersistentCookies);
    QCOMPARE(profile->cachePath(), tempDir.path());
    QCOMPARE(profile->persistentPermissionsPolicy(),
             QWebEngineProfile::PersistentPermissionsPolicy::StoreOnDisk);
}

void tst_QWebEngineProfileBuilder::httpCacheType_data()
{
    QTest::addColumn<QWebEngineProfile::HttpCacheType>("policy");
    QTest::addColumn<bool>("isOffTheRecord");
    QTest::newRow("noCacheOffTheRecordProfile") << QWebEngineProfile::NoCache << true;
    QTest::newRow("memoryHttpCacheOffTheRecordProfile")
            << QWebEngineProfile::MemoryHttpCache << true;
    QTest::newRow("noCacheDiskBasedProfile") << QWebEngineProfile::NoCache << false;
    QTest::newRow("diskHttpCacheDiskBasedProfile") << QWebEngineProfile::DiskHttpCache << false;
}

void tst_QWebEngineProfileBuilder::httpCacheType()
{
    QFETCH(QWebEngineProfile::HttpCacheType, policy);
    QFETCH(bool, isOffTheRecord);

    QWebEngineProfileBuilder profileBuilder;
    profileBuilder.setHttpCacheType(policy);
    QString storageName = isOffTheRecord ? QStringLiteral("") : QStringLiteral("Test");
    QScopedPointer<QWebEngineProfile> profile(profileBuilder.createProfile(storageName));
    QVERIFY(!profile.isNull());

    QCOMPARE(profile->httpCacheType(), policy);
    QVERIFY(isOffTheRecord ? profile->isOffTheRecord() : !profile->isOffTheRecord());
    QCOMPARE(profile->storageName(), storageName);
    QWebEngineProfile::PersistentCookiesPolicy cookiesPolicy = isOffTheRecord
            ? QWebEngineProfile::NoPersistentCookies
            : QWebEngineProfile::AllowPersistentCookies;
    QCOMPARE(profile->persistentCookiesPolicy(), cookiesPolicy);
    QString cachePath = isOffTheRecord
            ? QStringLiteral("")
            : StandardCacheLocation() + QStringLiteral("/QtWebEngine/Test");
    QCOMPARE(profile->cachePath(), cachePath);
    QString storagePath = isOffTheRecord
            ? StandardAppDataLocation() + QStringLiteral("/QtWebEngine/OffTheRecord")
            : StandardAppDataLocation() + QStringLiteral("/QtWebEngine/Test");
    QCOMPARE(profile->persistentStoragePath(), storagePath);
    QCOMPARE(profile->persistentPermissionsPolicy(),
             profile->isOffTheRecord()
                     ? QWebEngineProfile::PersistentPermissionsPolicy::StoreInMemory
                     : QWebEngineProfile::PersistentPermissionsPolicy::StoreOnDisk);
}

void tst_QWebEngineProfileBuilder::persistentCookiesPolicy_data()
{
    QTest::addColumn<QWebEngineProfile::PersistentCookiesPolicy>("policy");
    QTest::addColumn<bool>("isOffTheRecord");
    QTest::newRow("noPersistentCookiesOffTheRecord")
            << QWebEngineProfile::NoPersistentCookies << true;
    QTest::newRow("allowPersistentCookiesOffTheRecord")
            << QWebEngineProfile::AllowPersistentCookies << true;
    QTest::newRow("onlyPersistentCookiesOffTheRecord")
            << QWebEngineProfile::OnlyPersistentCookies << true;
    QTest::newRow("forcePersistentCookiesOffTheRecord")
            << QWebEngineProfile::ForcePersistentCookies << true;
    QTest::newRow("forcePersistentCookiesDiskBasedProfile")
            << QWebEngineProfile::ForcePersistentCookies << false;
    QTest::newRow("allowPersistentCookiesDiskBasedProfile")
            << QWebEngineProfile::AllowPersistentCookies << false;
    QTest::newRow("onlyPersistentCookiesDiskBasedProfile")
            << QWebEngineProfile::OnlyPersistentCookies << false;
    QTest::newRow("noPersistentCookiesDiskBasedProfile")
            << QWebEngineProfile::NoPersistentCookies << false;
}

void tst_QWebEngineProfileBuilder::persistentCookiesPolicy()
{
    QFETCH(QWebEngineProfile::PersistentCookiesPolicy, policy);
    QFETCH(bool, isOffTheRecord);

    QWebEngineProfileBuilder profileBuilder;
    profileBuilder.setPersistentCookiesPolicy(policy);
    QString storageName = isOffTheRecord ? QStringLiteral("") : QStringLiteral("Test");
    QScopedPointer<QWebEngineProfile> profile(profileBuilder.createProfile(storageName));
    QVERIFY(!profile.isNull());

    QVERIFY(isOffTheRecord ? profile->isOffTheRecord() : !profile->isOffTheRecord());
    QCOMPARE(profile->storageName(), storageName);

    QCOMPARE(profile->httpCacheType(),
             isOffTheRecord ? QWebEngineProfile::MemoryHttpCache
                            : QWebEngineProfile::DiskHttpCache);
    QWebEngineProfile::PersistentCookiesPolicy cookiesPolicy =
            isOffTheRecord ? QWebEngineProfile::NoPersistentCookies : policy;

    QCOMPARE(profile->persistentCookiesPolicy(), cookiesPolicy);
    QString cachePath = isOffTheRecord
            ? QStringLiteral("")
            : StandardCacheLocation() + QStringLiteral("/QtWebEngine/Test");
    QCOMPARE(profile->cachePath(), cachePath);
    QString storagePath = isOffTheRecord
            ? StandardAppDataLocation() + QStringLiteral("/QtWebEngine/OffTheRecord")
            : StandardAppDataLocation() + QStringLiteral("/QtWebEngine/Test");
    QCOMPARE(profile->persistentStoragePath(), storagePath);
    QCOMPARE(profile->persistentPermissionsPolicy(),
             profile->isOffTheRecord()
                     ? QWebEngineProfile::PersistentPermissionsPolicy::StoreInMemory
                     : QWebEngineProfile::PersistentPermissionsPolicy::StoreOnDisk);
}

void tst_QWebEngineProfileBuilder::httpCacheSize()
{
    QWebEngineProfileBuilder profileBuilder;
    profileBuilder.setHttpCacheMaximumSize(100);
    QScopedPointer<QWebEngineProfile> profile(profileBuilder.createProfile(QStringLiteral("Test")));
    QVERIFY(!profile.isNull());

    QVERIFY(!profile->isOffTheRecord());
    QCOMPARE(profile->storageName(), QStringLiteral("Test"));
    QCOMPARE(profile->httpCacheType(), QWebEngineProfile::DiskHttpCache);
    QCOMPARE(profile->persistentCookiesPolicy(), QWebEngineProfile::AllowPersistentCookies);
    QCOMPARE(profile->cachePath(), StandardCacheLocation() + QStringLiteral("/QtWebEngine/Test"));
    QCOMPARE(profile->persistentStoragePath(),
             StandardAppDataLocation() + QStringLiteral("/QtWebEngine/Test"));
    QCOMPARE(profile->httpCacheMaximumSize(), 100);
    QCOMPARE(profile->persistentPermissionsPolicy(),
             QWebEngineProfile::PersistentPermissionsPolicy::StoreOnDisk);
}

void tst_QWebEngineProfileBuilder::persistentPermissionsPolicy_data()
{
    QTest::addColumn<QWebEngineProfile::PersistentPermissionsPolicy>("policy");
    QTest::addColumn<bool>("isOffTheRecord");
    QTest::newRow("storeInMemoryOffTheRecord")
            << QWebEngineProfile::PersistentPermissionsPolicy::StoreInMemory << true;
    QTest::newRow("askEveryTimeOffTheRecord")
            << QWebEngineProfile::PersistentPermissionsPolicy::AskEveryTime << true;
    QTest::newRow("storeOnDiskOffTheRecord")
            << QWebEngineProfile::PersistentPermissionsPolicy::StoreOnDisk << true;
    QTest::newRow("storeOnDiskDiskBasedProfile")
            << QWebEngineProfile::PersistentPermissionsPolicy::StoreOnDisk << false;
    QTest::newRow("askEveryTimeDiskBasedProfile")
            << QWebEngineProfile::PersistentPermissionsPolicy::AskEveryTime << false;
    QTest::newRow("storeInMemoryDiskBasedProfile")
            << QWebEngineProfile::PersistentPermissionsPolicy::StoreInMemory << false;
}
void tst_QWebEngineProfileBuilder::persistentPermissionsPolicy()
{
    QFETCH(QWebEngineProfile::PersistentPermissionsPolicy, policy);
    QFETCH(bool, isOffTheRecord);

    QWebEngineProfileBuilder profileBuilder;
    profileBuilder.setPersistentPermissionsPolicy(policy);
    QString storageName = isOffTheRecord ? QStringLiteral("") : QStringLiteral("Test");
    QScopedPointer<QWebEngineProfile> profile(profileBuilder.createProfile(storageName));
    QVERIFY(!profile.isNull());

    QVERIFY(isOffTheRecord ? profile->isOffTheRecord() : !profile->isOffTheRecord());
    QCOMPARE(profile->storageName(), storageName);
    QCOMPARE(profile->httpCacheType(),
             isOffTheRecord ? QWebEngineProfile::MemoryHttpCache
                            : QWebEngineProfile::DiskHttpCache);
    QCOMPARE(profile->persistentPermissionsPolicy(),
             profile->isOffTheRecord()
                             && (policy
                                 == QWebEngineProfile::PersistentPermissionsPolicy::StoreOnDisk)
                     ? QWebEngineProfile::PersistentPermissionsPolicy::StoreInMemory
                     : policy);

    QWebEngineProfile::PersistentCookiesPolicy cookiesPolicy = isOffTheRecord
            ? QWebEngineProfile::NoPersistentCookies
            : QWebEngineProfile::AllowPersistentCookies;
    QCOMPARE(profile->persistentCookiesPolicy(), cookiesPolicy);

    QString cachePath = isOffTheRecord
            ? QStringLiteral("")
            : StandardCacheLocation() + QStringLiteral("/QtWebEngine/Test");
    QCOMPARE(profile->cachePath(), cachePath);

    QString storagePath = isOffTheRecord
            ? StandardAppDataLocation() + QStringLiteral("/QtWebEngine/OffTheRecord")
            : StandardAppDataLocation() + QStringLiteral("/QtWebEngine/Test");
    QCOMPARE(profile->persistentStoragePath(), storagePath);
}

#if QT_CONFIG(ssl)
void tst_QWebEngineProfileBuilder::additionalTrustedCertificates()
{
    if (QTestPrivate::isSecureTransportBlockingTest()) {
        QSKIP("SecureTransport will block the test server while accessing the login keychain");
    }

    QFile certFile(":/resources/server.pem");
    QVERIFY2(certFile.open(QIODevice::ReadOnly), qPrintable(certFile.errorString()));
    const QSslCertificate cert(&certFile, QSsl::Pem);

    QFile keyFile(":/resources/server.key");
    QVERIFY2(keyFile.open(QIODevice::ReadOnly), qPrintable(keyFile.errorString()));
    const QSslKey sslKey(&keyFile, QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey, "");

    HttpsServer server(":/resources/server.pem", ":/resources/server.key", ":/resources/ca.pem");
    server.setExpectError(false);
    QVERIFY(server.start());

    connect(&server, &HttpsServer::newRequest, [](HttpReqRep *rr) {
        rr->setResponseBody(QByteArrayLiteral("<html><body>TEST</body></html>"));
        rr->sendResponse();
    });

    {
        QWebEnginePage page;
        page.settings()->setAttribute(QWebEngineSettings::ErrorPageEnabled, false);

        page.profile()->clientCertificateStore()->add(cert, sslKey);

        connect(&page, &QWebEnginePage::selectClientCertificate, &page,
            [](QWebEngineClientCertificateSelection selection) {
                    Q_UNUSED(selection)
                    QFAIL("Should have rejected handshake already.");
                });

        QSignalSpy certificateErrorSpy(&page, &QWebEnginePage::certificateError);
        page.setUrl(server.url());

        QTRY_COMPARE_WITH_TIMEOUT(certificateErrorSpy.size() > 0, true, 20000);

        auto error = certificateErrorSpy.takeFirst().at(0).value<QWebEngineCertificateError>();
        QCOMPARE(error.type(), QWebEngineCertificateError::CertificateAuthorityInvalid);
    }

    // Add the appropriate server certificate, connection should work then.

    QList<QSslCertificate> certs;

    for (QString filename : {":/resources/server.pem", ":/resources/ca.pem"}) {
        QFile file(filename);
        QVERIFY2(file.open(QIODevice::ReadOnly), qPrintable(file.errorString()));
        certs.emplace_back(&file, QSsl::Pem);
        QVERIFY(!certs.back().isNull());
    }

    QWebEngineProfileBuilder profileBuilder;
    profileBuilder.setAdditionalTrustedCertificates(certs);
    QScopedPointer<QWebEngineProfile> profile(profileBuilder.createProfile(QStringLiteral("Test")));
    QVERIFY(profile);

    QCOMPARE(profile->additionalTrustedCertificates(), certs);

    {
        QWebEnginePage page(profile.get());
        page.settings()->setAttribute(QWebEngineSettings::ErrorPageEnabled, false);

        page.profile()->clientCertificateStore()->add(cert, sslKey);

        connect(&page, &QWebEnginePage::selectClientCertificate, &page,
            [&cert](QWebEngineClientCertificateSelection selection) {
                QVERIFY(!selection.certificates().isEmpty());
                for (const QSslCertificate &sCert : selection.certificates()) {
                    if (cert == sCert) {
                        selection.select(sCert);
                        return;
                    }
                }
                QFAIL("No certificate found.");
            });

        QSignalSpy loadFinishedSpy(&page, &QWebEnginePage::loadFinished);
        page.setUrl(server.url());

        QTRY_COMPARE_WITH_TIMEOUT(loadFinishedSpy.size() > 0, true, 20000);
        QCOMPARE(loadFinishedSpy.takeFirst().at(0).toBool(), true);
        QCOMPARE(toPlainTextSync(&page), QStringLiteral("TEST"));
    }

    QVERIFY(server.stop());
}
#endif // QT_CONFIG(ssl)

void tst_QWebEngineProfileBuilder::useSameDataPathForProfiles()
{
    QWebEngineProfileBuilder profileBuilder;
    QScopedPointer<QWebEngineProfile> profile(profileBuilder.createProfile(QStringLiteral("Test")));
    QVERIFY(!profile.isNull());
    QVERIFY(!profile->isOffTheRecord());
    QCOMPARE(profile->storageName(), QStringLiteral("Test"));
    QCOMPARE(profile->httpCacheType(), QWebEngineProfile::DiskHttpCache);
    QCOMPARE(profile->persistentCookiesPolicy(), QWebEngineProfile::AllowPersistentCookies);
    QCOMPARE(profile->cachePath(), StandardCacheLocation() + QStringLiteral("/QtWebEngine/Test"));
    QCOMPARE(profile->persistentStoragePath(),
             StandardAppDataLocation() + QStringLiteral("/QtWebEngine/Test"));

    QScopedPointer<QWebEngineProfile> secondProfile(
            profileBuilder.createProfile(QStringLiteral("Test")));
    QVERIFY(secondProfile.isNull());
}

QTEST_MAIN(tst_QWebEngineProfileBuilder)
#include "tst_qwebengineprofilebuilder.moc"
