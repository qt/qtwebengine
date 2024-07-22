// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QDir>
#include <QEventLoop>
#include <QPrinter>
#include <QWebEngineSettings>
#include <QWebEngineView>

class PrintTester : public QObject
{
    Q_OBJECT
public:
    explicit PrintTester(const QString &outputDir, const QUrl &url);
    int run();
    void waitForResult();

private slots:
    void loadFinished(bool ok);
    void printingFinished(bool success);
    void pdfPrintingFinished(const QString &filePath, bool success);

private:
    QWebEngineView m_view;
    QString m_outputDir;
    QUrl m_url;
    QEventLoop m_waitForResult;
    QPrinter m_printer;
};

PrintTester::PrintTester(const QString &outputDir, const QUrl &url)
    : m_outputDir(outputDir), m_url(url)
{
    connect(&m_view, &QWebEngineView::loadFinished, this, &PrintTester::loadFinished);
    connect(&m_view, &QWebEngineView::pdfPrintingFinished, this, &PrintTester::pdfPrintingFinished);
    connect(&m_view, &QWebEngineView::printFinished, this, &PrintTester::printingFinished);

    m_view.settings()->setAttribute(QWebEngineSettings::PrintHeaderAndFooter, false);
    m_view.settings()->setAttribute(QWebEngineSettings::PreferCSSMarginsForPrinting, false);
}

int PrintTester::run()
{
    m_view.load(m_url);
    return QApplication::exec();
}

void PrintTester::waitForResult()
{
    m_waitForResult.exec();
}

void PrintTester::loadFinished(bool ok)
{
    if (!ok) {
        qDebug() << "Page load was not successful.";
        QApplication::exit();
    }

    // Expected to be ignored; page sizes are coming from CSS
    const std::map<QString, QPageSize::PageSizeId> pageSizes = {
        { "a4", QPageSize::A4 },
        { "a5", QPageSize::A5 },
    };

    // Expected to be ignored; orientations are coming from CSS
    const std::map<QString, QPageLayout::Orientation> orientations = {
        { "portrait", QPageLayout::Portrait },
        { "landscape", QPageLayout::Landscape },
    };

    // Should be ignored when PreferCSSMarginsForPrinting is enabled
    const std::map<QString, QMargins> margins = {
        { "default", QMargins() },
        { "uniform", QMargins(20, 20, 20, 20) },
        { "topbottom", QMargins(5, 30, 5, 30) },
    };

    for (auto const &[pageSizeName, pageSize] : pageSizes) {
        for (auto const &[orientationName, orientation] : orientations) {
            for (auto const &[marginName, margin] : margins) {
                const QString fileNameSuffix =
                        pageSizeName + "_" + orientationName + "_" + marginName + "margins.pdf";
                QPageLayout layout(QPageSize(pageSize), orientation, margin);
                m_view.printToPdf(m_outputDir + "/printToPdf_" + fileNameSuffix, layout);
                waitForResult();

                m_printer.setOutputFileName(m_outputDir + "/print_" + fileNameSuffix);
                m_printer.setPageLayout(layout);
                m_view.print(&m_printer);
                waitForResult();
            }
        }
    }

    QApplication::quit();
}

void PrintTester::printingFinished(bool success)
{
    if (!success)
        QApplication::quit();
    qDebug() << m_printer.outputFileName() << "done";
    m_waitForResult.quit();
}

void PrintTester::pdfPrintingFinished(const QString &filePath, bool success)
{
    if (!success)
        QApplication::quit();
    qDebug() << filePath << "done";
    m_waitForResult.quit();
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("QtExamples");
    QCoreApplication::setApplicationName("printing");

    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::translate(
            "main", "Runs print() and printToPdf() API with various layouts."));
    parser.addHelpOption();
    parser.addPositionalArgument(
            QCoreApplication::translate("main", "OUTPUT"),
            QCoreApplication::translate("main",
                                        "Output directory name for generated PDF files. The "
                                        "directory must already exist."));
    parser.addPositionalArgument(
            QCoreApplication::translate("main", "INPUT"),
            QCoreApplication::translate(
                    "main", "Input URL for testing. If not set, a sample HTML file will be used."));

    parser.process(QCoreApplication::arguments());

    const QStringList requiredArguments = parser.positionalArguments();
    if (requiredArguments.size() < 1)
        parser.showHelp(1);

    if (!QDir(requiredArguments.at(0)).exists()) {
        qWarning("Output directory does not exist.");
        return 1;
    }

    PrintTester tester(requiredArguments.at(0),
                       requiredArguments.size() == 1
                               ? QUrl("qrc:/printing_example.html")
                               : QUrl::fromUserInput(requiredArguments.at(1)));
    return tester.run();
}

#include "main.moc"
