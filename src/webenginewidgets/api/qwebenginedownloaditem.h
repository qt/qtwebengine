/****************************************************************************
**
** Copyright (C) 2015 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QWEBENGINEDOWNLOADITEM_H
#define QWEBENGINEDOWNLOADITEM_H

#include "qtwebenginewidgetsglobal.h"

#include <QObject>

QT_BEGIN_NAMESPACE

class QWebEngineDownloadItemPrivate;
class QWebEngineProfilePrivate;

class QWEBENGINEWIDGETS_EXPORT QWebEngineDownloadItem : public QObject
{
    Q_OBJECT
public:
    ~QWebEngineDownloadItem();

    enum DownloadState {
        DownloadRequested,
        DownloadInProgress,
        DownloadCompleted,
        DownloadCancelled,
        DownloadInterrupted
    };
    Q_ENUMS(DownloadState)

    quint32 id();
    DownloadState state();
    qint64 totalBytes();
    qint64 receivedBytes();
    QUrl url();
    QString path();
    void setPath(QString path);
    bool isFinished();

public Q_SLOTS:
    void accept();
    void cancel();

Q_SIGNALS:
    void finished();
    void stateChanged(DownloadState state);
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);

private:
    Q_DISABLE_COPY(QWebEngineDownloadItem)
    Q_DECLARE_PRIVATE(QWebEngineDownloadItem)

    friend class QWebEngineProfilePrivate;

    QWebEngineDownloadItem(QWebEngineDownloadItemPrivate*, QObject *parent = 0);
    QScopedPointer<QWebEngineDownloadItemPrivate> d_ptr;
};

QT_END_NAMESPACE

#endif // QWEBENGINEDOWNLOADITEM_H
