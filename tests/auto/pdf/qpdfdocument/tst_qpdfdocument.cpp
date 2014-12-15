
#include <QtTest/QtTest>

#include <QPainter>
#include <QPdfDocument>
#include <QPrinter>
#include <QTemporaryFile>

class tst_QPdfDocument: public QObject
{
    Q_OBJECT
public:

private slots:
    void pageCount();
};


void tst_QPdfDocument::pageCount()
{
    QTemporaryFile tempPdf;
    tempPdf.setAutoRemove(true);
    QVERIFY(tempPdf.open());

    QPageLayout layout(QPageSize(QPageSize::A4), QPageLayout::Portrait, QMarginsF());

    {
        QPrinter printer;
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(tempPdf.fileName());
        printer.setPageLayout(layout);

        {
            QPainter painter(&printer);
            painter.drawText(100, 100, QStringLiteral("Hello Page 1"));
            printer.newPage();
            painter.drawText(100, 100, QStringLiteral("Hello Page 2"));
        }
    }

    QPdfDocument doc;
    QCOMPARE(doc.pageCount(), 0);
    QCOMPARE(doc.load(tempPdf.fileName()), QPdfDocument::NoError);
    QCOMPARE(doc.pageCount(), 2);

    QCOMPARE(doc.pageSize(0).toSize(), layout.fullRectPoints().size());
}

QTEST_MAIN(tst_QPdfDocument)

#include "tst_qpdfdocument.moc"

