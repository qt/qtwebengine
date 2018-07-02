/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

#include <QWebEnginePage>
#include <QTemporaryDir>
#include <QTest>
#include <QSignalSpy>
#include <util.h>

class tst_Printing : public QObject
{
    Q_OBJECT
private slots:
    void printToPdf();
};


void tst_Printing::printToPdf()
{
    QTemporaryDir tempDir(QDir::tempPath() + "/tst_qwebengineview-XXXXXX");
    QVERIFY(tempDir.isValid());
    QWebEnginePage page;
    QSignalSpy spy(&page, SIGNAL(loadFinished(bool)));
    page.load(QUrl("qrc:///resources/basic_printing_page.html"));
    QTRY_VERIFY(spy.count() == 1);

    QSignalSpy savePdfSpy(&page, SIGNAL(pdfPrintingFinished(const QString&, bool)));
    QPageLayout layout(QPageSize(QPageSize::A4), QPageLayout::Portrait, QMarginsF(0.0, 0.0, 0.0, 0.0));
    QString path = tempDir.path() + "/print_1_success.pdf";
    page.printToPdf(path, layout);
    QTRY_VERIFY2(savePdfSpy.count() == 1, "Printing to PDF file failed without signal");

    QList<QVariant> successArguments = savePdfSpy.takeFirst();
    QVERIFY2(successArguments.at(0).toString() == path, "File path for first saved PDF does not match arguments");
    QVERIFY2(successArguments.at(1).toBool() == true, "Printing to PDF file failed though it should succeed");

#if !defined(Q_OS_WIN)
    path = tempDir.path() + "/print_//2_failed.pdf";
#else
    path = tempDir.path() + "/print_|2_failed.pdf";
#endif
    page.printToPdf(path, QPageLayout());
    QTRY_VERIFY2(savePdfSpy.count() == 1, "Printing to PDF file failed without signal");

    QList<QVariant> failedArguments = savePdfSpy.takeFirst();
    QVERIFY2(failedArguments.at(0).toString() == path, "File path for second saved PDF does not match arguments");
    QVERIFY2(failedArguments.at(1).toBool() == false, "Printing to PDF file succeeded though it should fail");

    CallbackSpy<QByteArray> successfulSpy;
    page.printToPdf(successfulSpy.ref(), layout);
    QVERIFY(successfulSpy.waitForResult().length() > 0);

    CallbackSpy<QByteArray> failedInvalidLayoutSpy;
    page.printToPdf(failedInvalidLayoutSpy.ref(), QPageLayout());
    QCOMPARE(failedInvalidLayoutSpy.waitForResult().length(), 0);
}

QTEST_MAIN(tst_Printing)
#include "tst_printing.moc"
