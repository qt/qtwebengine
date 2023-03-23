/*
    Copyright (C) 2015 The Qt Company Ltd.

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

#include <QtTest/QtTest>
#include <QtWebEngineCore/qtwebenginecore-config.h>
#include <qwebenginepage.h>
#include <qwebengineprofile.h>
#include <qwebenginescript.h>
#include <qwebenginescriptcollection.h>
#include <qwebenginesettings.h>
#include <qwebengineview.h>
#include <util.h>
#if QT_CONFIG(webengine_webchannel)
#include <QWebChannel>
#endif

static bool verifyOrder(QStringList orderList)
{
    QStringList expected = {
        "DocumentCreation",
        "DOMContentLoaded",
        "DocumentReady",
        "load",
        "Deferred"
    };

    if (orderList.size() != 5)
        return false;
    if (orderList.at(4) == "load (timeout)") {
        if (orderList.at(3) != "Deferred")
            return false;
        expected[3] = "Deferred";
        expected[4] = "load (timeout)";
    }

    return orderList == expected;
}

class tst_QWebEngineScript: public QObject {
    Q_OBJECT

private Q_SLOTS:
    void domEditing();
    void loadEvents();
    void scriptWorld_data();
    void scriptWorld();
    void scriptDisabled();
    void viewSource();
    void scriptModifications();
#if QT_CONFIG(webengine_webchannel)
    void webChannel_data();
    void webChannel();
    void webChannelResettingAndUnsetting();
    void webChannelWithExistingQtObject();
    void navigation();
    void webChannelWithBadString();
    void webChannelWithJavaScriptDisabled();
#endif
    void noTransportWithoutWebChannel();
    void scriptsInNestedIframes();
    void matchQrcUrl();
    void injectionOrder();
};

void tst_QWebEngineScript::domEditing()
{
    QWebEnginePage page;
    QWebEngineView view;
    view.setPage(&page);
    QWebEngineScript s;
    s.setInjectionPoint(QWebEngineScript::DocumentReady);
    s.setWorldId(QWebEngineScript::ApplicationWorld + 1);
    s.setSourceCode("el = document.createElement(\"div\");\
                el.id = \"banner\";\
                el.style.position = \"absolute\";\
                el.style.width = \"100%\";\
                el.style.padding = \"1em\";\
                el.style.textAlign = \"center\";\
                el.style.top = \"0\";\
                el.style.left = \"0\";\
                el.style.backgroundColor = \"#80C342\";\
                el.innerText = \"Injected banner\";\
                document.body.appendChild(el);");
    page.scripts().insert(s);
    page.load(QUrl("about:blank"));
    view.show();
    QSignalSpy spyFinished(&page, &QWebEnginePage::loadFinished);
    QVERIFY(spyFinished.wait());
    QCOMPARE(evaluateJavaScriptSync(&page, "document.getElementById(\"banner\").innerText"), QVariant(QStringLiteral("Injected banner")));
    // elementFromPoint only works for exposed elements
    QVERIFY(QTest::qWaitForWindowExposed(&view));
    QCOMPARE(evaluateJavaScriptSync(&page, "document.elementFromPoint(2, 2).id"), QVariant::fromValue(QStringLiteral("banner")));
}

void tst_QWebEngineScript::loadEvents()
{
    // Test relative order of injection and loading.
    //
    // - install event listeners for "DOMContentLoaded" and "load" events
    // - install user scripts for every injection point
    // - check that event listeners and user scripts execute in the expected order

    class Page;
    class Profile : public QWebEngineProfile {
        QWebEngineScript scriptFor(QWebEngineScript::ScriptWorldId worldId,
                                   QWebEngineScript::InjectionPoint injectionPoint) {
            QWebEngineScript script;
            script.setWorldId(worldId);
            script.setInjectionPoint(injectionPoint);
            script.setRunsOnSubFrames(true);
            if (injectionPoint == QWebEngineScript::DocumentCreation) {
                script.setSourceCode(QStringLiteral(R"(
                var log = ['DocumentCreation'];
                var timestamps = {'DocumentCreation': Date.now()};
                for (let type of ['DOMContentLoaded', 'load']) {
                    window.addEventListener(type, () => {
                                                    timestamps[type] = Date.now();
                                                    if (type === 'load' && log.includes('Deferred') && timestamps['Deferred'] - timestamps['DOMContentLoaded'] > 500)
                                                        log.push(type + ' (timeout)');
                                                    else
                                                        log.push(type);
                                                    });
                }
                )"));
            } else if (injectionPoint == QWebEngineScript::DocumentReady) {
                script.setSourceCode(QStringLiteral(R"(
                                                    timestamps['DocumentReady'] = Date.now();
                                                    log.push('DocumentReady');
                                                    )"));
            } else {
                script.setSourceCode(QStringLiteral(R"(
                                                    timestamps['Deferred'] = Date.now();
                                                    log.push('Deferred');
                                                    )"));
            }
            return script;
        }
    public:
        Profile() {
            scripts()->insert(scriptFor(QWebEngineScript::MainWorld, QWebEngineScript::DocumentCreation));
            scripts()->insert(scriptFor(QWebEngineScript::MainWorld, QWebEngineScript::DocumentReady));
            scripts()->insert(scriptFor(QWebEngineScript::MainWorld, QWebEngineScript::Deferred));
            scripts()->insert(scriptFor(QWebEngineScript::ApplicationWorld, QWebEngineScript::DocumentCreation));
            scripts()->insert(scriptFor(QWebEngineScript::ApplicationWorld, QWebEngineScript::DocumentReady));
            scripts()->insert(scriptFor(QWebEngineScript::ApplicationWorld, QWebEngineScript::Deferred));
        }
        std::list<Page> pages;
    };

    class Page : public QWebEnginePage {
        QWebEnginePage *createWindow(WebWindowType) override {
            profile.pages.emplace_back(profile);
            return &profile.pages.back();
        };
    public:
        Page(Profile &profile) : QWebEnginePage(&profile), profile(profile) {}
        QVariant eval(const QString &code, QWebEngineScript::ScriptWorldId worldId)
        {
            return evaluateJavaScriptSyncInWorld(this, code, worldId);
        }
        Profile &profile;
        QSignalSpy spy{this, &QWebEnginePage::loadFinished};
    };

    Profile profile;
    profile.pages.emplace_back(profile);
    Page &page = profile.pages.back();

    // Single frame / setHtml
    page.setHtml(QStringLiteral("<!DOCTYPE html><html><head><title>mr</title></head><body></body></html>"));
    QTRY_COMPARE_WITH_TIMEOUT(page.spy.size(), 1, 20000);
    QVERIFY(page.spy.takeFirst().value(0).toBool());
    QVERIFY(verifyOrder(page.eval("window.log", QWebEngineScript::MainWorld).toStringList()));
    QVERIFY(verifyOrder(page.eval("window.log", QWebEngineScript::ApplicationWorld).toStringList()));

    // After discard
    page.setLifecycleState(QWebEnginePage::LifecycleState::Discarded);
    page.setLifecycleState(QWebEnginePage::LifecycleState::Active);
    QTRY_COMPARE_WITH_TIMEOUT(page.spy.size(), 1, 20000);
    QVERIFY(page.spy.takeFirst().value(0).toBool());
    QVERIFY(verifyOrder(page.eval("window.log", QWebEngineScript::MainWorld).toStringList()));
    QVERIFY(verifyOrder(page.eval("window.log", QWebEngineScript::ApplicationWorld).toStringList()));

    // Multiple frames
    page.load(QUrl("qrc:/resources/test_iframe_main.html"));
    QTRY_COMPARE_WITH_TIMEOUT(page.spy.size(), 1, 20000);
    QVERIFY(page.spy.takeFirst().value(0).toBool());
    QVERIFY(verifyOrder(page.eval("window.log", QWebEngineScript::MainWorld).toStringList()));
    QVERIFY(verifyOrder(page.eval("window.log", QWebEngineScript::ApplicationWorld).toStringList()));
    QVERIFY(verifyOrder(page.eval("window[0].log", QWebEngineScript::MainWorld).toStringList()));
    QVERIFY(verifyOrder(page.eval("window[0].log", QWebEngineScript::ApplicationWorld).toStringList()));
    QVERIFY(verifyOrder(page.eval("window[0][0].log", QWebEngineScript::MainWorld).toStringList()));
    QVERIFY(verifyOrder(page.eval("window[0][0].log", QWebEngineScript::ApplicationWorld).toStringList()));

    // Cross-process navigation
    page.load(QUrl("chrome://gpu"));
    QTRY_COMPARE_WITH_TIMEOUT(page.spy.size(), 1, 20000);
    QVERIFY(page.spy.takeFirst().value(0).toBool());
    QVERIFY(verifyOrder(page.eval("window.log", QWebEngineScript::MainWorld).toStringList()));
    QVERIFY(verifyOrder(page.eval("window.log", QWebEngineScript::ApplicationWorld).toStringList()));

    // Using window.open from JS
    QVERIFY(profile.pages.size() == 1);
    page.load(QUrl("qrc:/resources/test_window_open.html"));
    QTRY_COMPARE(profile.pages.size(), 2u);
    QTRY_COMPARE(profile.pages.front().spy.size(), 1);
    QTRY_COMPARE(profile.pages.back().spy.size(), 1);
    QVERIFY(verifyOrder(profile.pages.front().eval("window.log", QWebEngineScript::MainWorld).toStringList()));
    QVERIFY(verifyOrder(profile.pages.front().eval("window.log", QWebEngineScript::ApplicationWorld).toStringList()));
    QVERIFY(verifyOrder(profile.pages.back().eval("window.log", QWebEngineScript::MainWorld).toStringList()));
    QVERIFY(verifyOrder(profile.pages.back().eval("window.log", QWebEngineScript::ApplicationWorld).toStringList()));
}

void tst_QWebEngineScript::scriptWorld_data()
{
    QTest::addColumn<int>("worldId");

    QTest::newRow("ApplicationWorld") << static_cast<int>(QWebEngineScript::ApplicationWorld);
    QTest::newRow("UserWorld") << static_cast<int>(QWebEngineScript::UserWorld);
    QTest::newRow("150") << 150;
}

void tst_QWebEngineScript::scriptWorld()
{
    QFETCH(int, worldId);
    QWebEnginePage page;
    QWebEngineScript script;
    script.setInjectionPoint(QWebEngineScript::DocumentCreation);
    script.setWorldId(QWebEngineScript::MainWorld);
    script.setSourceCode(QStringLiteral("var userScriptTest = 1;"));
    page.scripts().insert(script);
    page.load(QUrl("about:blank"));
    QSignalSpy spyFinished(&page, &QWebEnginePage::loadFinished);
    QVERIFY(spyFinished.wait());
    QCOMPARE(evaluateJavaScriptSync(&page, "typeof(userScriptTest) != \"undefined\" && userScriptTest == 1;"), QVariant::fromValue(true));
    QCOMPARE(evaluateJavaScriptSyncInWorld(&page, "typeof(userScriptTest) == \"undefined\"", worldId), QVariant::fromValue(true));
    script.setWorldId(worldId);
    page.scripts().clear();
    page.scripts().insert(script);
    page.load(QUrl("about:blank"));
    QVERIFY(spyFinished.wait());
    QCOMPARE(evaluateJavaScriptSync(&page, "typeof(userScriptTest) == \"undefined\""), QVariant::fromValue(true));
    QCOMPARE(evaluateJavaScriptSyncInWorld(&page, "typeof(userScriptTest) != \"undefined\" && userScriptTest == 1;", worldId), QVariant::fromValue(true));
}

// Based on QTBUG-74304
void tst_QWebEngineScript::scriptDisabled()
{
    QWebEnginePage page;
    page.settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, false);
    QWebEngineScript script;
    script.setInjectionPoint(QWebEngineScript::DocumentCreation);
    script.setWorldId(QWebEngineScript::MainWorld);
    script.setSourceCode("var foo = 42");
    page.scripts().insert(script);
    page.load(QUrl("about:blank"));
    QSignalSpy spy(&page, &QWebEnginePage::loadFinished);
    QTRY_COMPARE(spy.size(), 1);
    QCOMPARE(spy.takeFirst().value(0).toBool(), true);
    // MainWorld scripts are disabled by the setting...
    QCOMPARE(evaluateJavaScriptSyncInWorld(&page, "foo", QWebEngineScript::MainWorld), QVariant());
    QCOMPARE(evaluateJavaScriptSyncInWorld(&page, "foo", QWebEngineScript::ApplicationWorld), QVariant());
    script.setWorldId(QWebEngineScript::ApplicationWorld);
    page.scripts().clear();
    page.scripts().insert(script);
    page.load(QUrl("about:blank"));
    QTRY_COMPARE(spy.size(), 1);
    QCOMPARE(spy.takeFirst().value(0).toBool(), true);
    // ...but ApplicationWorld scripts should still work
    QCOMPARE(evaluateJavaScriptSyncInWorld(&page, "foo", QWebEngineScript::MainWorld), QVariant());
    QCOMPARE(evaluateJavaScriptSyncInWorld(&page, "foo", QWebEngineScript::ApplicationWorld), QVariant(42));
}

// Based on QTBUG-66011
void tst_QWebEngineScript::viewSource()
{
    QWebEnginePage page;
    QWebEngineScript script;
    script.setInjectionPoint(QWebEngineScript::DocumentCreation);
    script.setWorldId(QWebEngineScript::MainWorld);
    script.setSourceCode("var foo = 42");
    page.scripts().insert(script);
    page.load(QUrl("view-source:about:blank"));
    QSignalSpy spy(&page, &QWebEnginePage::loadFinished);
    QTRY_COMPARE(spy.size(), 1);
    QCOMPARE(spy.takeFirst().value(0).toBool(), true);
    QCOMPARE(evaluateJavaScriptSync(&page, "foo"), QVariant(42));
}

void tst_QWebEngineScript::scriptModifications()
{
    QWebEnginePage page;
    QWebEngineScript script;
    script.setName(QStringLiteral("String1"));
    script.setInjectionPoint(QWebEngineScript::DocumentCreation);
    script.setWorldId(QWebEngineScript::MainWorld);
    script.setSourceCode("var foo = \"SUCCESS\";");
    page.scripts().insert(script);
    page.setHtml(QStringLiteral("<html><head><script>document.addEventListener(\"DOMContentLoaded\", function() {\
                                 document.body.innerText = foo;});\
                                 </script></head><body></body></html>"));
    QVERIFY(page.scripts().count() == 1);
    QSignalSpy spyFinished(&page, &QWebEnginePage::loadFinished);
    QVERIFY(spyFinished.wait());
    QCOMPARE(evaluateJavaScriptSync(&page, "document.body.innerText"), QVariant::fromValue(QStringLiteral("SUCCESS")));
    script.setSourceCode("var foo = \"FAILURE\"");
    page.triggerAction(QWebEnginePage::ReloadAndBypassCache);
    QVERIFY(spyFinished.wait());
    QCOMPARE(evaluateJavaScriptSync(&page, "document.body.innerText"), QVariant::fromValue(QStringLiteral("SUCCESS")));
    QVERIFY(page.scripts().count() == 1);
    QList<QWebEngineScript> s = page.scripts().find(QStringLiteral("String1"));
    QVERIFY(page.scripts().remove(s.first()));
    QVERIFY(page.scripts().count() == 0);
}

class TestObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
public:
    TestObject(QObject *parent = 0) : QObject(parent) { }

    void setText(const QString &text)
    {
        if (text == m_text)
            return;
        m_text = text;
        emit textChanged(text);
    }

    QString text() const { return m_text; }

signals:
    void textChanged(const QString &text);

private:
    QString m_text;
};

#if QT_CONFIG(webengine_webchannel)
static QString readFile(const QString &path)
{
    QFile file(path);
    file.open(QFile::ReadOnly);
    QByteArray contents = file.readAll();
    file.close();
    return contents;
}

static QWebEngineScript webChannelScript()
{
    QString sourceCode = readFile(QStringLiteral(":/qtwebchannel/qwebchannel.js"));
    Q_ASSERT(!sourceCode.isEmpty());

    QWebEngineScript script;
    script.setSourceCode(sourceCode);
    script.setInjectionPoint(QWebEngineScript::DocumentCreation);
    script.setWorldId(QWebEngineScript::MainWorld);
    return script;
}

void tst_QWebEngineScript::webChannel_data()
{
    QTest::addColumn<int>("worldId");
    QTest::addColumn<bool>("reloadFirst");
    QTest::newRow("MainWorld") << static_cast<int>(QWebEngineScript::MainWorld) << false;
    QTest::newRow("ApplicationWorld") << static_cast<int>(QWebEngineScript::ApplicationWorld) << false;
    QTest::newRow("MainWorldWithReload") << static_cast<int>(QWebEngineScript::MainWorld) << true;
    QTest::newRow("ApplicationWorldWithReload") << static_cast<int>(QWebEngineScript::ApplicationWorld) << true;
}

void tst_QWebEngineScript::webChannel()
{
    QFETCH(int, worldId);
    QFETCH(bool, reloadFirst);
    QWebEnginePage page;
    TestObject testObject;
    QScopedPointer<QWebChannel> channel(new QWebChannel(this));
    channel->registerObject(QStringLiteral("object"), &testObject);
    page.setWebChannel(channel.data(), worldId);

    QWebEngineScript script = webChannelScript();
    script.setWorldId(worldId);
    page.scripts().insert(script);
    page.setHtml(QStringLiteral("<html><body></body></html>"));
    QSignalSpy spyFinished(&page, &QWebEnginePage::loadFinished);
    QVERIFY(spyFinished.wait());
    if (reloadFirst) {
        // Check that the transport is also reinstalled on navigation
        page.triggerAction(QWebEnginePage::Reload);
        QVERIFY(spyFinished.wait());
    }
    page.runJavaScript(QLatin1String(
                                "new QWebChannel(qt.webChannelTransport,"
                                "  function(channel) {"
                                "    channel.objects.object.text = 'test';"
                                "  }"
                                ");"), worldId);
    QSignalSpy spyTextChanged(&testObject, &TestObject::textChanged);
    QVERIFY(spyTextChanged.wait());
    QCOMPARE(testObject.text(), QStringLiteral("test"));

    if (worldId != QWebEngineScript::MainWorld)
        QCOMPARE(evaluateJavaScriptSync(&page, "qt.webChannelTransport"), QVariant());
}
#endif
void tst_QWebEngineScript::noTransportWithoutWebChannel()
{
    QWebEnginePage page;
    page.setHtml(QStringLiteral("<html><body></body></html>"));

    QCOMPARE(evaluateJavaScriptSync(&page, "qt.webChannelTransport"), QVariant());
    page.triggerAction(QWebEnginePage::Reload);
    QSignalSpy spyFinished(&page, &QWebEnginePage::loadFinished);
    QVERIFY(spyFinished.wait());
    QCOMPARE(evaluateJavaScriptSync(&page, "qt.webChannelTransport"), QVariant());
}

void tst_QWebEngineScript::scriptsInNestedIframes()
{
    QWebEnginePage page;
    QWebEngineView view;
    view.setPage(&page);
    QWebEngineScript s;
    s.setInjectionPoint(QWebEngineScript::DocumentReady);
    s.setWorldId(QWebEngineScript::ApplicationWorld);

    // Prepend a "Modified prefix" to every frame's div content.
    s.setSourceCode("var elements = document.getElementsByTagName(\"div\");\
                    var i;\
                    for (i = 0; i < elements.length; i++) {\
                        var content = elements[i].innerHTML;\
                        elements[i].innerHTML = \"Modified \" + content;\
                    }\
                    ");

    // Make sure the script runs on all frames.
    s.setRunsOnSubFrames(true);
    page.scripts().insert(s);

    QSignalSpy spyFinished(&page, &QWebEnginePage::loadFinished);
    page.load(QUrl("qrc:/resources/test_iframe_main.html"));
    view.show();
    QTRY_VERIFY_WITH_TIMEOUT(spyFinished.size() > 0, 20000);

    // Check that main frame has modified content.
    QCOMPARE(
        evaluateJavaScriptSyncInWorld(&page, "document.getElementsByTagName(\"div\")[0].innerHTML",
                                      QWebEngineScript::ApplicationWorld),
                QVariant::fromValue(QStringLiteral("Modified Main text")));

    // Check that outer frame has modified content.
    QCOMPARE(
        evaluateJavaScriptSyncInWorld(&page,
                                      "var i = document.getElementById(\"outer\").contentDocument;\
                                       i.getElementsByTagName(\"div\")[0].innerHTML",
                                      QWebEngineScript::ApplicationWorld),
                QVariant::fromValue(QStringLiteral("Modified Outer text")));


    // Check that inner frame has modified content.
    QCOMPARE(
        evaluateJavaScriptSyncInWorld(&page,
                                      "var i = document.getElementById(\"outer\").contentDocument;\
                                       var i2 = i.getElementById(\"inner\").contentDocument;\
                                       i2.getElementsByTagName(\"div\")[0].innerHTML",
                                      QWebEngineScript::ApplicationWorld),
                QVariant::fromValue(QStringLiteral("Modified Inner text")));
}
#if QT_CONFIG(webengine_webchannel)
void tst_QWebEngineScript::webChannelResettingAndUnsetting()
{
    QWebEnginePage page;

    // There should be no webChannelTransport yet.
    QCOMPARE(evaluateJavaScriptSyncInWorld(&page, "qt.webChannelTransport", QWebEngineScript::MainWorld),
             QVariant());
    QCOMPARE(evaluateJavaScriptSyncInWorld(&page, "qt.webChannelTransport", QWebEngineScript::ApplicationWorld),
             QVariant());

    QWebChannel channel;
    page.setWebChannel(&channel, QWebEngineScript::MainWorld);

    // There should be one in MainWorld now.
    QCOMPARE(evaluateJavaScriptSyncInWorld(&page, "qt.webChannelTransport", QWebEngineScript::MainWorld),
             QVariant(QVariantMap()));
    QCOMPARE(evaluateJavaScriptSyncInWorld(&page, "qt.webChannelTransport", QWebEngineScript::ApplicationWorld),
             QVariant());

    page.setWebChannel(&channel, QWebEngineScript::ApplicationWorld);

    // Now it should have moved to ApplicationWorld.
    QCOMPARE(evaluateJavaScriptSyncInWorld(&page, "qt.webChannelTransport", QWebEngineScript::MainWorld),
             QVariant());
    QCOMPARE(evaluateJavaScriptSyncInWorld(&page, "qt.webChannelTransport", QWebEngineScript::ApplicationWorld),
             QVariant(QVariantMap()));

    page.setWebChannel(nullptr);

    // And now it should be gone again.
    QCOMPARE(evaluateJavaScriptSyncInWorld(&page, "qt.webChannelTransport", QWebEngineScript::MainWorld),
             QVariant());
    QCOMPARE(evaluateJavaScriptSyncInWorld(&page, "qt.webChannelTransport", QWebEngineScript::ApplicationWorld),
             QVariant());
}

void tst_QWebEngineScript::webChannelWithExistingQtObject()
{
    QWebEnginePage page;

    evaluateJavaScriptSync(&page, "qt = 42");
    QCOMPARE(evaluateJavaScriptSync(&page, "qt.webChannelTransport"), QVariant());

    QWebChannel channel;
    page.setWebChannel(&channel);

    // setWebChannel should have overwritten the qt variable
    QCOMPARE(evaluateJavaScriptSync(&page, "qt.webChannelTransport"), QVariant(QVariantMap()));
}

static QWebEngineScript locationMonitorScript()
{
    QWebEngineScript script = webChannelScript();
    script.setSourceCode(script.sourceCode() + QStringLiteral(R"(
        new QWebChannel(qt.webChannelTransport, channel => {
            channel.objects.object.text = window.location.href;
        })
    )"));
    return script;
}

void tst_QWebEngineScript::navigation()
{
    QWebEnginePage page;
    TestObject testObject;
    QSignalSpy spyTextChanged(&testObject, &TestObject::textChanged);
    QWebChannel channel;
    channel.registerObject(QStringLiteral("object"), &testObject);
    page.setWebChannel(&channel);
    page.scripts().insert(locationMonitorScript());

    QString url1 = QStringLiteral("about:blank");
    page.setUrl(url1);
    QTRY_COMPARE(spyTextChanged.size(), 1);
    QCOMPARE(testObject.text(), url1);

    QString url2 = QStringLiteral("chrome://gpu/");
    page.setUrl(url2);
    QTRY_COMPARE(spyTextChanged.size(), 2);
    QCOMPARE(testObject.text(), url2);

    QString url3 = QStringLiteral("qrc:/resources/test_iframe_main.html");
    page.setUrl(url3);
    QTRY_COMPARE(spyTextChanged.size(), 3);
    QCOMPARE(testObject.text(), url3);

    page.setLifecycleState(QWebEnginePage::LifecycleState::Discarded);
    page.setUrl(url1);
    QTRY_COMPARE(spyTextChanged.size(), 4);
    QCOMPARE(testObject.text(), url1);
}

// Try to set TestObject::text to an invalid UTF-16 string.
//
// See QTBUG-61969.
void tst_QWebEngineScript::webChannelWithBadString()
{
    QWebEnginePage page;
    TestObject host;
    QSignalSpy hostSpy(&host, &TestObject::textChanged);
    QWebChannel channel;
    channel.registerObject(QStringLiteral("host"), &host);
    page.setWebChannel(&channel);
    page.setUrl(QStringLiteral("qrc:/resources/webChannelWithBadString.html"));
    QVERIFY(hostSpy.wait(20000));
    // expect 0xD800 see https://chromium-review.googlesource.com/c/1282993
    QChar data(0xd800);
    QCOMPARE(host.text(), data);
}

void tst_QWebEngineScript::webChannelWithJavaScriptDisabled()
{
    QWebEnginePage page;
    QSignalSpy spyFinished(&page, &QWebEnginePage::loadFinished);
    // JavaScript disabled in main world
    page.settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, false);

    TestObject testObject;
    QScopedPointer<QWebChannel> channel(new QWebChannel(this));
    channel->registerObject(QStringLiteral("object"), &testObject);
    page.setWebChannel(channel.data(), QWebEngineScript::ApplicationWorld);

    QWebEngineScript script = webChannelScript();
    script.setWorldId(QWebEngineScript::ApplicationWorld);
    page.scripts().insert(script);

    page.setHtml(QStringLiteral("<html><body></body></html>"));
    QVERIFY(spyFinished.wait());

    QSignalSpy spyTextChanged(&testObject, &TestObject::textChanged);
    page.runJavaScript(QLatin1String(
                                "new QWebChannel(qt.webChannelTransport,"
                                "  function(channel) {"
                                "    channel.objects.object.text = 'test';"
                                "  }"
                                ");"), QWebEngineScript::ApplicationWorld);
    QVERIFY(spyTextChanged.wait());
    QCOMPARE(testObject.text(), QStringLiteral("test"));
}
#endif

void tst_QWebEngineScript::matchQrcUrl()
{
    QWebEngineProfile profile;
    QWebEnginePage page(&profile);
    QWebEngineScript s;
    s.setInjectionPoint(QWebEngineScript::DocumentReady);
    s.setWorldId(QWebEngineScript::MainWorld);
    s.setSourceCode(QStringLiteral(R"(
// ==UserScript==
// @match qrc:/*title_b.html
// ==/UserScript==

document.title = 'New title';
    )"));
    page.scripts().insert(s);
    loadSync(&page, QUrl("qrc:/resources/title_a.html"));
    QCOMPARE(page.title(), "A");
    loadSync(&page, QUrl("qrc:/resources/title_b.html"));
    QCOMPARE(page.title(), "New title");
}

// Add many scripts and check order of execution.
void tst_QWebEngineScript::injectionOrder()
{
    QWebEngineProfile profile;
    class Page : public QWebEnginePage
    {
    public:
        Page(QWebEngineProfile *profile) : QWebEnginePage(profile) {}
        QVector<QString> log;

    protected:
        void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel, const QString &message, int,
                                      const QString &) override
        {
            log.append(message);
        }
    } page(&profile);
    QWebEngineScript::InjectionPoint points[] = {
        QWebEngineScript::DocumentCreation,
        QWebEngineScript::DocumentReady,
        QWebEngineScript::Deferred,
    };
    int nPoints = 3;
    int nCollections = 2;
    int nScripts = 5;

    QVector<QString> expected;
    for (int iPoint = 0; iPoint != nPoints; ++iPoint) {
        for (int iCollection = 0; iCollection != nCollections; ++iCollection) {
            for (int iScript = 0; iScript != nScripts; ++iScript) {
                QWebEngineScript script;
                script.setName(QString("%1%2%3").arg(iPoint).arg(iCollection).arg(iScript));
                expected.append(script.name());
                script.setInjectionPoint(points[iPoint]);
                script.setWorldId(QWebEngineScript::MainWorld);
                script.setSourceCode(QStringLiteral("console.error('%1');").arg(script.name()));
                if (iCollection == 0)
                    profile.scripts()->insert(script);
                else
                    page.scripts().insert(script);
            }
        }
    }

    page.load(QUrl("qrc:/resources/test_iframe_inner.html"));
    QTRY_COMPARE(page.log, expected);
}

QTEST_MAIN(tst_QWebEngineScript)

#include "tst_qwebenginescript.moc"
