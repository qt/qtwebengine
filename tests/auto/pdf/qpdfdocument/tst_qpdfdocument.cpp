
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

private slots:
    void pageCount();
    void loadFromIODevice();
    void loadAsync();
    void password();
    void close();
    void loadAfterClose();
    void closeOnDestroy();
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
    QCOMPARE(doc.pageCount(), 0);
    QCOMPARE(doc.load(tempPdf.fileName()), QPdfDocument::NoError);
    QCOMPARE(doc.pageCount(), 2);

    QCOMPARE(doc.pageSize(0).toSize(), tempPdf.pageLayout.fullRectPoints().size());
}

void tst_QPdfDocument::loadFromIODevice()
{
    TemporaryPdf tempPdf;
    QPdfDocument doc;
    QSignalSpy startedSpy(&doc, SIGNAL(documentLoadStarted()));
    QSignalSpy finishedSpy(&doc, SIGNAL(documentLoadFinished()));
    doc.load(&tempPdf);
    QCOMPARE(startedSpy.count(), 1);
    QCOMPARE(finishedSpy.count(), 1);
    QCOMPARE(doc.error(), QPdfDocument::NoError);
    QCOMPARE(doc.pageCount(), 2);
}

void tst_QPdfDocument::loadAsync()
{
    TemporaryPdf tempPdf;

    QNetworkAccessManager nam;

    QUrl url = QUrl::fromLocalFile(tempPdf.fileName());
    QScopedPointer<QNetworkReply> reply(nam.get(QNetworkRequest(url)));

    QPdfDocument doc;
    QSignalSpy startedSpy(&doc, SIGNAL(documentLoadStarted()));
    QSignalSpy finishedSpy(&doc, SIGNAL(documentLoadFinished()));

    doc.load(reply.data());

    QCOMPARE(startedSpy.count(), 1);
    QCOMPARE(finishedSpy.count(), 1);
    QCOMPARE(doc.pageCount(), 2);
}

void tst_QPdfDocument::password()
{
    QPdfDocument doc;
    QCOMPARE(doc.pageCount(), 0);
    QCOMPARE(doc.load(QFINDTESTDATA("pdf-sample.protected.pdf")), QPdfDocument::IncorrectPasswordError);
    doc.setPassword(QStringLiteral("WrongPassword"));
    QCOMPARE(doc.load(QFINDTESTDATA("pdf-sample.protected.pdf")), QPdfDocument::IncorrectPasswordError);
    doc.setPassword(QStringLiteral("Qt"));
    QCOMPARE(doc.load(QFINDTESTDATA("pdf-sample.protected.pdf")), QPdfDocument::NoError);
    QCOMPARE(doc.pageCount(), 1);
}

void tst_QPdfDocument::close()
{
    TemporaryPdf tempPdf;
    QPdfDocument doc;

    QSignalSpy aboutToBeClosedSpy(&doc, SIGNAL(aboutToBeClosed()));

    doc.load(&tempPdf);
    QCOMPARE(aboutToBeClosedSpy.count(), 0);
    doc.close();
    QCOMPARE(aboutToBeClosedSpy.count(), 1);
    QCOMPARE(doc.pageCount(), 0);
}

void tst_QPdfDocument::loadAfterClose()
{
    TemporaryPdf tempPdf;
    QPdfDocument doc;

    QSignalSpy aboutToBeClosedSpy(&doc, SIGNAL(aboutToBeClosed()));

    doc.load(&tempPdf);
    QCOMPARE(aboutToBeClosedSpy.count(), 0);
    doc.close();
    QCOMPARE(aboutToBeClosedSpy.count(), 1);

    QSignalSpy startedSpy(&doc, SIGNAL(documentLoadStarted()));
    QSignalSpy finishedSpy(&doc, SIGNAL(documentLoadFinished()));
    doc.load(&tempPdf);
    QCOMPARE(startedSpy.count(), 1);
    QCOMPARE(finishedSpy.count(), 1);
    QCOMPARE(doc.error(), QPdfDocument::NoError);
    QCOMPARE(doc.pageCount(), 2);
}

void tst_QPdfDocument::closeOnDestroy()
{
    TemporaryPdf tempPdf;

    // deleting an open document should automatically close it
    {
        QPdfDocument *doc = new QPdfDocument;

        QSignalSpy aboutToBeClosedSpy(doc, SIGNAL(aboutToBeClosed()));
        doc->load(&tempPdf);

        delete doc;

        QCOMPARE(aboutToBeClosedSpy.count(), 1);
    }

    // deleting a closed document should not emit any signal
    {
        QPdfDocument *doc = new QPdfDocument;
        doc->load(&tempPdf);
        doc->close();

        QSignalSpy aboutToBeClosedSpy(doc, SIGNAL(aboutToBeClosed()));

        delete doc;

        QCOMPARE(aboutToBeClosedSpy.count(), 0);
    }
}

QTEST_MAIN(tst_QPdfDocument)

#include "tst_qpdfdocument.moc"

