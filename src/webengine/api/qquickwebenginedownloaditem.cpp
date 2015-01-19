/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
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

#include "qquickwebenginedownloaditem_p.h"
#include "qquickwebenginedownloaditem_p_p.h"

QT_BEGIN_NAMESPACE

QQuickWebEngineDownloadItemPrivate::QQuickWebEngineDownloadItemPrivate(QQuickWebEngineProfilePrivate *p)
    : profile(p)
    , downloadStarted(false)
    , downloadId(-1)
    , downloadState(QQuickWebEngineDownloadItem::DownloadCancelled)
    , downloadProgress(0)
{
}

QQuickWebEngineDownloadItemPrivate::~QQuickWebEngineDownloadItemPrivate()
{
    profile->downloadDestroyed(downloadId);
}

void QQuickWebEngineDownloadItemPrivate::update(QQuickWebEngineDownloadItem::DownloadState state, int progress)
{
    Q_Q(QQuickWebEngineDownloadItem);
    if (state != downloadState) {
        downloadState = state;
        Q_EMIT q->stateChanged();
    }
    if (progress != downloadProgress) {
        downloadProgress = progress;
        Q_EMIT q->progressChanged();
    }
}

void QQuickWebEngineDownloadItem::cancel()
{
    Q_D(QQuickWebEngineDownloadItem);

    if (d->downloadState == QQuickWebEngineDownloadItem::DownloadCompleted
            || d->downloadState == QQuickWebEngineDownloadItem::DownloadCancelled)
        return;

    d->update(QQuickWebEngineDownloadItem::DownloadCancelled, d->downloadProgress);

    // We directly cancel the download if the user cancels before
    // it even started, so no need to notify the profile here.
    if (d->downloadStarted)
        d->profile->cancelDownload(d->downloadId);
}

quint32 QQuickWebEngineDownloadItem::id()
{
    Q_D(QQuickWebEngineDownloadItem);
    return d->downloadId;
}

QQuickWebEngineDownloadItem::DownloadState QQuickWebEngineDownloadItem::state()
{
    Q_D(QQuickWebEngineDownloadItem);
    return d->downloadState;
}

int QQuickWebEngineDownloadItem::progress()
{
    Q_D(QQuickWebEngineDownloadItem);
    return d->downloadProgress;
}

QString QQuickWebEngineDownloadItem::path()
{
    Q_D(QQuickWebEngineDownloadItem);
    return d->downloadPath;
}

void QQuickWebEngineDownloadItem::setPath(QString path)
{
    Q_D(QQuickWebEngineDownloadItem);
    if (d->downloadStarted) {
        qWarning("Setting the download path is not allowed after the download has been started.");
        return;
    }
    if (d->downloadPath != path) {
        d->downloadPath = path;
        Q_EMIT pathChanged();
    }
}

QQuickWebEngineDownloadItem::QQuickWebEngineDownloadItem(QQuickWebEngineDownloadItemPrivate *p, QObject *parent)
    : QObject(parent)
    , d_ptr(p)
{
    p->q_ptr = this;
}

QQuickWebEngineDownloadItem::~QQuickWebEngineDownloadItem()
{
}

QT_END_NAMESPACE
