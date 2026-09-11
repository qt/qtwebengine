// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>
#include <QtWebEngineCore/qwebenginepage.h>

class tst_QtVersion : public QObject
{
    Q_OBJECT
signals:
    void done();
private Q_SLOTS:
    void checkVersion();
    void checkChromiumVersionFile();
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

void tst_QtVersion::checkChromiumVersionFile()
{
    QFile versionFile(QStringLiteral(":/CHROMIUM_VERSION"));
    QVERIFY2(versionFile.open(QIODevice::ReadOnly | QIODevice::Text),
             qPrintable(versionFile.errorString()));
    const QString content = QString::fromLatin1(versionFile.readAll());

    const QRegularExpression chromiumVersionRegExp(
            QStringLiteral("Based on Chromium version:[ \\t]*([0-9]+(?:\\.[0-9]+)+)"));
    const QRegularExpressionMatch chromiumVersionMatch = chromiumVersionRegExp.match(content);
    QVERIFY2(chromiumVersionMatch.hasMatch(),
             "Could not parse the Chromium version out of CHROMIUM_VERSION");
    QCOMPARE(chromiumVersionMatch.captured(1), QLatin1StringView(qWebEngineChromiumVersion()));

    const QRegularExpression securityPatchVersionRegExp(QStringLiteral(
            "Patched with security patches up to Chromium version:[ \\t]*([0-9]+(?:\\.[0-9]+)+)"));
    const QRegularExpressionMatch securityPatchVersionMatch =
            securityPatchVersionRegExp.match(content);
    QVERIFY2(securityPatchVersionMatch.hasMatch(),
             "Could not parse the security patch version out of CHROMIUM_VERSION");
    QCOMPARE(securityPatchVersionMatch.captured(1),
             QLatin1StringView(qWebEngineChromiumSecurityPatchVersion()));
}

QTEST_MAIN(tst_QtVersion)

#include "tst_qtversion.moc"
