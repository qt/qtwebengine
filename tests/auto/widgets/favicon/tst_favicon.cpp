// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <QtTest/QtTest>
#include <util.h>

#include <QWebEngineHistory>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QWebEngineView>

class tst_Favicon : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void init();
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

private Q_SLOTS:
    void faviconLoad();
    void faviconLoadFromResources();
    void faviconLoadEncodedUrl();
    void faviconLoadAfterHistoryNavigation();
    void faviconLoadPushState();
    void noFavicon();
    void aboutBlank();
    void unavailableFavicon();
    void errorPageEnabled();
    void errorPageDisabled();
    void touchIcon();
    void multiIcon();
    void downloadIconsDisabled_data();
    void downloadIconsDisabled();
    void downloadTouchIconsEnabled_data();
    void downloadTouchIconsEnabled();
    void dynamicFavicon();
    void touchIconWithSameURL();

    void iconDatabaseOTR();
    void requestIconForIconURL_data();
    void requestIconForIconURL();
    void requestIconForPageURL_data();
    void requestIconForPageURL();
    void desiredSize();

private:
    QWebEngineView *m_view;
    QWebEnginePage *m_page;
    QWebEngineProfile *m_profile;
};

void tst_Favicon::init()
{
    m_profile = new QWebEngineProfile(this);
    m_view = new QWebEngineView();
    m_page = new QWebEnginePage(m_profile, m_view);
    m_view->setPage(m_page);
}

void tst_Favicon::initTestCase() { }

void tst_Favicon::cleanupTestCase() { }

void tst_Favicon::cleanup()
{
    delete m_view;
    delete m_profile;
}

void tst_Favicon::faviconLoad()
{
    if (!QDir(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()).exists())
        W_QSKIP(QString("This test requires access to resources found in '%1'")
                        .arg(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath())
                        .toLatin1()
                        .constData(),
                SkipAll);

    QSignalSpy loadFinishedSpy(m_page, SIGNAL(loadFinished(bool)));
    QSignalSpy iconUrlChangedSpy(m_page, SIGNAL(iconUrlChanged(QUrl)));
    QSignalSpy iconChangedSpy(m_page, SIGNAL(iconChanged(QIcon)));

    QUrl url = QUrl::fromLocalFile(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()
                                   + QLatin1String("/resources/favicon-single.html"));
    m_page->load(url);

    QTRY_COMPARE_WITH_TIMEOUT(loadFinishedSpy.size(), 1, 30000);
    QTRY_COMPARE(iconUrlChangedSpy.size(), 1);
    QTRY_COMPARE(iconChangedSpy.size(), 1);

    QUrl iconUrl = iconUrlChangedSpy.at(0).at(0).toString();
    QCOMPARE(iconUrl, m_page->iconUrl());
    QCOMPARE(iconUrl,
             QUrl::fromLocalFile(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()
                                 + QLatin1String("/resources/icons/qt32.ico")));

    const QIcon &icon = m_page->icon();
    QVERIFY(!icon.isNull());

    QCOMPARE(icon.availableSizes().size(), 2);
    QVERIFY(icon.availableSizes().contains(QSize(16, 16)));
    QVERIFY(icon.availableSizes().contains(QSize(32, 32)));
}

void tst_Favicon::faviconLoadFromResources()
{
    QSignalSpy loadFinishedSpy(m_page, SIGNAL(loadFinished(bool)));
    QSignalSpy iconUrlChangedSpy(m_page, SIGNAL(iconUrlChanged(QUrl)));
    QSignalSpy iconChangedSpy(m_page, SIGNAL(iconChanged(QIcon)));

    QUrl url("qrc:/resources/favicon-single.html");
    m_page->load(url);

    QTRY_COMPARE_WITH_TIMEOUT(loadFinishedSpy.size(), 1, 30000);
    QTRY_COMPARE(iconUrlChangedSpy.size(), 1);
    QTRY_COMPARE(iconChangedSpy.size(), 1);

    QUrl iconUrl = iconUrlChangedSpy.at(0).at(0).toString();
    QCOMPARE(iconUrl, m_page->iconUrl());
    QCOMPARE(iconUrl, QUrl("qrc:/resources/icons/qt32.ico"));

    const QIcon &icon = m_page->icon();
    QVERIFY(!icon.isNull());

    QCOMPARE(icon.availableSizes().size(), 2);
    QVERIFY(icon.availableSizes().contains(QSize(16, 16)));
    QVERIFY(icon.availableSizes().contains(QSize(32, 32)));
}

void tst_Favicon::faviconLoadEncodedUrl()
{
    if (!QDir(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()).exists())
        W_QSKIP(QString("This test requires access to resources found in '%1'")
                        .arg(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath())
                        .toLatin1()
                        .constData(),
                SkipAll);

    QSignalSpy loadFinishedSpy(m_page, SIGNAL(loadFinished(bool)));
    QSignalSpy iconUrlChangedSpy(m_page, SIGNAL(iconUrlChanged(QUrl)));
    QSignalSpy iconChangedSpy(m_page, SIGNAL(iconChanged(QIcon)));

    QString urlString = QUrl::fromLocalFile(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()
                                            + QLatin1String("/resources/favicon-single.html"))
                                .toString();
    QUrl url(urlString + QLatin1String("?favicon=load should work with#whitespace!"));
    m_page->load(url);

    QTRY_COMPARE_WITH_TIMEOUT(loadFinishedSpy.size(), 1, 30000);
    QTRY_COMPARE(iconUrlChangedSpy.size(), 1);
    QTRY_COMPARE(iconChangedSpy.size(), 1);

    QUrl iconUrl = iconUrlChangedSpy.at(0).at(0).toString();
    QCOMPARE(m_page->iconUrl(), iconUrl);
    QCOMPARE(iconUrl,
             QUrl::fromLocalFile(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()
                                 + QLatin1String("/resources/icons/qt32.ico")));

    const QIcon &icon = m_page->icon();
    QVERIFY(!icon.isNull());

    QCOMPARE(icon.availableSizes().size(), 2);
    QVERIFY(icon.availableSizes().contains(QSize(16, 16)));
    QVERIFY(icon.availableSizes().contains(QSize(32, 32)));
}

void tst_Favicon::faviconLoadAfterHistoryNavigation()
{
    QSignalSpy loadFinishedSpy(m_page, SIGNAL(loadFinished(bool)));
    QSignalSpy iconUrlChangedSpy(m_page, SIGNAL(iconUrlChanged(QUrl)));
    QSignalSpy iconChangedSpy(m_page, SIGNAL(iconChanged(QIcon)));

    m_page->load(QUrl("qrc:/resources/favicon-single.html"));
    QTRY_COMPARE_WITH_TIMEOUT(loadFinishedSpy.size(), 1, 30000);
    QTRY_COMPARE(iconUrlChangedSpy.size(), 1);
    QTRY_COMPARE(iconChangedSpy.size(), 1);
    QCOMPARE(m_page->iconUrl(), QUrl("qrc:/resources/icons/qt32.ico"));

    m_page->load(QUrl("qrc:/resources/favicon-multi.html"));
    QTRY_COMPARE_WITH_TIMEOUT(loadFinishedSpy.size(), 2, 30000);
    QTRY_COMPARE(iconUrlChangedSpy.size(), 3);
    QTRY_COMPARE(iconChangedSpy.size(), 3);
    QCOMPARE(m_page->iconUrl(), QUrl("qrc:/resources/icons/qtmulti.ico"));

    m_page->triggerAction(QWebEnginePage::Back);
    QTRY_COMPARE_WITH_TIMEOUT(loadFinishedSpy.size(), 3, 30000);
    QTRY_COMPARE(iconUrlChangedSpy.size(), 5);
    QTRY_COMPARE(iconChangedSpy.size(), 5);
    QCOMPARE(m_page->iconUrl(), QUrl("qrc:/resources/icons/qt32.ico"));

    m_page->triggerAction(QWebEnginePage::Forward);
    QTRY_COMPARE_WITH_TIMEOUT(loadFinishedSpy.size(), 4, 30000);
    QTRY_COMPARE(iconUrlChangedSpy.size(), 7);
    QTRY_COMPARE(iconChangedSpy.size(), 7);
    QCOMPARE(m_page->iconUrl(), QUrl("qrc:/resources/icons/qtmulti.ico"));
}

void tst_Favicon::faviconLoadPushState()
{
    QSignalSpy loadFinishedSpy(m_page, SIGNAL(loadFinished(bool)));
    QSignalSpy iconUrlChangedSpy(m_page, SIGNAL(iconUrlChanged(QUrl)));
    QSignalSpy iconChangedSpy(m_page, SIGNAL(iconChanged(QIcon)));

    QUrl url("qrc:/resources/favicon-single.html");
    m_page->load(url);

    QTRY_COMPARE_WITH_TIMEOUT(loadFinishedSpy.size(), 1, 30000);
    QTRY_COMPARE(iconUrlChangedSpy.size(), 1);
    QTRY_COMPARE(iconChangedSpy.size(), 1);

    QUrl iconUrl = iconUrlChangedSpy.at(0).at(0).toString();
    QCOMPARE(iconUrl, m_page->iconUrl());
    QCOMPARE(iconUrl, QUrl("qrc:/resources/icons/qt32.ico"));

    const QIcon &icon = m_page->icon();
    QVERIFY(!icon.isNull());

    iconUrlChangedSpy.clear();
    iconChangedSpy.clear();

    // pushState() is a same document navigation and should not reset or
    // update favicon.
    QCOMPARE(m_page->history()->count(), 1);
    evaluateJavaScriptSync(m_page, "history.pushState('', '')");
    QTRY_COMPARE(m_page->history()->count(), 2);

    // Favicon change is not expected.
    QCOMPARE(iconUrlChangedSpy.size(), 0);
    QCOMPARE(iconChangedSpy.size(), 0);
    QCOMPARE(m_page->iconUrl(), QUrl("qrc:/resources/icons/qt32.ico"));
}

void tst_Favicon::noFavicon()
{
    if (!QDir(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()).exists())
        W_QSKIP(QString("This test requires access to resources found in '%1'")
                        .arg(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath())
                        .toLatin1()
                        .constData(),
                SkipAll);

    QSignalSpy loadFinishedSpy(m_page, SIGNAL(loadFinished(bool)));
    QSignalSpy iconUrlChangedSpy(m_page, SIGNAL(iconUrlChanged(QUrl)));
    QSignalSpy iconChangedSpy(m_page, SIGNAL(iconChanged(QIcon)));

    QUrl url = QUrl::fromLocalFile(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()
                                   + QLatin1String("/resources/test1.html"));
    m_page->load(url);

    QTRY_COMPARE_WITH_TIMEOUT(loadFinishedSpy.size(), 1, 30000);
    QCOMPARE(iconUrlChangedSpy.size(), 0);
    QCOMPARE(iconChangedSpy.size(), 0);

    QVERIFY(m_page->iconUrl().isEmpty());
    QVERIFY(m_page->icon().isNull());
}

void tst_Favicon::aboutBlank()
{
    QSignalSpy loadFinishedSpy(m_page, SIGNAL(loadFinished(bool)));
    QSignalSpy iconUrlChangedSpy(m_page, SIGNAL(iconUrlChanged(QUrl)));
    QSignalSpy iconChangedSpy(m_page, SIGNAL(iconChanged(QIcon)));

    QUrl url("about:blank");
    m_page->load(url);

    QTRY_COMPARE_WITH_TIMEOUT(loadFinishedSpy.size(), 1, 30000);
    QCOMPARE(iconUrlChangedSpy.size(), 0);
    QCOMPARE(iconChangedSpy.size(), 0);

    QVERIFY(m_page->iconUrl().isEmpty());
    QVERIFY(m_page->icon().isNull());
}

void tst_Favicon::unavailableFavicon()
{
    if (!QDir(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()).exists())
        W_QSKIP(QString("This test requires access to resources found in '%1'")
                        .arg(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath())
                        .toLatin1()
                        .constData(),
                SkipAll);

    QSignalSpy loadFinishedSpy(m_page, SIGNAL(loadFinished(bool)));
    QSignalSpy iconUrlChangedSpy(m_page, SIGNAL(iconUrlChanged(QUrl)));
    QSignalSpy iconChangedSpy(m_page, SIGNAL(iconChanged(QIcon)));

    QUrl url = QUrl::fromLocalFile(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()
                                   + QLatin1String("/resources/favicon-unavailable.html"));
    m_page->load(url);

    QTRY_COMPARE_WITH_TIMEOUT(loadFinishedSpy.size(), 1, 30000);
    QCOMPARE(iconUrlChangedSpy.size(), 0);
    QCOMPARE(iconChangedSpy.size(), 0);

    QVERIFY(m_page->iconUrl().isEmpty());
    QVERIFY(m_page->icon().isNull());
}

void tst_Favicon::errorPageEnabled()
{
    m_page->settings()->setAttribute(QWebEngineSettings::ErrorPageEnabled, true);

    QSignalSpy loadFinishedSpy(m_page, SIGNAL(loadFinished(bool)));
    QSignalSpy iconUrlChangedSpy(m_page, SIGNAL(iconUrlChanged(QUrl)));
    QSignalSpy iconChangedSpy(m_page, SIGNAL(iconChanged(QIcon)));

    QUrl url("http://url.invalid");
    m_page->load(url);

    QTRY_COMPARE_WITH_TIMEOUT(loadFinishedSpy.size(), 1, 30000);
    QCOMPARE(iconUrlChangedSpy.size(), 0);
    QCOMPARE(iconChangedSpy.size(), 0);

    QVERIFY(m_page->iconUrl().isEmpty());
    QVERIFY(m_page->icon().isNull());
}

void tst_Favicon::errorPageDisabled()
{
    m_page->settings()->setAttribute(QWebEngineSettings::ErrorPageEnabled, false);

    QSignalSpy loadFinishedSpy(m_page, SIGNAL(loadFinished(bool)));
    QSignalSpy iconUrlChangedSpy(m_page, SIGNAL(iconUrlChanged(QUrl)));
    QSignalSpy iconChangedSpy(m_page, SIGNAL(iconChanged(QIcon)));

    QUrl url("http://url.invalid");
    m_page->load(url);

    QTRY_COMPARE_WITH_TIMEOUT(loadFinishedSpy.size(), 1, 30000);
    QCOMPARE(iconUrlChangedSpy.size(), 0);
    QCOMPARE(iconChangedSpy.size(), 0);

    QVERIFY(m_page->iconUrl().isEmpty());
    QVERIFY(m_page->icon().isNull());
}

void tst_Favicon::touchIcon()
{
    if (!QDir(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()).exists())
        W_QSKIP(QString("This test requires access to resources found in '%1'")
                        .arg(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath())
                        .toLatin1()
                        .constData(),
                SkipAll);

    QSignalSpy loadFinishedSpy(m_page, SIGNAL(loadFinished(bool)));
    QSignalSpy iconUrlChangedSpy(m_page, SIGNAL(iconUrlChanged(QUrl)));
    QSignalSpy iconChangedSpy(m_page, SIGNAL(iconChanged(QIcon)));

    QUrl url = QUrl::fromLocalFile(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()
                                   + QLatin1String("/resources/favicon-touch.html"));
    m_page->load(url);

    QTRY_COMPARE_WITH_TIMEOUT(loadFinishedSpy.size(), 1, 30000);
    QCOMPARE(iconUrlChangedSpy.size(), 0);
    QCOMPARE(iconChangedSpy.size(), 0);

    QVERIFY(m_page->iconUrl().isEmpty());
    QVERIFY(m_page->icon().isNull());
}

void tst_Favicon::multiIcon()
{
    if (!QDir(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()).exists())
        W_QSKIP(QString("This test requires access to resources found in '%1'")
                        .arg(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath())
                        .toLatin1()
                        .constData(),
                SkipAll);

    QSignalSpy loadFinishedSpy(m_page, SIGNAL(loadFinished(bool)));
    QSignalSpy iconUrlChangedSpy(m_page, SIGNAL(iconUrlChanged(QUrl)));
    QSignalSpy iconChangedSpy(m_page, SIGNAL(iconChanged(QIcon)));

    QUrl url = QUrl::fromLocalFile(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()
                                   + QLatin1String("/resources/favicon-multi.html"));
    QUrl iconUrl;
    QIcon icon;

    // If touch icons are disabled, the favicon is provided in two sizes (16x16 and 32x32) according
    // to the supported scale factors (100P, 200P).
    m_page->settings()->setAttribute(QWebEngineSettings::TouchIconsEnabled, false);
    m_page->load(url);

    QTRY_COMPARE_WITH_TIMEOUT(loadFinishedSpy.size(), 1, 30000);
    QTRY_COMPARE(iconUrlChangedSpy.size(), 1);
    QTRY_COMPARE(iconChangedSpy.size(), 1);

    iconUrl = iconUrlChangedSpy.at(0).at(0).toString();
    QCOMPARE(m_page->iconUrl(), iconUrl);
    QCOMPARE(iconUrl,
             QUrl::fromLocalFile(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()
                                 + QLatin1String("/resources/icons/qtmulti.ico")));

    icon = m_page->icon();
    QVERIFY(!icon.isNull());
    QCOMPARE(icon.availableSizes().size(), 2);
    QVERIFY(icon.availableSizes().contains(QSize(16, 16)));
    QVERIFY(icon.availableSizes().contains(QSize(32, 32)));

    // Reset
    loadFinishedSpy.clear();
    m_page->load(QUrl("about:blank"));
    QTRY_COMPARE_WITH_TIMEOUT(loadFinishedSpy.size(), 1, 30000);
    iconUrlChangedSpy.clear();
    iconChangedSpy.clear();
    loadFinishedSpy.clear();
    icon = QIcon();

    // If touch icons are enabled, the largest icon is provided.
    m_page->settings()->setAttribute(QWebEngineSettings::TouchIconsEnabled, true);
    m_page->load(url);

    QTRY_COMPARE_WITH_TIMEOUT(loadFinishedSpy.size(), 1, 30000);
    QTRY_COMPARE(iconUrlChangedSpy.size(), 1);
    QTRY_COMPARE(iconChangedSpy.size(), 1);

    iconUrl = iconUrlChangedSpy.at(0).at(0).toString();
    QCOMPARE(m_page->iconUrl(), iconUrl);
    QCOMPARE(iconUrl,
             QUrl::fromLocalFile(QDir(QT_TESTCASE_SOURCEDIR).canonicalPath()
                                 + QLatin1String("/resources/icons/qtmulti.ico")));

    icon = m_page->icon();
    QVERIFY(!icon.isNull());
    QCOMPARE(icon.availableSizes().size(), 1);
    QVERIFY(icon.availableSizes().contains(QSize(64, 64)));
}

void tst_Favicon::downloadIconsDisabled_data()
{
    QTest::addColumn<QUrl>("url");
    QTest::newRow("misc") << QUrl("qrc:/resources/favicon-misc.html");
    QTest::newRow("shortcut") << QUrl("qrc:/resources/favicon-shortcut.html");
    QTest::newRow("single") << QUrl("qrc:/resources/favicon-single.html");
    QTest::newRow("touch") << QUrl("qrc:/resources/favicon-touch.html");
    QTest::newRow("unavailable") << QUrl("qrc:/resources/favicon-unavailable.html");
}

void tst_Favicon::downloadIconsDisabled()
{
    QFETCH(QUrl, url);

    m_page->settings()->setAttribute(QWebEngineSettings::AutoLoadIconsForPage, false);

    QSignalSpy loadFinishedSpy(m_page, SIGNAL(loadFinished(bool)));
    QSignalSpy iconUrlChangedSpy(m_page, SIGNAL(iconUrlChanged(QUrl)));
    QSignalSpy iconChangedSpy(m_page, SIGNAL(iconChanged(QIcon)));

    m_page->load(url);

    QTRY_COMPARE_WITH_TIMEOUT(loadFinishedSpy.size(), 1, 30000);
    QCOMPARE(iconUrlChangedSpy.size(), 0);
    QCOMPARE(iconChangedSpy.size(), 0);

    QVERIFY(m_page->iconUrl().isEmpty());
    QVERIFY(m_page->icon().isNull());
}

void tst_Favicon::downloadTouchIconsEnabled_data()
{
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<QUrl>("expectedIconUrl");
    QTest::addColumn<QSize>("expectedIconSize");
    QTest::newRow("misc") << QUrl("qrc:/resources/favicon-misc.html")
                          << QUrl("qrc:/resources/icons/qt144.png") << QSize(144, 144);
    QTest::newRow("shortcut") << QUrl("qrc:/resources/favicon-shortcut.html")
                              << QUrl("qrc:/resources/icons/qt144.png") << QSize(144, 144);
    QTest::newRow("single") << QUrl("qrc:/resources/favicon-single.html")
                            << QUrl("qrc:/resources/icons/qt32.ico") << QSize(32, 32);
    QTest::newRow("touch") << QUrl("qrc:/resources/favicon-touch.html")
                           << QUrl("qrc:/resources/icons/qt144.png") << QSize(144, 144);
}

void tst_Favicon::downloadTouchIconsEnabled()
{
    QFETCH(QUrl, url);
    QFETCH(QUrl, expectedIconUrl);
    QFETCH(QSize, expectedIconSize);

    m_page->settings()->setAttribute(QWebEngineSettings::TouchIconsEnabled, true);

    QSignalSpy loadFinishedSpy(m_page, SIGNAL(loadFinished(bool)));
    QSignalSpy iconUrlChangedSpy(m_page, SIGNAL(iconUrlChanged(QUrl)));
    QSignalSpy iconChangedSpy(m_page, SIGNAL(iconChanged(QIcon)));

    m_page->load(url);

    QTRY_COMPARE_WITH_TIMEOUT(loadFinishedSpy.size(), 1, 30000);
    QTRY_COMPARE(iconUrlChangedSpy.size(), 1);
    QTRY_COMPARE(iconChangedSpy.size(), 1);

    const QUrl &iconUrl = iconUrlChangedSpy.at(0).at(0).toString();
    QCOMPARE(m_page->iconUrl(), iconUrl);
    QCOMPARE(iconUrl, expectedIconUrl);

    const QIcon &icon = m_page->icon();
    QVERIFY(!icon.isNull());

    QCOMPARE(icon.availableSizes().size(), 1);
    QCOMPARE(icon.availableSizes().first(), expectedIconSize);
}

void tst_Favicon::dynamicFavicon()
{
    QSignalSpy loadFinishedSpy(m_page, SIGNAL(loadFinished(bool)));
    QSignalSpy iconUrlChangedSpy(m_page, SIGNAL(iconUrlChanged(QUrl)));
    QSignalSpy iconChangedSpy(m_page, SIGNAL(iconChanged(QIcon)));

    QMap<Qt::GlobalColor, QString> colors;
    colors.insert(Qt::red,
                  QString("iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mP8z8DwHwAFBQIAX8jx0gAAAABJRU5ErkJggg=="));
    colors.insert(Qt::green,
                  QString("iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNk+M/wHwAEBgIApD5fRAAAAABJRU5ErkJggg=="));
    colors.insert(Qt::blue,
                  QString("iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNkYPj/HwADBwIAMCbHYQAAAABJRU5ErkJggg=="));

    m_page->setHtml("<html>"
                    "<link rel='icon' type='image/png' "
                    "href='data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mNk+A8AAQUBAScY42YAAAAASUVORK5CYII='/>"
                    "</html>");
    QTRY_COMPARE_WITH_TIMEOUT(loadFinishedSpy.size(), 1, 30000);
    QTRY_COMPARE(iconUrlChangedSpy.size(), 1);
    QTRY_COMPARE(iconChangedSpy.size(), 1);

    QCOMPARE(m_page->icon().pixmap(1, 1).toImage().pixelColor(0, 0), QColor(Qt::black));

    for (Qt::GlobalColor color : colors.keys()) {
        iconChangedSpy.clear();
        evaluateJavaScriptSync(
                m_page,
                "document.getElementsByTagName('link')[0].href = 'data:image/png;base64," + colors[color] + "';");
        QTRY_COMPARE(iconChangedSpy.size(), 1);
        QTRY_COMPARE(m_page->iconUrl().toString(),
                     QString("data:image/png;base64," + colors[color]));
        QCOMPARE(m_page->icon().pixmap(1, 1).toImage().pixelColor(0, 0), QColor(color));
    }
}

void tst_Favicon::touchIconWithSameURL()
{
    m_page->settings()->setAttribute(QWebEngineSettings::TouchIconsEnabled, false);

    const QString icon("data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mNk+A8AAQUBAScY42YAAAAASUVORK5CYII=");
    QSignalSpy loadFinishedSpy(m_page, SIGNAL(loadFinished(bool)));
    QSignalSpy iconUrlChangedSpy(m_page, SIGNAL(iconUrlChanged(QUrl)));
    QSignalSpy iconChangedSpy(m_page, SIGNAL(iconChanged(QIcon)));

    m_page->setHtml("<html>"
                    "<link rel='icon' type='image/png' href='" + icon + "'/>"
                    "<link rel='apple-touch-icon' type='image/png' href='" + icon + "'/>"
                    "</html>");
    QTRY_COMPARE_WITH_TIMEOUT(loadFinishedSpy.size(), 1, 30000);

    // The default favicon has to be loaded even if its URL is also set as a touch icon while touch
    // icons are disabled.
    QTRY_COMPARE(iconUrlChangedSpy.size(), 1);
    QCOMPARE(m_page->iconUrl().toString(), icon);
    QTRY_COMPARE(iconChangedSpy.size(), 1);

    loadFinishedSpy.clear();
    iconUrlChangedSpy.clear();
    iconChangedSpy.clear();

    m_page->setHtml("<html>"
                    "<link rel='apple-touch-icon' type='image/png' href='" + icon + "'/>"
                    "</html>");
    QTRY_COMPARE(loadFinishedSpy.size(), 1);

    // This page only has a touch icon. With disabled touch icons we don't expect any icon to be
    // shown even if the same icon was loaded previously.
    QTRY_COMPARE(iconUrlChangedSpy.size(), 1);
    QVERIFY(m_page->iconUrl().toString().isEmpty());
    QTRY_COMPARE(iconChangedSpy.size(), 1);
}

void tst_Favicon::iconDatabaseOTR()
{
    QWebEngineProfile profile;
    QWebEngineView view;
    QWebEnginePage *page = new QWebEnginePage(&profile, &view);
    view.setPage(page);

    QSignalSpy loadFinishedSpy(page, SIGNAL(loadFinished(bool)));
    QSignalSpy iconUrlChangedSpy(page, SIGNAL(iconUrlChanged(QUrl)));
    QSignalSpy iconChangedSpy(page, SIGNAL(iconChanged(QIcon)));

    page->load(QUrl("qrc:/resources/favicon-misc.html"));

    QTRY_COMPARE(loadFinishedSpy.size(), 1);
    QTRY_COMPARE(iconUrlChangedSpy.size(), 1);
    QTRY_COMPARE(iconChangedSpy.size(), 1);

    {
        bool iconRequestDone = false;
        profile.requestIconForIconURL(page->iconUrl(), 0,
                                      [page, &iconRequestDone](const QIcon &icon, const QUrl &iconUrl) {
            QVERIFY(icon.isNull());
            QCOMPARE(iconUrl, page->iconUrl());
            iconRequestDone = true;
        });
        QTRY_VERIFY(iconRequestDone);
    }

    {
        bool iconRequestDone = false;
        profile.requestIconForPageURL(page->url(), 0,
                                      [page, &iconRequestDone](const QIcon &icon, const QUrl &iconUrl, const QUrl &pageUrl) {
            QVERIFY(icon.isNull());
            QVERIFY(iconUrl.isEmpty());
            QCOMPARE(pageUrl, page->url());
            iconRequestDone = true;
        });
        QTRY_VERIFY(iconRequestDone);
    }
}

void tst_Favicon::requestIconForIconURL_data()
{
    QTest::addColumn<bool>("touchIconsEnabled");
    QTest::newRow("touch icons enabled") << true;
    QTest::newRow("touch icons disabled") << false;
}

void tst_Favicon::requestIconForIconURL()
{
    QFETCH(bool, touchIconsEnabled);

    QTemporaryDir tmpDir;
    QWebEngineProfile profile("iconDatabase-iconurl");
    profile.setPersistentStoragePath(tmpDir.path());
    profile.settings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
    profile.settings()->setAttribute(QWebEngineSettings::TouchIconsEnabled, touchIconsEnabled);

    QWebEngineView view;
    QWebEnginePage *page = new QWebEnginePage(&profile, &view);
    view.setPage(page);

    QSignalSpy loadFinishedSpy(page, SIGNAL(loadFinished(bool)));
    QSignalSpy iconUrlChangedSpy(page, SIGNAL(iconUrlChanged(QUrl)));
    QSignalSpy iconChangedSpy(page, SIGNAL(iconChanged(QIcon)));

    page->load(QUrl("qrc:/resources/favicon-misc.html"));

    QTRY_COMPARE(loadFinishedSpy.size(), 1);
    QTRY_COMPARE(iconUrlChangedSpy.size(), 1);
    QTRY_COMPARE(iconChangedSpy.size(), 1);

    page->load(QUrl("about:blank"));

    QTRY_COMPARE(loadFinishedSpy.size(), 2);
    QTRY_COMPARE(iconUrlChangedSpy.size(), 2);
    QTRY_COMPARE(iconChangedSpy.size(), 2);
    QVERIFY(page->icon().isNull());
    QVERIFY(page->iconUrl().isEmpty());

    {
        bool iconRequestDone = false;
        profile.requestIconForIconURL(QUrl("qrc:/resources/icons/qt144.png"), 0,
                                      [touchIconsEnabled, &iconRequestDone](const QIcon &icon, const QUrl &iconUrl) {
            if (touchIconsEnabled) {
                QVERIFY(!icon.isNull());
                QCOMPARE(icon.pixmap(QSize(32, 32), 1.0).toImage().pixel(16, 16), 0xfff2f9ec);
            } else {
                QVERIFY(icon.isNull());
            }

            QCOMPARE(iconUrl, QUrl("qrc:/resources/icons/qt144.png"));
            iconRequestDone = true;
        });
        QTRY_VERIFY(iconRequestDone);
    }

    {
        bool iconRequestDone = false;
        profile.requestIconForIconURL(QUrl("qrc:/resources/icons/qt32.ico"), 0,
                                      [&iconRequestDone](const QIcon &icon, const QUrl &iconUrl) {
            QVERIFY(!icon.isNull());
            QCOMPARE(icon.pixmap(QSize(32, 32), 1.0).toImage().pixel(16, 16), 0xffeef7e6);
            QCOMPARE(iconUrl, QUrl("qrc:/resources/icons/qt32.ico"));
            iconRequestDone = true;
        });
        QTRY_VERIFY(iconRequestDone);
    }
}

void tst_Favicon::requestIconForPageURL_data()
{
    QTest::addColumn<bool>("touchIconsEnabled");
    QTest::newRow("touch icons enabled") << true;
    QTest::newRow("touch icons disabled") << false;
}

void tst_Favicon::requestIconForPageURL()
{
    QFETCH(bool, touchIconsEnabled);

    QTemporaryDir tmpDir;
    QWebEngineProfile profile("iconDatabase-pageurl");
    profile.setPersistentStoragePath(tmpDir.path());
    profile.settings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
    profile.settings()->setAttribute(QWebEngineSettings::TouchIconsEnabled, touchIconsEnabled);


    QWebEngineView view;
    QWebEnginePage *page = new QWebEnginePage(&profile, &view);
    view.setPage(page);

    QSignalSpy loadFinishedSpy(page, SIGNAL(loadFinished(bool)));
    QSignalSpy iconUrlChangedSpy(page, SIGNAL(iconUrlChanged(QUrl)));
    QSignalSpy iconChangedSpy(page, SIGNAL(iconChanged(QIcon)));

    page->load(QUrl("qrc:/resources/favicon-misc.html"));

    QTRY_COMPARE(loadFinishedSpy.size(), 1);
    QTRY_COMPARE(iconUrlChangedSpy.size(), 1);
    QTRY_COMPARE(iconChangedSpy.size(), 1);

    page->load(QUrl("about:blank"));

    QTRY_COMPARE(loadFinishedSpy.size(), 2);
    QTRY_COMPARE(iconUrlChangedSpy.size(), 2);
    QTRY_COMPARE(iconChangedSpy.size(), 2);
    QVERIFY(page->icon().isNull());
    QVERIFY(page->iconUrl().isEmpty());

    {
        bool iconRequestDone = false;
        profile.requestIconForPageURL(QUrl("qrc:/resources/favicon-misc.html"), 0,
                                      [touchIconsEnabled, &iconRequestDone](const QIcon &icon, const QUrl &iconUrl, const QUrl &pageUrl) {
            QVERIFY(!icon.isNull());
            if (touchIconsEnabled) {
                QCOMPARE(icon.pixmap(QSize(32, 32), 1.0).toImage().pixel(16, 16), 0xfff2f9ec);
                QCOMPARE(iconUrl, QUrl("qrc:/resources/icons/qt144.png"));
            } else {
                QCOMPARE(icon.pixmap(QSize(32, 32), 1.0).toImage().pixel(16, 16), 0xffeef7e6);
                QCOMPARE(iconUrl, QUrl("qrc:/resources/icons/qt32.ico"));
            }

            QCOMPARE(pageUrl, QUrl("qrc:/resources/favicon-misc.html"));
            iconRequestDone = true;
        });
        QTRY_VERIFY(iconRequestDone);
    }
}

void tst_Favicon::desiredSize()
{
    QTemporaryDir tmpDir;
    QWebEngineProfile profile("iconDatabase-desiredsize");
    profile.setPersistentStoragePath(tmpDir.path());
    profile.settings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);

    QWebEngineView view;
    QWebEnginePage *page = new QWebEnginePage(&profile, &view);
    view.setPage(page);

    // Disable touch icons: icon with size 16x16 will be loaded.
    {
        profile.settings()->setAttribute(QWebEngineSettings::TouchIconsEnabled, false);

        QSignalSpy loadFinishedSpy(page, SIGNAL(loadFinished(bool)));
        QSignalSpy iconUrlChangedSpy(page, SIGNAL(iconUrlChanged(QUrl)));
        QSignalSpy iconChangedSpy(page, SIGNAL(iconChanged(QIcon)));

        page->load(QUrl("qrc:/resources/favicon-multi.html"));

        QTRY_COMPARE(loadFinishedSpy.size(), 1);
        QTRY_COMPARE(iconUrlChangedSpy.size(), 1);
        QTRY_COMPARE(iconChangedSpy.size(), 1);

        page->load(QUrl("about:blank"));

        QTRY_COMPARE(loadFinishedSpy.size(), 2);
        QTRY_COMPARE(iconUrlChangedSpy.size(), 2);
        QTRY_COMPARE(iconChangedSpy.size(), 2);
        QVERIFY(page->icon().isNull());
        QVERIFY(page->iconUrl().isEmpty());
    }

    int desiredSizeInPixel = 16;
    QRgb expectedPixel = 0xfffdfefc;

    // Request icon with size 16x16 (desiredSizeInPixel).
    {
        bool iconRequestDone = false;
        profile.requestIconForPageURL(QUrl("qrc:/resources/favicon-multi.html"), desiredSizeInPixel,
                                      [desiredSizeInPixel, expectedPixel, &iconRequestDone](const QIcon &icon, const QUrl &iconUrl, const QUrl &pageUrl) {
            QVERIFY(!icon.isNull());
            QRgb pixel = icon.pixmap(QSize(desiredSizeInPixel, desiredSizeInPixel), 1.0)
                            .toImage()
                            .pixel(8, 8);
            QCOMPARE(pixel, expectedPixel);
            QCOMPARE(iconUrl, QUrl("qrc:/resources/icons/qtmulti.ico"));
            QCOMPARE(pageUrl, QUrl("qrc:/resources/favicon-multi.html"));
            iconRequestDone = true;
        });
        QTRY_VERIFY(iconRequestDone);
    }

    // Enable touch icons: icon with the largest size (64x64) will be loaded.
    {
        profile.settings()->setAttribute(QWebEngineSettings::TouchIconsEnabled, true);

        QSignalSpy loadFinishedSpy(page, SIGNAL(loadFinished(bool)));
        QSignalSpy iconUrlChangedSpy(page, SIGNAL(iconUrlChanged(QUrl)));
        QSignalSpy iconChangedSpy(page, SIGNAL(iconChanged(QIcon)));

        page->load(QUrl("qrc:/resources/favicon-multi.html"));

        QTRY_COMPARE(loadFinishedSpy.size(), 1);
        QTRY_COMPARE(iconUrlChangedSpy.size(), 1);
        QTRY_COMPARE(iconChangedSpy.size(), 1);

        page->load(QUrl("about:blank"));

        QTRY_COMPARE(loadFinishedSpy.size(), 2);
        QTRY_COMPARE(iconUrlChangedSpy.size(), 2);
        QTRY_COMPARE(iconChangedSpy.size(), 2);
        QVERIFY(page->icon().isNull());
        QVERIFY(page->iconUrl().isEmpty());
    }

    // Request icon with size 16x16.
    // The icon is stored with two sizes in the database. This request should result same pixel
    // as the first one.
    {
        bool iconRequestDone = false;
        profile.requestIconForPageURL(QUrl("qrc:/resources/favicon-multi.html"), desiredSizeInPixel,
                                      [desiredSizeInPixel, expectedPixel, &iconRequestDone](const QIcon &icon, const QUrl &iconUrl, const QUrl &pageUrl) {
            QVERIFY(!icon.isNull());
            QRgb pixel = icon.pixmap(QSize(desiredSizeInPixel, desiredSizeInPixel), 1.0)
                            .toImage()
                            .pixel(8, 8);
            QCOMPARE(pixel, expectedPixel);
            QCOMPARE(iconUrl, QUrl("qrc:/resources/icons/qtmulti.ico"));
            QCOMPARE(pageUrl, QUrl("qrc:/resources/favicon-multi.html"));
            iconRequestDone = true;
        });
        QTRY_VERIFY(iconRequestDone);
    }

    // Request icon with size 64x64.
    // This requests the another size from the database. The pixel should differ.
    {
        bool iconRequestDone = false;
        profile.requestIconForPageURL(QUrl("qrc:/resources/favicon-multi.html"), 64,
                                      [desiredSizeInPixel, expectedPixel, &iconRequestDone](const QIcon &icon, const QUrl &iconUrl, const QUrl &pageUrl) {
            QVERIFY(!icon.isNull());
            QRgb pixel = icon.pixmap(QSize(desiredSizeInPixel, desiredSizeInPixel), 1.0)
                            .toImage()
                            .pixel(8, 8);
            QVERIFY(pixel != expectedPixel);
            QCOMPARE(iconUrl, QUrl("qrc:/resources/icons/qtmulti.ico"));
            QCOMPARE(pageUrl, QUrl("qrc:/resources/favicon-multi.html"));
            iconRequestDone = true;
        });
        QTRY_VERIFY(iconRequestDone);
    }
}

QTEST_MAIN(tst_Favicon)

#include "tst_favicon.moc"
