// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtCore/qfile.h>
#include <QtNetwork/qsslkey.h>
#include <QtWebEngineCore/qwebenginecertificateerror.h>
#include <QtWebEngineCore/qwebengineclientcertificatestore.h>
#include <QtWebEngineCore/qwebengineprofile.h>
#include <QtWebEngineCore/qwebenginepage.h>
#include <QtWebEngineWidgets/qwebengineview.h>
#include <QtWidgets/qapplication.h>
#include <QtWidgets/qdialog.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qlistwidget.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qboxlayout.h>

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("QtExamples");
    QApplication app(argc, argv);

    QFile certFile(":/resources/client.pem");
    certFile.open(QIODevice::ReadOnly);
    const QSslCertificate cert(certFile.readAll(), QSsl::Pem);

    QFile keyFile(":/resources/client.key");
    keyFile.open(QIODevice::ReadOnly);
    const QSslKey sslKey(keyFile.readAll(), QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey, "");

    QWebEngineProfile::defaultProfile()->clientCertificateStore()->add(cert, sslKey);

    QWebEnginePage page;
    QObject::connect(&page, &QWebEnginePage::certificateError,
                     [](QWebEngineCertificateError e) { e.acceptCertificate(); });

    QObject::connect(
            &page, &QWebEnginePage::selectClientCertificate, &page,
            [&cert](QWebEngineClientCertificateSelection selection) {
                QDialog dialog;
                QVBoxLayout *layout = new QVBoxLayout;
                QLabel *label = new QLabel(QLatin1String("Select certificate"));
                QListWidget *listWidget = new QListWidget;
                listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
                QPushButton *button = new QPushButton(QLatin1String("Select"));
                layout->addWidget(label);
                layout->addWidget(listWidget);
                layout->addWidget(button);
                QObject::connect(button, &QPushButton::clicked, [&dialog]() { dialog.accept(); });
                const QList<QSslCertificate> &list = selection.certificates();
                for (const QSslCertificate &cert : list) {
                    listWidget->addItem(cert.subjectDisplayName() + " : " + cert.serialNumber());
                }
                dialog.setLayout(layout);
                if (dialog.exec() == QDialog::Accepted)
                    selection.select(list[listWidget->currentRow()]);
                else
                    selection.selectNone();
            });

    QWebEngineView view(&page);
    view.setUrl(QUrl("https://localhost:5555"));
    view.resize(800, 600);
    view.show();

    return app.exec();
}
