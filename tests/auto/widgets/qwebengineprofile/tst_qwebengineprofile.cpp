/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#include "../util.h"
#include <QtTest/QtTest>
#include <qwebengineprofile.h>
#include <qwebenginepage.h>

class tst_QWebEngineProfile : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void defaultProfile();
    void profileConstructors();
    void clearDataFromCache();
    void disableCache();
};

void tst_QWebEngineProfile::defaultProfile()
{
    QWebEngineProfile *profile = QWebEngineProfile::defaultProfile();
    QVERIFY(profile);
    QVERIFY(!profile->isOffTheRecord());
    QCOMPARE(profile->storageName(), QStringLiteral("Default"));
    QCOMPARE(profile->httpCacheType(), QWebEngineProfile::DiskHttpCache);
    QCOMPARE(profile->persistentCookiesPolicy(), QWebEngineProfile::AllowPersistentCookies);
}

void tst_QWebEngineProfile::profileConstructors()
{
    QWebEngineProfile otrProfile;
    QWebEngineProfile diskProfile(QStringLiteral("Test"));

    QVERIFY(otrProfile.isOffTheRecord());
    QVERIFY(!diskProfile.isOffTheRecord());
    QCOMPARE(diskProfile.storageName(), QStringLiteral("Test"));
    QCOMPARE(otrProfile.httpCacheType(), QWebEngineProfile::MemoryHttpCache);
    QCOMPARE(diskProfile.httpCacheType(), QWebEngineProfile::DiskHttpCache);
    QCOMPARE(otrProfile.persistentCookiesPolicy(), QWebEngineProfile::NoPersistentCookies);
    QCOMPARE(diskProfile.persistentCookiesPolicy(), QWebEngineProfile::AllowPersistentCookies);
}

void tst_QWebEngineProfile::clearDataFromCache()
{
    QWebEnginePage page;

    QDir cacheDir("./tst_QWebEngineProfile_cacheDir");
    cacheDir.makeAbsolute();
    if (cacheDir.exists())
        cacheDir.removeRecursively();
    cacheDir.mkpath(cacheDir.path());

    QWebEngineProfile *profile = page.profile();
    profile->setCachePath(cacheDir.path());
    profile->setHttpCacheType(QWebEngineProfile::DiskHttpCache);

    QSignalSpy loadFinishedSpy(&page, SIGNAL(loadFinished(bool)));
    page.load(QUrl("http://qt-project.org"));
    if (!loadFinishedSpy.wait(10000) || !loadFinishedSpy.at(0).at(0).toBool())
        QSKIP("Couldn't load page from network, skipping test.");

    cacheDir.refresh();
    QVERIFY(cacheDir.entryList().contains("Cache"));
    cacheDir.cd("./Cache");
    int filesBeforeClear = cacheDir.entryList().count();

    QFileSystemWatcher fileSystemWatcher;
    fileSystemWatcher.addPath(cacheDir.path());
    QSignalSpy directoryChangedSpy(&fileSystemWatcher, SIGNAL(directoryChanged(const QString &)));

    // It deletes most of the files, but not all of them.
    profile->clearHttpCache();
    QTest::qWait(1000);
    QTRY_VERIFY(directoryChangedSpy.count() > 0);

    cacheDir.refresh();
    QVERIFY(filesBeforeClear > cacheDir.entryList().count());

    cacheDir.removeRecursively();
}

void tst_QWebEngineProfile::disableCache()
{
    QWebEnginePage page;
    QDir cacheDir("./tst_QWebEngineProfile_cacheDir");
    if (cacheDir.exists())
        cacheDir.removeRecursively();
    cacheDir.mkpath(cacheDir.path());

    QWebEngineProfile *profile = page.profile();
    profile->setCachePath(cacheDir.path());
    QVERIFY(!cacheDir.entryList().contains("Cache"));

    profile->setHttpCacheType(QWebEngineProfile::NoCache);
    QSignalSpy loadFinishedSpy(&page, SIGNAL(loadFinished(bool)));
    page.load(QUrl("http://qt-project.org"));
    if (!loadFinishedSpy.wait(10000) || !loadFinishedSpy.at(0).at(0).toBool())
        QSKIP("Couldn't load page from network, skipping test.");

    cacheDir.refresh();
    QVERIFY(!cacheDir.entryList().contains("Cache"));

    profile->setHttpCacheType(QWebEngineProfile::DiskHttpCache);
    page.load(QUrl("http://qt-project.org"));
    if (!loadFinishedSpy.wait(10000) || !loadFinishedSpy.at(1).at(0).toBool())
        QSKIP("Couldn't load page from network, skipping test.");

    cacheDir.refresh();
    QVERIFY(cacheDir.entryList().contains("Cache"));

    cacheDir.removeRecursively();
}

QTEST_MAIN(tst_QWebEngineProfile)
#include "tst_qwebengineprofile.moc"
