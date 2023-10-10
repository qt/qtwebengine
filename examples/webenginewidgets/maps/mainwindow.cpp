// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "mainwindow.h"

#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_view(new QWebEngineView(this))
{
    setCentralWidget(m_view);

    QWebEnginePage *page = m_view->page();

    connect(page, &QWebEnginePage::featurePermissionRequested,
            [this, page](const QUrl &securityOrigin, QWebEnginePage::Feature feature) {
        if (feature != QWebEnginePage::Geolocation)
            return;

        QMessageBox msgBox(this);
        msgBox.setText(tr("%1 wants to know your location").arg(securityOrigin.host()));
        msgBox.setInformativeText(tr("Do you want to send your current location to this website?"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);

        if (msgBox.exec() == QMessageBox::Yes) {
            page->setFeaturePermission(
                securityOrigin, feature, QWebEnginePage::PermissionGrantedByUser);
        } else {
            page->setFeaturePermission(
                securityOrigin, feature, QWebEnginePage::PermissionDeniedByUser);
        }
    });

    page->load(QUrl(QStringLiteral("https://maps.google.com")));
}
