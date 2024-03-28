// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QApplication>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QWebEnginePage>
#include <QWebEngineView>

class GeoPermissionWebView : public QWebEngineView {
  Q_OBJECT

public slots:
    void handlePermissionRequested(QWebEnginePermission permission)
    {
        qWarning("Feature Permission");
        QString title = tr("Permission Request");
        QString question = QLatin1String("Allow access to geolocation?");
        if (!question.isEmpty() && QMessageBox::question(window(), title, question) == QMessageBox::Yes)
            permission.grant();
        else
            permission.deny();
    }

};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QMainWindow w;
    GeoPermissionWebView webview;
    QWebEnginePage page;
    QObject::connect(&page, &QWebEnginePage::permissionRequested, &webview,
            &GeoPermissionWebView::handlePermissionRequested);
    webview.setPage(&page);
    page.load(QUrl("qrc:/geolocation.html"));
    w.setCentralWidget(&webview);
    w.show();

    return a.exec();
}

#include "main.moc"
