/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file. Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include "ui_downloads.h"
#include "ui_downloaditem.h"

#include <QtCore/QFileInfo>
#include <QtCore/QTime>
#include <QtCore/QUrl>

#include <QWebEngineDownloadItem>

class DownloadManager;
class DownloadWidget : public QWidget, public Ui_DownloadItem
{
    Q_OBJECT

signals:
    void statusChanged();

public:
    DownloadWidget(QWebEngineDownloadItem *download, QWidget *parent = 0);
    bool downloading() const;
    bool downloadedSuccessfully() const;

    void init();
    bool getFileName(bool promptForFileName = false);

private slots:
    void stop();
    void open();

    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void finished();

private:
    friend class DownloadManager;
    void updateInfoLabel();
    QString dataString(int size) const;

    QUrl m_url;
    QFileInfo m_file;
    qint64 m_bytesReceived;
    QTime m_downloadTime;
    bool m_stopped;

    QScopedPointer<QWebEngineDownloadItem> m_download;
};

class AutoSaver;
class DownloadModel;
QT_BEGIN_NAMESPACE
class QFileIconProvider;
QT_END_NAMESPACE

class DownloadManager : public QDialog, public Ui_DownloadDialog
{
    Q_OBJECT
    Q_PROPERTY(RemovePolicy removePolicy READ removePolicy WRITE setRemovePolicy)
    Q_ENUMS(RemovePolicy)

public:
    enum RemovePolicy {
        Never,
        Exit,
        SuccessFullDownload
    };

    DownloadManager(QWidget *parent = 0);
    ~DownloadManager();
    int activeDownloads() const;

    RemovePolicy removePolicy() const;
    void setRemovePolicy(RemovePolicy policy);

public slots:
    void download(QWebEngineDownloadItem *download);
    void cleanup();

private slots:
    void save() const;
    void updateRow();

private:
    void addItem(DownloadWidget *item);
    void updateItemCount();
    void load();

    AutoSaver *m_autoSaver;
    DownloadModel *m_model;
    QFileIconProvider *m_iconProvider;
    QList<DownloadWidget*> m_downloads;
    RemovePolicy m_removePolicy;
    friend class DownloadModel;
};

class DownloadModel : public QAbstractListModel
{
    friend class DownloadManager;
    Q_OBJECT

public:
    DownloadModel(DownloadManager *downloadManager, QObject *parent = 0);
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

private:
    DownloadManager *m_downloadManager;

};

#endif // DOWNLOADMANAGER_H
