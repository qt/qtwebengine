// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <QTest>
#include <QSignalSpy>
#include <QWebEngineProfile>
#include <QWebEnginePage>
#include <QWebEngineView>

class tst_OffScreen : public QObject {
    Q_OBJECT
public:
    tst_OffScreen(){}

private slots:
    void offscreen();
};

void tst_OffScreen::offscreen()
{
    QWebEngineProfile profile;
    QWebEnginePage page(&profile);
    QWebEngineView view;
    QSignalSpy loadFinishedSpy(&page, SIGNAL(loadFinished(bool)));
    view.setPage(&page);
    page.load(QUrl("qrc:/test.html"));
    view.show();
    QTRY_COMPARE(view.isVisible(), true);
    QTRY_COMPARE_WITH_TIMEOUT(loadFinishedSpy.size() > 0, true, 20000);
    QCOMPARE(loadFinishedSpy.takeFirst().at(0).toBool(), true);
}

#include "tst_offscreen.moc"
QTEST_MAIN(tst_OffScreen)

