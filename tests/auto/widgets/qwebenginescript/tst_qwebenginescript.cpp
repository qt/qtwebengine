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
#include <qwebenginescript.h>
#include <qwebenginescriptcollection.h>
#include <qwebengineview.h>
#include "../util.h"

//#define DEBUG_SCRIPT_MESSAGES
#ifdef DEBUG_SCRIPT_MESSAGES
class WebEnginePage : public QWebEnginePage {
    void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString &message, int lineNumber, const QString &sourceID) {
        qDebug() << level << message << lineNumber << sourceID;
    }

};
#else
typedef QWebEnginePage WebEnginePage;
#endif

class tst_QWebEngineScript: public QObject {
    Q_OBJECT

private Q_SLOTS:
    void domEditing();
    void injectionPoint();
    void injectionPoint_data();
    void scriptWorld();
    void scriptModifications();

};


void tst_QWebEngineScript::domEditing()
{
    WebEnginePage page;
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
    waitForSignal(&page, SIGNAL(loadFinished(bool)));
    QCOMPARE(evaluateJavaScriptSync(&page, "document.getElementById(\"banner\").innerText"), QVariant(QStringLiteral("Injected banner")));
    // elementFromPoint only works for exposed elements
    QTest::qWaitForWindowExposed(&view);
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
    WebEnginePage page;
    page.scripts().insert(s);
    page.setHtml(QStringLiteral("<html><head><script> var contents;") + testScript
                 + QStringLiteral("document.addEventListener(\"load\", setTimeout(function(event) {\
                                   document.body.innerText = contents;\
                                   }, 550));\
                                   </script></head><body></body></html>"));
    waitForSignal(&page, SIGNAL(loadFinished(bool)));
    QTest::qWait(550);
    QCOMPARE(evaluateJavaScriptSync(&page, "document.body.innerText"), QVariant::fromValue(QStringLiteral("SUCCESS")));
}

void tst_QWebEngineScript::injectionPoint_data()
{
    QTest::addColumn<int>("injectionPoint");
    QTest::addColumn<QString>("testScript");
    QTest::newRow("DocumentCreation") << static_cast<int>(QWebEngineScript::DocumentCreation)
                                      << QStringLiteral("var contents = (typeof(foo) == \"undefined\")? \"FAILURE\" : \"SUCCESS\";");
    QTest::newRow("DocumentReady") << static_cast<int>(QWebEngineScript::DocumentReady)
    // use a zero timeout to make sure the user script got a chance to run as the order is undefined.
                                   << QStringLiteral("document.addEventListener(\"DOMContentLoaded\", setTimeout(function(event) {\
                                                      contents = (typeof(foo) == \"undefined\")? \"FAILURE\" : \"SUCCESS\";\
                                                      }, 0));");
    QTest::newRow("Deferred") << static_cast<int>(QWebEngineScript::Deferred)
                              << QStringLiteral("document.addEventListener(\"load\", setTimeout(function(event) {\
                                                 contents = (typeof(foo) == \"undefined\")? \"FAILURE\" : \"SUCCESS\";\
                                                 }, 500));");
}

void tst_QWebEngineScript::scriptWorld()
{
    WebEnginePage page;
    QWebEngineScript script;
    script.setInjectionPoint(QWebEngineScript::DocumentCreation);
    script.setWorldId(QWebEngineScript::MainWorld);
    script.setSourceCode(QStringLiteral("var userScriptTest = 1;"));
    page.scripts().insert(script);
    page.load(QUrl("about:blank"));
    waitForSignal(&page, SIGNAL(loadFinished(bool)));
    QCOMPARE(evaluateJavaScriptSync(&page, "typeof(userScriptTest) != \"undefined\" && userScriptTest == 1;"), QVariant::fromValue(true));
    script.setWorldId(QWebEngineScript::ApplicationWorld);
    page.scripts().clear();
    page.scripts().insert(script);
    page.load(QUrl("about:blank"));
    waitForSignal(&page, SIGNAL(loadFinished(bool)));
    QCOMPARE(evaluateJavaScriptSync(&page, "typeof(userScriptTest) == \"undefined\""), QVariant::fromValue(true));
}

void tst_QWebEngineScript::scriptModifications()
{
    WebEnginePage page;
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
    waitForSignal(&page, SIGNAL(loadFinished(bool)));
    QCOMPARE(evaluateJavaScriptSync(&page, "document.body.innerText"), QVariant::fromValue(QStringLiteral("SUCCESS")));
    script.setSourceCode("var foo = \"FAILURE\"");
    page.triggerAction(QWebEnginePage::ReloadAndBypassCache);
    waitForSignal(&page, SIGNAL(loadFinished(bool)));
    QCOMPARE(evaluateJavaScriptSync(&page, "document.body.innerText"), QVariant::fromValue(QStringLiteral("SUCCESS")));
    QVERIFY(page.scripts().count() == 1);
    QWebEngineScript s = page.scripts().findScript(QStringLiteral("String1"));
    QVERIFY(page.scripts().remove(s));
    QVERIFY(page.scripts().count() == 0);
}

QTEST_MAIN(tst_QWebEngineScript)

#include "tst_qwebenginescript.moc"
