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

#include <qwebenginepage.h>
#include <qwebengineprofile.h>
#include <qwebenginescript.h>
#include <qwebenginescriptcollection.h>
#include <qwebengineview.h>
#include "../util.h"
#include <QWebChannel>

class tst_QWebEngineScript: public QObject {
    Q_OBJECT

private Q_SLOTS:
    void domEditing();
    void injectionPoint();
    void injectionPoint_data();
    void scriptWorld();
    void scriptModifications();
    void webChannel_data();
    void webChannel();
    void noTransportWithoutWebChannel();
    void scriptsInNestedIframes();
    void webChannelResettingAndUnsetting();
    void webChannelWithExistingQtObject();
    void navigation();
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

void tst_QWebEngineScript::injectionPoint()
{
    QFETCH(int, injectionPoint);
    QFETCH(QString, testScript);

    QWebEngineScript s;
    s.setSourceCode("var foo = \"foobar\";");
    s.setInjectionPoint(static_cast<QWebEngineScript::InjectionPoint>(injectionPoint));
    s.setWorldId(QWebEngineScript::MainWorld);
    QWebEnginePage page;
    QSignalSpy spyFinished(&page, &QWebEnginePage::loadFinished);
    page.scripts().insert(s);
    page.setHtml(QStringLiteral("<html><head><script>") + testScript + QStringLiteral("</script></head><body></body></html>"));
    QVERIFY(spyFinished.wait());
    const QVariant expected(QVariant::fromValue(QStringLiteral("SUCCESS")));
    QTRY_COMPARE(evaluateJavaScriptSync(&page, "document.myContents"), expected);
}

void tst_QWebEngineScript::injectionPoint_data()
{
    QTest::addColumn<int>("injectionPoint");
    QTest::addColumn<QString>("testScript");
    QTest::newRow("DocumentCreation") << static_cast<int>(QWebEngineScript::DocumentCreation)
                                      << QStringLiteral("document.myContents = (typeof(foo) == \"undefined\")? \"FAILURE\" : \"SUCCESS\";");
    QTest::newRow("DocumentReady") << static_cast<int>(QWebEngineScript::DocumentReady)
    // use a zero timeout to make sure the user script got a chance to run as the order is undefined.
                                   << QStringLiteral("document.addEventListener(\"DOMContentLoaded\", function() {\
                                                        setTimeout(function() {\
                                                          document.myContents = (typeof(foo) == \"undefined\")? \"FAILURE\" : \"SUCCESS\";\
                                                        }, 0)});");
    QTest::newRow("Deferred") << static_cast<int>(QWebEngineScript::DocumentReady)
                              << QStringLiteral("document.onreadystatechange = function() { \
                                                   if (document.readyState == \"complete\") { \
                                                     setTimeout(function() {\
                                                       document.myContents = (typeof(foo) == \"undefined\")? \"FAILURE\" : \"SUCCESS\";\
                                                     }, 0);\
                                                   } \
                                                 };");
}

void tst_QWebEngineScript::scriptWorld()
{
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
    QCOMPARE(evaluateJavaScriptSyncInWorld(&page, "typeof(userScriptTest) == \"undefined\"", QWebEngineScript::ApplicationWorld), QVariant::fromValue(true));
    script.setWorldId(QWebEngineScript::ApplicationWorld);
    page.scripts().clear();
    page.scripts().insert(script);
    page.load(QUrl("about:blank"));
    QVERIFY(spyFinished.wait());
    QCOMPARE(evaluateJavaScriptSync(&page, "typeof(userScriptTest) == \"undefined\""), QVariant::fromValue(true));
    QCOMPARE(evaluateJavaScriptSyncInWorld(&page, "typeof(userScriptTest) != \"undefined\" && userScriptTest == 1;", QWebEngineScript::ApplicationWorld), QVariant::fromValue(true));
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
    QWebEngineScript s = page.scripts().findScript(QStringLiteral("String1"));
    QVERIFY(page.scripts().remove(s));
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
    QString sourceCode = readFile(QStringLiteral(":/qwebchannel.js"));
    if (sourceCode.isEmpty())
        return {};

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
        QCOMPARE(evaluateJavaScriptSync(&page, "qt.webChannelTransport"), QVariant(QVariant::Invalid));
}

void tst_QWebEngineScript::noTransportWithoutWebChannel()
{
    QWebEnginePage page;
    page.setHtml(QStringLiteral("<html><body></body></html>"));

    QCOMPARE(evaluateJavaScriptSync(&page, "qt.webChannelTransport"), QVariant(QVariant::Invalid));
    page.triggerAction(QWebEnginePage::Reload);
    QSignalSpy spyFinished(&page, &QWebEnginePage::loadFinished);
    QVERIFY(spyFinished.wait());
    QCOMPARE(evaluateJavaScriptSync(&page, "qt.webChannelTransport"), QVariant(QVariant::Invalid));
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
    QVERIFY(spyFinished.wait());

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

void tst_QWebEngineScript::webChannelResettingAndUnsetting()
{
    QWebEnginePage page;

    // There should be no webChannelTransport yet.
    QCOMPARE(evaluateJavaScriptSyncInWorld(&page, "qt.webChannelTransport", QWebEngineScript::MainWorld),
             QVariant(QVariant::Invalid));
    QCOMPARE(evaluateJavaScriptSyncInWorld(&page, "qt.webChannelTransport", QWebEngineScript::ApplicationWorld),
             QVariant(QVariant::Invalid));

    QWebChannel channel;
    page.setWebChannel(&channel, QWebEngineScript::MainWorld);

    // There should be one in MainWorld now.
    QCOMPARE(evaluateJavaScriptSyncInWorld(&page, "qt.webChannelTransport", QWebEngineScript::MainWorld),
             QVariant(QVariantMap()));
    QCOMPARE(evaluateJavaScriptSyncInWorld(&page, "qt.webChannelTransport", QWebEngineScript::ApplicationWorld),
             QVariant(QVariant::Invalid));

    page.setWebChannel(&channel, QWebEngineScript::ApplicationWorld);

    // Now it should have moved to ApplicationWorld.
    QCOMPARE(evaluateJavaScriptSyncInWorld(&page, "qt.webChannelTransport", QWebEngineScript::MainWorld),
             QVariant(QVariant::Invalid));
    QCOMPARE(evaluateJavaScriptSyncInWorld(&page, "qt.webChannelTransport", QWebEngineScript::ApplicationWorld),
             QVariant(QVariantMap()));

    page.setWebChannel(nullptr);

    // And now it should be gone again.
    QCOMPARE(evaluateJavaScriptSyncInWorld(&page, "qt.webChannelTransport", QWebEngineScript::MainWorld),
             QVariant(QVariant::Invalid));
    QCOMPARE(evaluateJavaScriptSyncInWorld(&page, "qt.webChannelTransport", QWebEngineScript::ApplicationWorld),
             QVariant(QVariant::Invalid));
}

void tst_QWebEngineScript::webChannelWithExistingQtObject()
{
    QWebEnginePage page;

    evaluateJavaScriptSync(&page, "qt = 42");
    QCOMPARE(evaluateJavaScriptSync(&page, "qt.webChannelTransport"), QVariant(QVariant::Invalid));

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
    QTRY_COMPARE(spyTextChanged.count(), 1);
    QCOMPARE(testObject.text(), url1);

    QString url2 = QStringLiteral("chrome://gpu/");
    page.setUrl(url2);
    QTRY_COMPARE(spyTextChanged.count(), 2);
    QCOMPARE(testObject.text(), url2);

    QString url3 = QStringLiteral("qrc:/resources/test_iframe_main.html");
    page.setUrl(url3);
    QTRY_COMPARE(spyTextChanged.count(), 3);
    QCOMPARE(testObject.text(), url3);
}

QTEST_MAIN(tst_QWebEngineScript)

#include "tst_qwebenginescript.moc"
