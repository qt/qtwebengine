// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <QtTest/QtTest>

#include <qwebenginepage.h>

class tst_DevTools : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void attachAndDestroyPageFirst();
    void attachAndDestroyInspectorFirst();
};

void tst_DevTools::attachAndDestroyPageFirst()
{
    // External inspector + manual destruction of page first
    QWebEnginePage* page = new QWebEnginePage();
    QWebEnginePage* inspector = new QWebEnginePage();

    QSignalSpy spy(page, &QWebEnginePage::loadFinished);
    page->load(QUrl("data:text/plain,foobarbaz"));
    QTRY_COMPARE_WITH_TIMEOUT(spy.size(), 1, 12000);

    // shouldn't do anything until page is set
    page->triggerAction(QWebEnginePage::InspectElement);

    inspector->setInspectedPage(page);
    page->triggerAction(QWebEnginePage::InspectElement);

    // This is deliberately racy:
    QTest::qWait(10);

    delete page;
    delete inspector;
}

void tst_DevTools::attachAndDestroyInspectorFirst()
{
    // External inspector + manual destruction of inspector first
    QWebEnginePage* page = new QWebEnginePage();

    // shouldn't do anything until page is set
    page->triggerAction(QWebEnginePage::InspectElement);

    QWebEnginePage* inspector = new QWebEnginePage();
    inspector->setInspectedPage(page);

    QSignalSpy spy(page, &QWebEnginePage::loadFinished);
    page->setHtml(QStringLiteral("<body><h1>FOO BAR!</h1></body>"));
    QTRY_COMPARE_WITH_TIMEOUT(spy.size(), 1, 12000);

    page->triggerAction(QWebEnginePage::InspectElement);

    delete inspector;

    page->triggerAction(QWebEnginePage::InspectElement);

    // This is deliberately racy:
    QTest::qWait(10);

    delete page;
}


QTEST_MAIN(tst_DevTools)

#include "tst_devtools.moc"
