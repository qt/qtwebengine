// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef DOWNLOADMANAGERWIDGET_H
#define DOWNLOADMANAGERWIDGET_H

#include "ui_downloadmanagerwidget.h"

#include <QWidget>

QT_BEGIN_NAMESPACE
class QWebEngineDownloadRequest;
QT_END_NAMESPACE

class DownloadWidget;

// Displays a list of downloads.
class DownloadManagerWidget final : public QWidget, public Ui::DownloadManagerWidget
{
    Q_OBJECT
public:
    explicit DownloadManagerWidget(QWidget *parent = nullptr);

    // Prompts user with a "Save As" dialog. If the user doesn't cancel it, then
    // the QWebEngineDownloadRequest will be accepted and the DownloadManagerWidget
    // will be shown on the screen.
    void downloadRequested(QWebEngineDownloadRequest *webItem);

private:
    void add(DownloadWidget *downloadWidget);
    void remove(DownloadWidget *downloadWidget);

    int m_numDownloads = 0;
};

#endif // DOWNLOADMANAGERWIDGET_H
