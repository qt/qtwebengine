// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only


#include <QtTest/QtTest>

#include <QPainter>
#include <QPdfDocument>
#include <QPrinter>
#include <QDateTime>
#include <QTemporaryFile>
#include <QTimeZone>
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
    void pageLabels();
    void getSelection_data();
    void getSelection();
    void getSelectionAtIndex_data();
    void getSelectionAtIndex();

private:
    void consistencyCheck(QPdfDocument &doc) const;
};

struct TemporaryPdf: public QTemporaryFile
{
    TemporaryPdf();
    QPageLayout pageLayout;

    static QString pageText(int page) {
        switch (page) {
        case 0: return QStringLiteral("Hello Page 1");
        case 1: return QStringLiteral("Hello Page 2");
        default: return {};
        }
    }
};


TemporaryPdf::TemporaryPdf():QTemporaryFile(QStringLiteral("qpdfdocument"))
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
            painter.drawText(100, 100, pageText(0));
            printer.newPage();
            painter.drawText(100, 100, pageText(1));
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
    QCOMPARE(doc.load(tempPdf.fileName()), QPdfDocument::Error::None);
    QCOMPARE(doc.pageCount(), 2);
    QCOMPARE(pageCountChangedSpy.size(), 1);
    QCOMPARE(pageCountChangedSpy[0][0].toInt(), doc.pageCount());

    QCOMPARE(doc.pagePointSize(0).toSize(), tempPdf.pageLayout.fullRectPoints().size());
}

void tst_QPdfDocument::loadFromIODevice()
{
    TemporaryPdf tempPdf;
    QPdfDocument doc;
    QSignalSpy statusChangedSpy(&doc, SIGNAL(statusChanged(QPdfDocument::Status)));
    QSignalSpy pageCountChangedSpy(&doc, SIGNAL(pageCountChanged(int)));
    doc.load(&tempPdf);
    QCOMPARE(statusChangedSpy.size(), 2);
    QCOMPARE(statusChangedSpy[0][0].value<QPdfDocument::Status>(), QPdfDocument::Status::Loading);
    QCOMPARE(statusChangedSpy[1][0].value<QPdfDocument::Status>(), QPdfDocument::Status::Ready);
    QCOMPARE(doc.error(), QPdfDocument::Error::None);
    QCOMPARE(doc.pageCount(), 2);
    QCOMPARE(pageCountChangedSpy.size(), 1);
    QCOMPARE(pageCountChangedSpy[0][0].toInt(), doc.pageCount());

    consistencyCheck(doc);
}

void tst_QPdfDocument::consistencyCheck(QPdfDocument &doc) const
{
    for (int i = 0; i < doc.pageCount(); ++i) {
        const QString expected = TemporaryPdf::pageText(i);
        QPdfSelection page = doc.getAllText(i);
        QCOMPARE(page.text(), expected);
        auto pageMoved = std::move(page);
        QCOMPARE(pageMoved.text(), expected);
    }
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

    QCOMPARE(statusChangedSpy.size(), 2);
    QCOMPARE(statusChangedSpy[0][0].value<QPdfDocument::Status>(), QPdfDocument::Status::Loading);
    QCOMPARE(statusChangedSpy[1][0].value<QPdfDocument::Status>(), QPdfDocument::Status::Ready);
    QCOMPARE(doc.pageCount(), 2);
    QCOMPARE(pageCountChangedSpy.size(), 1);
    QCOMPARE(pageCountChangedSpy[0][0].toInt(), doc.pageCount());

    consistencyCheck(doc);
}

void tst_QPdfDocument::password()
{
    QPdfDocument doc;
    QSignalSpy passwordChangedSpy(&doc, SIGNAL(passwordChanged()));

    QCOMPARE(doc.pageCount(), 0);
    QCOMPARE(doc.load(QFINDTESTDATA("pdf-sample.protected.pdf")), QPdfDocument::Error::IncorrectPassword);
    QCOMPARE(passwordChangedSpy.size(), 0);
    doc.setPassword(QStringLiteral("WrongPassword"));
    QCOMPARE(passwordChangedSpy.size(), 1);
    QCOMPARE(doc.load(QFINDTESTDATA("pdf-sample.protected.pdf")), QPdfDocument::Error::IncorrectPassword);
    QCOMPARE(doc.status(), QPdfDocument::Status::Error);
    doc.setPassword(QStringLiteral("Qt"));
    QCOMPARE(passwordChangedSpy.size(), 2);
    QCOMPARE(doc.load(QFINDTESTDATA("pdf-sample.protected.pdf")), QPdfDocument::Error::None);
    QCOMPARE(doc.pageCount(), 1);
}

void tst_QPdfDocument::close()
{
    TemporaryPdf tempPdf;
    QPdfDocument doc;

    QSignalSpy statusChangedSpy(&doc, SIGNAL(statusChanged(QPdfDocument::Status)));
    QSignalSpy pageCountChangedSpy(&doc, SIGNAL(pageCountChanged(int)));

    doc.load(&tempPdf);

    QCOMPARE(statusChangedSpy.size(), 2);
    QCOMPARE(statusChangedSpy[0][0].value<QPdfDocument::Status>(), QPdfDocument::Status::Loading);
    QCOMPARE(statusChangedSpy[1][0].value<QPdfDocument::Status>(), QPdfDocument::Status::Ready);
    QCOMPARE(pageCountChangedSpy.size(), 1);
    QCOMPARE(pageCountChangedSpy[0][0].toInt(), doc.pageCount());

    statusChangedSpy.clear();
    pageCountChangedSpy.clear();

    consistencyCheck(doc);
    if (QTest::currentTestFailed())
        return;

    doc.close();
    QCOMPARE(statusChangedSpy.size(), 2);
    QCOMPARE(statusChangedSpy[0][0].value<QPdfDocument::Status>(), QPdfDocument::Status::Unloading);
    QCOMPARE(statusChangedSpy[1][0].value<QPdfDocument::Status>(), QPdfDocument::Status::Null);
    QCOMPARE(doc.pageCount(), 0);
    QCOMPARE(pageCountChangedSpy.size(), 1);
    QCOMPARE(pageCountChangedSpy[0][0].toInt(), doc.pageCount());
}

void tst_QPdfDocument::loadAfterClose()
{
    TemporaryPdf tempPdf;
    QPdfDocument doc;

    QSignalSpy statusChangedSpy(&doc, SIGNAL(statusChanged(QPdfDocument::Status)));
    QSignalSpy pageCountChangedSpy(&doc, SIGNAL(pageCountChanged(int)));

    doc.load(&tempPdf);
    QCOMPARE(statusChangedSpy.size(), 2);
    QCOMPARE(statusChangedSpy[0][0].value<QPdfDocument::Status>(), QPdfDocument::Status::Loading);
    QCOMPARE(statusChangedSpy[1][0].value<QPdfDocument::Status>(), QPdfDocument::Status::Ready);
    QCOMPARE(pageCountChangedSpy.size(), 1);
    QCOMPARE(pageCountChangedSpy[0][0].toInt(), doc.pageCount());
    statusChangedSpy.clear();
    pageCountChangedSpy.clear();

    doc.close();
    QCOMPARE(statusChangedSpy.size(), 2);
    QCOMPARE(statusChangedSpy[0][0].value<QPdfDocument::Status>(), QPdfDocument::Status::Unloading);
    QCOMPARE(statusChangedSpy[1][0].value<QPdfDocument::Status>(), QPdfDocument::Status::Null);
    QCOMPARE(pageCountChangedSpy.size(), 1);
    QCOMPARE(pageCountChangedSpy[0][0].toInt(), doc.pageCount());
    statusChangedSpy.clear();
    pageCountChangedSpy.clear();

    doc.load(&tempPdf);
    QCOMPARE(statusChangedSpy.size(), 2);
    QCOMPARE(statusChangedSpy[0][0].value<QPdfDocument::Status>(), QPdfDocument::Status::Loading);
    QCOMPARE(statusChangedSpy[1][0].value<QPdfDocument::Status>(), QPdfDocument::Status::Ready);
    QCOMPARE(doc.error(), QPdfDocument::Error::None);
    QCOMPARE(doc.pageCount(), 2);
    QCOMPARE(pageCountChangedSpy.size(), 1);
    QCOMPARE(pageCountChangedSpy[0][0].toInt(), doc.pageCount());

    consistencyCheck(doc);
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

        QCOMPARE(statusChangedSpy.size(), 2);
        QCOMPARE(statusChangedSpy[0][0].value<QPdfDocument::Status>(), QPdfDocument::Status::Unloading);
        QCOMPARE(statusChangedSpy[1][0].value<QPdfDocument::Status>(), QPdfDocument::Status::Null);
        QCOMPARE(pageCountChangedSpy.size(), 1);
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

        QCOMPARE(statusChangedSpy.size(), 0);
        QCOMPARE(pageCountChangedSpy.size(), 0);
    }
}

void tst_QPdfDocument::status()
{
    TemporaryPdf tempPdf;

    QPdfDocument doc;
    QCOMPARE(doc.status(), QPdfDocument::Status::Null);

    QSignalSpy statusChangedSpy(&doc, SIGNAL(statusChanged(QPdfDocument::Status)));

    // open existing document
    doc.load(&tempPdf);
    QCOMPARE(statusChangedSpy.size(), 2);
    QCOMPARE(statusChangedSpy[0][0].value<QPdfDocument::Status>(), QPdfDocument::Status::Loading);
    QCOMPARE(statusChangedSpy[1][0].value<QPdfDocument::Status>(), QPdfDocument::Status::Ready);
    statusChangedSpy.clear();

    QCOMPARE(doc.status(), QPdfDocument::Status::Ready);

    // close document
    doc.close();

    QCOMPARE(statusChangedSpy.size(), 2);
    QCOMPARE(statusChangedSpy[0][0].value<QPdfDocument::Status>(), QPdfDocument::Status::Unloading);
    QCOMPARE(statusChangedSpy[1][0].value<QPdfDocument::Status>(), QPdfDocument::Status::Null);
    statusChangedSpy.clear();

    QCOMPARE(doc.status(), QPdfDocument::Status::Null);

    // try to open non-existing document
    doc.load(QFINDTESTDATA("does-not-exist.pdf"));
    QCOMPARE(statusChangedSpy.size(), 2);
    QCOMPARE(statusChangedSpy[0][0].value<QPdfDocument::Status>(), QPdfDocument::Status::Loading);
    QCOMPARE(statusChangedSpy[1][0].value<QPdfDocument::Status>(), QPdfDocument::Status::Error);
    QCOMPARE(doc.status(), QPdfDocument::Status::Error);
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
        if (statusChangedSpy.size() == 2)
            break;
        if (stopWatch.elapsed() >= 30000)
            break;
    }

    QCOMPARE(statusChangedSpy.size(), 2);
    QCOMPARE(statusChangedSpy[0][0].value<QPdfDocument::Status>(), QPdfDocument::Status::Loading);
    QCOMPARE(statusChangedSpy[1][0].value<QPdfDocument::Status>(), QPdfDocument::Status::Error);
    statusChangedSpy.clear();
}

void tst_QPdfDocument::passwordClearedOnClose()
{
    TemporaryPdf tempPdf;
    QPdfDocument doc;

    QSignalSpy passwordChangedSpy(&doc, SIGNAL(passwordChanged()));

    doc.setPassword(QStringLiteral("Qt"));
    QCOMPARE(passwordChangedSpy.size(), 1);
    QCOMPARE(doc.load(QFINDTESTDATA("pdf-sample.protected.pdf")), QPdfDocument::Error::None);
    passwordChangedSpy.clear();

    doc.close(); // password is cleared on close
    QCOMPARE(passwordChangedSpy.size(), 1);
    passwordChangedSpy.clear();

    doc.load(&tempPdf);
    doc.close(); // signal is not emitted if password didn't change
    QCOMPARE(passwordChangedSpy.size(), 0);
}

void tst_QPdfDocument::metaData()
{
    QPdfDocument doc;

    // a closed document does not return any meta data
    QCOMPARE(doc.metaData(QPdfDocument::MetaDataField::Title).toString(), QString());
    QCOMPARE(doc.metaData(QPdfDocument::MetaDataField::Subject).toString(), QString());
    QCOMPARE(doc.metaData(QPdfDocument::MetaDataField::Author).toString(), QString());
    QCOMPARE(doc.metaData(QPdfDocument::MetaDataField::Keywords).toString(), QString());
    QCOMPARE(doc.metaData(QPdfDocument::MetaDataField::Producer).toString(), QString());
    QCOMPARE(doc.metaData(QPdfDocument::MetaDataField::Creator).toString(), QString());
    QCOMPARE(doc.metaData(QPdfDocument::MetaDataField::CreationDate).toDateTime(), QDateTime());
    QCOMPARE(doc.metaData(QPdfDocument::MetaDataField::ModificationDate).toDateTime(), QDateTime());

    QCOMPARE(doc.load(QFINDTESTDATA("pdf-sample.metadata.pdf")), QPdfDocument::Error::None);

    // check for proper meta data from sample document
    QCOMPARE(doc.metaData(QPdfDocument::MetaDataField::Title).toString(), QString::fromLatin1("Qt PDF Unit Test Document"));
    QCOMPARE(doc.metaData(QPdfDocument::MetaDataField::Subject).toString(), QString::fromLatin1("A test for meta data access"));
    QCOMPARE(doc.metaData(QPdfDocument::MetaDataField::Author).toString(), QString::fromLatin1("John Doe"));
    QCOMPARE(doc.metaData(QPdfDocument::MetaDataField::Keywords).toString(), QString::fromLatin1("meta data keywords"));
    QCOMPARE(doc.metaData(QPdfDocument::MetaDataField::Producer).toString(), QString::fromLatin1("LibreOffice 5.1"));
    QCOMPARE(doc.metaData(QPdfDocument::MetaDataField::Creator).toString(), QString::fromLatin1("Writer"));
    QCOMPARE(doc.metaData(QPdfDocument::MetaDataField::CreationDate).toDateTime(),
             QDateTime(QDate(2016, 8, 7), QTime(7, 3, 6), QTimeZone::UTC));
    QCOMPARE(doc.metaData(QPdfDocument::MetaDataField::ModificationDate).toDateTime(),
             QDateTime(QDate(2016, 8, 8), QTime(8, 3, 6), QTimeZone::UTC));
}

void tst_QPdfDocument::pageLabels()
{
    QPdfDocument doc;
    QCOMPARE(doc.load(QFINDTESTDATA("test.pdf")), QPdfDocument::Error::None);
    QCOMPARE(doc.pageCount(), 3);
    QCOMPARE(doc.pageLabel(0), "Qt");
    QCOMPARE(doc.pageLabel(1), "1");
    QCOMPARE(doc.pageLabel(2), "i"); // i of the tiger!
}

void tst_QPdfDocument::getSelection_data()
{
    QTest::addColumn<QString>("pdfPath");
    QTest::addColumn<int>("page");
    QTest::addColumn<QPointF>("start");
    QTest::addColumn<QPointF>("end");
    QTest::addColumn<QString>("expectedText");
    QTest::addColumn<int>("expectedStartIndex");
    QTest::addColumn<int>("expectedEndIndex");
    QTest::addColumn<QRect>("expectedBounds");
    QTest::addColumn<int>("expectedPolygonCount");

    QTest::newRow("raid") << QFINDTESTDATA("test.pdf")
            << 1 << QPointF(316.4, 206) << QPointF(339, 201)
            << "raid" << 80 << 84 << QRect(316, 201, 21, 12) << 1;
    QTest::newRow("rotated text") << QFINDTESTDATA("rotated_text.pdf")
            << 0 << QPointF(102, 94) << QPointF(125, 73)
            << "world!" << 25 << 31 << QRect(98, 70, 26, 28) << 1;
}

void tst_QPdfDocument::getSelection()
{
    QFETCH(QString, pdfPath);
    QFETCH(int, page);
    QFETCH(QPointF, start);
    QFETCH(QPointF, end);
    QFETCH(QString, expectedText);
    QFETCH(int, expectedStartIndex);
    QFETCH(int, expectedEndIndex);
    QFETCH(QRect, expectedBounds);
    QFETCH(int, expectedPolygonCount);

    QPdfDocument doc;
    QCOMPARE(doc.load(pdfPath), QPdfDocument::Error::None);

    QPdfSelection sel = doc.getSelection(page, start, end);
    QCOMPARE(sel.text(), expectedText);
    QCOMPARE(sel.startIndex(), expectedStartIndex);
    QCOMPARE(sel.endIndex(), expectedEndIndex);
    QCOMPARE(sel.boundingRectangle().toRect(), expectedBounds);
    QCOMPARE(sel.bounds().size(), expectedPolygonCount);
}

void tst_QPdfDocument::getSelectionAtIndex_data()
{
    QTest::addColumn<QString>("pdfPath");
    QTest::addColumn<int>("page");
    QTest::addColumn<int>("start");
    QTest::addColumn<int>("maxLen");
    QTest::addColumn<QString>("expectedText");
    QTest::addColumn<QRect>("expectedBounds");
    QTest::addColumn<int>("expectedPolygonCount");

    QTest::newRow("raid") << QFINDTESTDATA("test.pdf")
            << 1 << 80 << 4 << "raid" << QRect(316, 201, 21, 12) << 1;
    QTest::newRow("rotated text") << QFINDTESTDATA("rotated_text.pdf")
            << 0 << 7 << 6 << "world!" << QRect(76, 102, 26, 28) << 1;
    QTest::newRow("displaced text") << QFINDTESTDATA("tagged_mcr_multipage.pdf")
            << 0 << 0 << 10 << "1" << QRect(34, 22, 3, 8) << 1;
}

void tst_QPdfDocument::getSelectionAtIndex()
{
    QFETCH(QString, pdfPath);
    QFETCH(int, page);
    QFETCH(int, start);
    QFETCH(int, maxLen);
    QFETCH(QString, expectedText);
    QFETCH(QRect, expectedBounds);
    QFETCH(int, expectedPolygonCount);

    QPdfDocument doc;
    QCOMPARE(doc.load(pdfPath), QPdfDocument::Error::None);

    QPdfSelection sel = doc.getSelectionAtIndex(page, start, maxLen);
    QCOMPARE(sel.text(), expectedText);
    QCOMPARE(sel.boundingRectangle().toRect(), expectedBounds);
    QCOMPARE(sel.bounds().size(), expectedPolygonCount);
}

QTEST_MAIN(tst_QPdfDocument)

#include "tst_qpdfdocument.moc"

