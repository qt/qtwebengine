// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QApplication>
#include <QCommandLineParser>
#include <QFile>
#include <QTextStream>
#include <QWebEngineView>

#include <functional>
#include <utility>

class Html2PdfConverter : public QObject
{
    Q_OBJECT
public:
    explicit Html2PdfConverter(QString inputPath, QString outputPath);
    int run();

private slots:
    void loadFinished(bool ok);
    void pdfPrintingFinished(const QString &filePath, bool success);

private:
    QString m_inputPath;
    QString m_outputPath;
    QScopedPointer<QWebEngineView> m_view;
};

Html2PdfConverter::Html2PdfConverter(QString inputPath, QString outputPath)
    : m_inputPath(std::move(inputPath))
    , m_outputPath(std::move(outputPath))
    , m_view(new QWebEngineView)
{
    connect(m_view.data(), &QWebEngineView::loadFinished,
            this, &Html2PdfConverter::loadFinished);
    connect(m_view.data(), &QWebEngineView::pdfPrintingFinished,
            this, &Html2PdfConverter::pdfPrintingFinished);
}

int Html2PdfConverter::run()
{
    m_view->load(QUrl::fromUserInput(m_inputPath));
    return QApplication::exec();
}

void Html2PdfConverter::loadFinished(bool ok)
{
    if (!ok) {
        QTextStream(stderr)
            << tr("failed to load URL '%1'").arg(m_inputPath) << "\n";
        QCoreApplication::exit(1);
        return;
    }

    m_view->printToPdf(m_outputPath);
}

void Html2PdfConverter::pdfPrintingFinished(const QString &filePath, bool success)
{
    if (!success) {
        QTextStream(stderr)
            << tr("failed to print to output file '%1'").arg(filePath) << "\n";
        QCoreApplication::exit(1);
    } else {
        QCoreApplication::quit();
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("QtExamples");
    QCoreApplication::setApplicationName("html2pdf");
    QCoreApplication::setApplicationVersion(QT_VERSION_STR);

    QCommandLineParser parser;
    parser.setApplicationDescription(
        QCoreApplication::translate("main", "Converts the web page INPUT into the PDF file OUTPUT."));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(
        QCoreApplication::translate("main", "INPUT"),
        QCoreApplication::translate("main", "Input URL for PDF conversion."));
    parser.addPositionalArgument(
        QCoreApplication::translate("main", "OUTPUT"),
        QCoreApplication::translate("main", "Output file name for PDF conversion."));

    parser.process(QCoreApplication::arguments());

    const QStringList requiredArguments = parser.positionalArguments();
    if (requiredArguments.size() != 2)
        parser.showHelp(1);

    Html2PdfConverter converter(requiredArguments.at(0), requiredArguments.at(1));
    return converter.run();
}

#include "html2pdf.moc"
