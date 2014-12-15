
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
    {
        QPrinter printer;
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(tempPdf.fileName());

        {
            QPainter painter(&printer);
            painter.drawText(0, 0, QStringLiteral("Hello Page 1"));
            printer.newPage();
            painter.drawText(0, 0, QStringLiteral("Hello Page 2"));
        }
    }

    QPdfDocument doc;
    QCOMPARE(doc.pageCount(), 0);
    QCOMPARE(doc.load(tempPdf.fileName()), QPdfDocument::NoError);
    QCOMPARE(doc.pageCount(), 2);
}

QTEST_MAIN(tst_QPdfDocument)

#include "tst_qpdfdocument.moc"

