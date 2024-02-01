// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <QtTest/QtTest>
#include <QtWebEngineCore/qwebenginepage.h>

class tst_QtVersion : public QObject
{
    Q_OBJECT
signals:
    void done();
private Q_SLOTS:
    void checkVersion();
};

void tst_QtVersion::checkVersion()
{
    QWebEnginePage page;
    QSignalSpy loadSpy(&page, &QWebEnginePage::loadFinished);
    QSignalSpy doneSpy(this, &tst_QtVersion::done);
    page.load(QUrl("chrome://qt"));
    QTRY_COMPARE_WITH_TIMEOUT(loadSpy.size(), 1, 12000);
    page.toPlainText([this](const QString &result) {
        QVERIFY(result.contains(qWebEngineVersion()));
        QVERIFY(result.contains(qWebEngineChromiumVersion()));
        QVERIFY(result.contains(qWebEngineChromiumSecurityPatchVersion()));
        emit done();
    });
    QTRY_VERIFY(doneSpy.size());
}

QTEST_MAIN(tst_QtVersion)

#include "tst_qtversion.moc"
