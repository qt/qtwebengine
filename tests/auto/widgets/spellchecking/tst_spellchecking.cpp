// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <util.h>
#include <QtTest/QtTest>
#include <QtWebEngineCore/qwebenginecontextmenurequest.h>
#include <QtWebEngineCore/qwebenginepage.h>
#include <QtWebEngineCore/qwebengineprofile.h>
#include <QtWebEngineCore/qwebenginesettings.h>
#include <QtWebEngineWidgets/qwebengineview.h>

class WebView : public QWebEngineView
{
    Q_OBJECT
public:
    void activateMenu(QWidget *widget, const QPoint &position)
    {
        QTest::mouseMove(widget, position);
        QTest::mousePress(widget, Qt::RightButton, {}, position);
        QContextMenuEvent evcont(QContextMenuEvent::Mouse, position, mapToGlobal(position));
        event(&evcont);
        QTest::mouseRelease(widget, Qt::RightButton, {}, position);
    }

    QWebEngineContextMenuRequest *data() { return m_data; }

signals:
    void menuReady();

protected:
    void contextMenuEvent(QContextMenuEvent *) override
    {
        m_data = lastContextMenuRequest();
        emit menuReady();
    }
private:
    QWebEngineContextMenuRequest *m_data;
};

class tst_Spellchecking : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void init();
    void cleanup();
    void settings();
    void spellcheck();
    void spellcheck_data();

private:
    void load();
    WebView *m_view;
};

void tst_Spellchecking::init()
{
    m_view = new WebView();
}

void tst_Spellchecking::load()
{
    m_view->page()->load(QUrl("qrc:///resources/index.html"));
    m_view->show();
    QSignalSpy spyFinished(m_view->page(), &QWebEnginePage::loadFinished);
    QVERIFY(spyFinished.wait());
}

void tst_Spellchecking::cleanup()
{
    delete m_view;
}

void tst_Spellchecking::settings()
{
    // Default profile has spellchecking disabled

    QVERIFY(!QWebEngineProfile::defaultProfile()->isSpellCheckEnabled());
    QVERIFY(QWebEngineProfile::defaultProfile()->spellCheckLanguages().isEmpty());

    // New named profiles have spellchecking disabled

    auto profile1 = std::make_unique<QWebEngineProfile>(QStringLiteral("Profile1"));
    QVERIFY(!profile1->isSpellCheckEnabled());
    QVERIFY(profile1->spellCheckLanguages().isEmpty());

    auto profile2 = std::make_unique<QWebEngineProfile>(QStringLiteral("Profile2"));
    QVERIFY(!profile2->isSpellCheckEnabled());
    QVERIFY(profile2->spellCheckLanguages().isEmpty());

    // New otr profiles have spellchecking disabled

    auto profile3 = std::make_unique<QWebEngineProfile>();
    QVERIFY(!profile2->isSpellCheckEnabled());
    QVERIFY(profile2->spellCheckLanguages().isEmpty());

    // Settings can be changed

    profile1->setSpellCheckEnabled(true);
    QVERIFY(profile1->isSpellCheckEnabled());

    profile1->setSpellCheckLanguages({"en-US"});
    QVERIFY(profile1->spellCheckLanguages() == QStringList({"en-US"}));

    profile1->setSpellCheckLanguages({"en-US","de-DE"});
    QVERIFY(profile1->spellCheckLanguages() == QStringList({"en-US","de-DE"}));

    // Settings are per profile

    QVERIFY(!profile2->isSpellCheckEnabled());
    QVERIFY(profile2->spellCheckLanguages().isEmpty());

    QVERIFY(!profile3->isSpellCheckEnabled());
    QVERIFY(profile3->spellCheckLanguages().isEmpty());

    // Settings are not persisted

    // TODO(juvaldma): Write from dtor currently usually happens *after* the
    // read from the ctor, so this test would pass even if settings were
    // persisted. It would start to fail on the second run though.
    profile1.reset();
    profile1 = std::make_unique<QWebEngineProfile>(QStringLiteral("Profile1"));
    QVERIFY(!profile1->isSpellCheckEnabled());
    QVERIFY(profile1->spellCheckLanguages().isEmpty());
}

void tst_Spellchecking::spellcheck()
{
    QFETCH(QStringList, languages);
    QFETCH(QStringList, suggestions);

    m_view->settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, true);

    QWebEngineProfile *profile = QWebEngineProfile::defaultProfile();
    QVERIFY(profile);
    profile->setSpellCheckLanguages(languages);
    profile->setSpellCheckEnabled(true);
    load();
    QCOMPARE(profile->spellCheckLanguages(), languages);

    // make textarea editable
    evaluateJavaScriptSync(m_view->page(), "makeEditable();");

    // calcuate position of misspelled word
    QVariantList list = evaluateJavaScriptSync(m_view->page(), "findWordPosition('I lowe Qt ....','lowe');").toList();
    QRect rect(list[0].value<int>(),list[1].value<int>(),list[2].value<int>(),list[3].value<int>());

    QTRY_VERIFY(m_view->focusWidget());
    //type text, spellchecker needs time
    QTest::mouseMove(m_view->focusWidget(), QPoint(20,20));
    QTest::mousePress(m_view->focusWidget(), Qt::LeftButton, {}, QPoint(20,20));
    QTest::mouseRelease(m_view->focusWidget(), Qt::LeftButton, {}, QPoint(20,20));
    QString text("I lowe Qt ....");
    for (int i = 0; i < text.size(); i++) {
        QTest::keyClicks(m_view->focusWidget(), text.at(i));
        QTest::qWait(60);
    }

    // make sure text is there
    QString result = evaluateJavaScriptSync(m_view->page(), "text();").toString();
    QCOMPARE(result, text);

    bool gotMisspelledWord = false; // clumsy QTRY_VERIFY still execs expr after first success
    QString detail;

    // check that spellchecker has done text processing and filled misspelled word
    QTRY_VERIFY2([&] () {
        detail.clear();
        if (gotMisspelledWord)
            return true;

        // open menu on misspelled word
        m_view->activateMenu(m_view->focusWidget(), rect.center());
        QSignalSpy spyMenuReady(m_view, &WebView::menuReady);
        if (!spyMenuReady.wait()) {
            detail = "menu was not shown";
            return false;
        }

        if (!m_view->data()) {
            detail = "invalid data";
            return false;
        }

        if (!m_view->data()->isContentEditable()) {
            detail = "content is not editable";
            return false;
        }

        if (m_view->data()->misspelledWord().isEmpty()) {
            detail = "no misspelled word";
            return false;
        };

        gotMisspelledWord = true;
        return true;
    } (), qPrintable(QString("Context menu: %1").arg(detail)));

    // check misspelled word
    QCOMPARE(m_view->data()->misspelledWord(), QStringLiteral("lowe"));

    // check suggestions
    QCOMPARE(m_view->data()->spellCheckerSuggestions(), suggestions);

    // check replace word
    m_view->page()->replaceMisspelledWord("love");
    text = "I love Qt ....";
    QTRY_VERIFY(evaluateJavaScriptSync(m_view->page(), "text();").toString() == text);
}

void tst_Spellchecking::spellcheck_data()
{
    QTest::addColumn<QStringList>("languages");
    QTest::addColumn<QStringList>("suggestions");
    QTest::newRow("en-US") << QStringList({"en-US"}) << QStringList({"low", "love"});
    QTest::newRow("en-US,de-DE") << QStringList({"en-US", "de-DE"}) << QStringList({"löwe", "low", "love"});
}

QTEST_MAIN(tst_Spellchecking)
#include "tst_spellchecking.moc"
