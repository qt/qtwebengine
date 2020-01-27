/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/


#include <QtTest/QtTest>

#include <QPainter>
#include <QPdfDocument>
#include <QPrinter>
#include <QTemporaryFile>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

class tst_QPdfDocument: public QObject
{
    Q_OBJECT

public:
    tst_QPdfDocument()
    {
        qRegisterMetaType<QPdfDocument::Status>();
    }

private slots:
    void pageCount();
    void loadFromIODevice();
    void loadAsync();
    void password();
    void close();
    void loadAfterClose();
    void closeOnDestroy();
    void status();
    void passwordClearedOnClose();
    void metaData();
};

struct TemporaryPdf: public QTemporaryFile
{
    TemporaryPdf();
    QPageLayout pageLayout;
};


TemporaryPdf::TemporaryPdf()
{
    open();
    pageLayout = QPageLayout(QPageSize(QPageSize::A4), QPageLayout::Portrait, QMarginsF());

    {
        QPrinter printer;
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(fileName());
        printer.setPageLayout(pageLayout);

        {
            QPainter painter(&printer);
            painter.drawText(100, 100, QStringLiteral("Hello Page 1"));
            printer.newPage();
            painter.drawText(100, 100, QStringLiteral("Hello Page 2"));
        }
    }

    seek(0);
}

void tst_QPdfDocument::pageCount()
{
    TemporaryPdf tempPdf;

    QPdfDocument doc;
    QSignalSpy pageCountChangedSpy(&doc, SIGNAL(pageCountChanged(int)));

    QCOMPARE(doc.pageCount(), 0);
    QCOMPARE(doc.load(tempPdf.fileName()), QPdfDocument::NoError);
    QCOMPARE(doc.pageCount(), 2);
    QCOMPARE(pageCountChangedSpy.count(), 1);
    QCOMPARE(pageCountChangedSpy[0][0].toInt(), doc.pageCount());

    QCOMPARE(doc.pageSize(0).toSize(), tempPdf.pageLayout.fullRectPoints().size());
}

void tst_QPdfDocument::loadFromIODevice()
{
    TemporaryPdf tempPdf;
    QPdfDocument doc;
    QSignalSpy statusChangedSpy(&doc, SIGNAL(statusChanged(QPdfDocument::Status)));
    QSignalSpy pageCountChangedSpy(&doc, SIGNAL(pageCountChanged(int)));
    doc.load(&tempPdf);
    QCOMPARE(statusChangedSpy.count(), 2);
    QCOMPARE(statusChangedSpy[0][0].value<QPdfDocument::Status>(), QPdfDocument::Loading);
    QCOMPARE(statusChangedSpy[1][0].value<QPdfDocument::Status>(), QPdfDocument::Ready);
    QCOMPARE(doc.error(), QPdfDocument::NoError);
    QCOMPARE(doc.pageCount(), 2);
    QCOMPARE(pageCountChangedSpy.count(), 1);
    QCOMPARE(pageCountChangedSpy[0][0].toInt(), doc.pageCount());
}

void tst_QPdfDocument::loadAsync()
{
    TemporaryPdf tempPdf;

    QNetworkAccessManager nam;

    QUrl url = QUrl::fromLocalFile(tempPdf.fileName());
    QScopedPointer<QNetworkReply> reply(nam.get(QNetworkRequest(url)));

    QPdfDocument doc;
    QSignalSpy statusChangedSpy(&doc, SIGNAL(statusChanged(QPdfDocument::Status)));
    QSignalSpy pageCountChangedSpy(&doc, SIGNAL(pageCountChanged(int)));

    doc.load(reply.data());

    QCOMPARE(statusChangedSpy.count(), 2);
    QCOMPARE(statusChangedSpy[0][0].value<QPdfDocument::Status>(), QPdfDocument::Loading);
    QCOMPARE(statusChangedSpy[1][0].value<QPdfDocument::Status>(), QPdfDocument::Ready);
    QCOMPARE(doc.pageCount(), 2);
    QCOMPARE(pageCountChangedSpy.count(), 1);
    QCOMPARE(pageCountChangedSpy[0][0].toInt(), doc.pageCount());
}

void tst_QPdfDocument::password()
{
    QPdfDocument doc;
    QSignalSpy passwordChangedSpy(&doc, SIGNAL(passwordChanged()));

    QCOMPARE(doc.pageCount(), 0);
    QCOMPARE(doc.load(QFINDTESTDATA("pdf-sample.protected.pdf")), QPdfDocument::IncorrectPasswordError);
    QCOMPARE(passwordChangedSpy.count(), 0);
    doc.setPassword(QStringLiteral("WrongPassword"));
    QCOMPARE(passwordChangedSpy.count(), 1);
    QCOMPARE(doc.load(QFINDTESTDATA("pdf-sample.protected.pdf")), QPdfDocument::IncorrectPasswordError);
    QCOMPARE(doc.status(), QPdfDocument::Error);
    doc.setPassword(QStringLiteral("Qt"));
    QCOMPARE(passwordChangedSpy.count(), 2);
    QCOMPARE(doc.load(QFINDTESTDATA("pdf-sample.protected.pdf")), QPdfDocument::NoError);
    QCOMPARE(doc.pageCount(), 1);
}

void tst_QPdfDocument::close()
{
    TemporaryPdf tempPdf;
    QPdfDocument doc;

    QSignalSpy statusChangedSpy(&doc, SIGNAL(statusChanged(QPdfDocument::Status)));
    QSignalSpy pageCountChangedSpy(&doc, SIGNAL(pageCountChanged(int)));

    doc.load(&tempPdf);

    QCOMPARE(statusChangedSpy.count(), 2);
    QCOMPARE(statusChangedSpy[0][0].value<QPdfDocument::Status>(), QPdfDocument::Loading);
    QCOMPARE(statusChangedSpy[1][0].value<QPdfDocument::Status>(), QPdfDocument::Ready);
    QCOMPARE(pageCountChangedSpy.count(), 1);
    QCOMPARE(pageCountChangedSpy[0][0].toInt(), doc.pageCount());

    statusChangedSpy.clear();
    pageCountChangedSpy.clear();

    doc.close();
    QCOMPARE(statusChangedSpy.count(), 2);
    QCOMPARE(statusChangedSpy[0][0].value<QPdfDocument::Status>(), QPdfDocument::Unloading);
    QCOMPARE(statusChangedSpy[1][0].value<QPdfDocument::Status>(), QPdfDocument::Null);
    QCOMPARE(doc.pageCount(), 0);
    QCOMPARE(pageCountChangedSpy.count(), 1);
    QCOMPARE(pageCountChangedSpy[0][0].toInt(), doc.pageCount());
}

void tst_QPdfDocument::loadAfterClose()
{
    TemporaryPdf tempPdf;
    QPdfDocument doc;

    QSignalSpy statusChangedSpy(&doc, SIGNAL(statusChanged(QPdfDocument::Status)));
    QSignalSpy pageCountChangedSpy(&doc, SIGNAL(pageCountChanged(int)));

    doc.load(&tempPdf);
    QCOMPARE(statusChangedSpy.count(), 2);
    QCOMPARE(statusChangedSpy[0][0].value<QPdfDocument::Status>(), QPdfDocument::Loading);
    QCOMPARE(statusChangedSpy[1][0].value<QPdfDocument::Status>(), QPdfDocument::Ready);
    QCOMPARE(pageCountChangedSpy.count(), 1);
    QCOMPARE(pageCountChangedSpy[0][0].toInt(), doc.pageCount());
    statusChangedSpy.clear();
    pageCountChangedSpy.clear();

    doc.close();
    QCOMPARE(statusChangedSpy.count(), 2);
    QCOMPARE(statusChangedSpy[0][0].value<QPdfDocument::Status>(), QPdfDocument::Unloading);
    QCOMPARE(statusChangedSpy[1][0].value<QPdfDocument::Status>(), QPdfDocument::Null);
    QCOMPARE(pageCountChangedSpy.count(), 1);
    QCOMPARE(pageCountChangedSpy[0][0].toInt(), doc.pageCount());
    statusChangedSpy.clear();
    pageCountChangedSpy.clear();

    doc.load(&tempPdf);
    QCOMPARE(statusChangedSpy.count(), 2);
    QCOMPARE(statusChangedSpy[0][0].value<QPdfDocument::Status>(), QPdfDocument::Loading);
    QCOMPARE(statusChangedSpy[1][0].value<QPdfDocument::Status>(), QPdfDocument::Ready);
    QCOMPARE(doc.error(), QPdfDocument::NoError);
    QCOMPARE(doc.pageCount(), 2);
    QCOMPARE(pageCountChangedSpy.count(), 1);
    QCOMPARE(pageCountChangedSpy[0][0].toInt(), doc.pageCount());
}

void tst_QPdfDocument::closeOnDestroy()
{
    TemporaryPdf tempPdf;

    // deleting an open document should automatically close it
    {
        QPdfDocument *doc = new QPdfDocument;

        doc->load(&tempPdf);

        QSignalSpy statusChangedSpy(doc, SIGNAL(statusChanged(QPdfDocument::Status)));
        QSignalSpy pageCountChangedSpy(doc, SIGNAL(pageCountChanged(int)));

        delete doc;

        QCOMPARE(statusChangedSpy.count(), 2);
        QCOMPARE(statusChangedSpy[0][0].value<QPdfDocument::Status>(), QPdfDocument::Unloading);
        QCOMPARE(statusChangedSpy[1][0].value<QPdfDocument::Status>(), QPdfDocument::Null);
        QCOMPARE(pageCountChangedSpy.count(), 1);
        QCOMPARE(pageCountChangedSpy[0][0].toInt(), 0);
    }

    // deleting a closed document should not emit any signal
    {
        QPdfDocument *doc = new QPdfDocument;
        doc->load(&tempPdf);
        doc->close();

        QSignalSpy statusChangedSpy(doc, SIGNAL(statusChanged(QPdfDocument::Status)));
        QSignalSpy pageCountChangedSpy(doc, SIGNAL(pageCountChanged(int)));

        delete doc;

        QCOMPARE(statusChangedSpy.count(), 0);
        QCOMPARE(pageCountChangedSpy.count(), 0);
    }
}

void tst_QPdfDocument::status()
{
    TemporaryPdf tempPdf;

    QPdfDocument doc;
    QCOMPARE(doc.status(), QPdfDocument::Null);

    QSignalSpy statusChangedSpy(&doc, SIGNAL(statusChanged(QPdfDocument::Status)));

    // open existing document
    doc.load(&tempPdf);
    QCOMPARE(statusChangedSpy.count(), 2);
    QCOMPARE(statusChangedSpy[0][0].value<QPdfDocument::Status>(), QPdfDocument::Loading);
    QCOMPARE(statusChangedSpy[1][0].value<QPdfDocument::Status>(), QPdfDocument::Ready);
    statusChangedSpy.clear();

    QCOMPARE(doc.status(), QPdfDocument::Ready);

    // close document
    doc.close();

    QCOMPARE(statusChangedSpy.count(), 2);
    QCOMPARE(statusChangedSpy[0][0].value<QPdfDocument::Status>(), QPdfDocument::Unloading);
    QCOMPARE(statusChangedSpy[1][0].value<QPdfDocument::Status>(), QPdfDocument::Null);
    statusChangedSpy.clear();

    QCOMPARE(doc.status(), QPdfDocument::Null);

    // try to open non-existing document
    doc.load(QFINDTESTDATA("does-not-exist.pdf"));
    QCOMPARE(statusChangedSpy.count(), 2);
    QCOMPARE(statusChangedSpy[0][0].value<QPdfDocument::Status>(), QPdfDocument::Loading);
    QCOMPARE(statusChangedSpy[1][0].value<QPdfDocument::Status>(), QPdfDocument::Error);
    QCOMPARE(doc.status(), QPdfDocument::Error);
    statusChangedSpy.clear();

    // try to open non-existing document asynchronously
    QNetworkAccessManager accessManager;

    const QUrl url("http://doesnotexist.qt.io");
    QScopedPointer<QNetworkReply> reply(accessManager.get(QNetworkRequest(url)));

    doc.load(reply.data());

    QElapsedTimer stopWatch;
    stopWatch.start();
    forever {
        QCoreApplication::instance()->processEvents();
        if (statusChangedSpy.count() == 2)
            break;
        if (stopWatch.elapsed() >= 30000)
            break;
    }

    QCOMPARE(statusChangedSpy.count(), 2);
    QCOMPARE(statusChangedSpy[0][0].value<QPdfDocument::Status>(), QPdfDocument::Loading);
    QCOMPARE(statusChangedSpy[1][0].value<QPdfDocument::Status>(), QPdfDocument::Error);
    statusChangedSpy.clear();
}

void tst_QPdfDocument::passwordClearedOnClose()
{
    TemporaryPdf tempPdf;
    QPdfDocument doc;

    QSignalSpy passwordChangedSpy(&doc, SIGNAL(passwordChanged()));

    doc.setPassword(QStringLiteral("Qt"));
    QCOMPARE(passwordChangedSpy.count(), 1);
    QCOMPARE(doc.load(QFINDTESTDATA("pdf-sample.protected.pdf")), QPdfDocument::NoError);
    passwordChangedSpy.clear();

    doc.close(); // password is cleared on close
    QCOMPARE(passwordChangedSpy.count(), 1);
    passwordChangedSpy.clear();

    doc.load(&tempPdf);
    doc.close(); // signal is not emitted if password didn't change
    QCOMPARE(passwordChangedSpy.count(), 0);
}

void tst_QPdfDocument::metaData()
{
    QPdfDocument doc;

    // a closed document does not return any meta data
    QCOMPARE(doc.metaData(QPdfDocument::Title).toString(), QString());
    QCOMPARE(doc.metaData(QPdfDocument::Subject).toString(), QString());
    QCOMPARE(doc.metaData(QPdfDocument::Author).toString(), QString());
    QCOMPARE(doc.metaData(QPdfDocument::Keywords).toString(), QString());
    QCOMPARE(doc.metaData(QPdfDocument::Producer).toString(), QString());
    QCOMPARE(doc.metaData(QPdfDocument::Creator).toString(), QString());
    QCOMPARE(doc.metaData(QPdfDocument::CreationDate).toDateTime(), QDateTime());
    QCOMPARE(doc.metaData(QPdfDocument::ModificationDate).toDateTime(), QDateTime());

    QCOMPARE(doc.load(QFINDTESTDATA("pdf-sample.metadata.pdf")), QPdfDocument::NoError);

    // check for proper meta data from sample document
    QCOMPARE(doc.metaData(QPdfDocument::Title).toString(), QString::fromLatin1("Qt PDF Unit Test Document"));
    QCOMPARE(doc.metaData(QPdfDocument::Subject).toString(), QString::fromLatin1("A test for meta data access"));
    QCOMPARE(doc.metaData(QPdfDocument::Author).toString(), QString::fromLatin1("John Doe"));
    QCOMPARE(doc.metaData(QPdfDocument::Keywords).toString(), QString::fromLatin1("meta data keywords"));
    QCOMPARE(doc.metaData(QPdfDocument::Producer).toString(), QString::fromLatin1("LibreOffice 5.1"));
    QCOMPARE(doc.metaData(QPdfDocument::Creator).toString(), QString::fromLatin1("Writer"));
    QCOMPARE(doc.metaData(QPdfDocument::CreationDate).toDateTime(), QDateTime(QDate(2016, 8, 7), QTime(7, 3, 6), Qt::UTC));
    QCOMPARE(doc.metaData(QPdfDocument::ModificationDate).toDateTime(), QDateTime(QDate(2016, 8, 8), QTime(8, 3, 6), Qt::UTC));
}

QTEST_MAIN(tst_QPdfDocument)

#include "tst_qpdfdocument.moc"

