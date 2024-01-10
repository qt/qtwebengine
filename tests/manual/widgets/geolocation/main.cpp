// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

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
    void handleFeaturePermissionRequested(const QUrl &securityOrigin,
                                               QWebEnginePage::Feature feature)
    {
        qWarning("Feature Permission");
        QString title = tr("Permission Request");
        QString question = QLatin1String("Allow access to geolocation?");
        if (!question.isEmpty() && QMessageBox::question(window(), title, question) == QMessageBox::Yes)
            page()->setFeaturePermission(securityOrigin, feature,
                                         QWebEnginePage::PermissionGrantedByUser);
        else
            page()->setFeaturePermission(securityOrigin, feature,
                                         QWebEnginePage::PermissionDeniedByUser);
    }

};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QMainWindow w;
    GeoPermissionWebView webview;
    QWebEnginePage page;
    QObject::connect(&page, &QWebEnginePage::featurePermissionRequested, &webview,
            &GeoPermissionWebView::handleFeaturePermissionRequested);
    webview.setPage(&page);
    page.load(QUrl("qrc:/geolocation.html"));
    w.setCentralWidget(&webview);
    w.show();

    return a.exec();
}

#include "main.moc"
