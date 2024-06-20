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

    connect(page, &QWebEnginePage::permissionRequested,
            [this, page](QWebEnginePermission permission) {
        if (permission.permissionType() != QWebEnginePermission::PermissionType::Geolocation)
            return;

        QMessageBox msgBox(this);
        msgBox.setText(tr("%1 wants to know your location").arg(permission.origin().host()));
        msgBox.setInformativeText(tr("Do you want to send your current location to this website?"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);

        if (msgBox.exec() == QMessageBox::Yes)
            permission.grant();
        else
            permission.deny();
    });

    page->load(QUrl(QStringLiteral("https://bing.com/maps")));
}
