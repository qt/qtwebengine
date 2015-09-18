/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
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
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QWEBENGINEDOWNLOADITEM_P_H
#define QWEBENGINEDOWNLOADITEM_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qtwebenginewidgetsglobal.h"

#include "qwebenginedownloaditem_p.h"
#include "qwebengineprofile_p.h"
#include <QString>

QT_BEGIN_NAMESPACE

class QWebEngineDownloadItemPrivate {
    QWebEngineDownloadItem *q_ptr;
    QWebEngineProfilePrivate* profile;
    friend class QWebEngineProfilePrivate;
public:
    Q_DECLARE_PUBLIC(QWebEngineDownloadItem)
    QWebEngineDownloadItemPrivate(QWebEngineProfilePrivate *p, const QUrl &url);
    ~QWebEngineDownloadItemPrivate();

    bool downloadFinished;
    quint32 downloadId;
    QWebEngineDownloadItem::DownloadState downloadState;
    QString downloadPath;
    const QUrl downloadUrl;
    QString mimeType;

    qint64 totalBytes;
    qint64 receivedBytes;

    void update(const QtWebEngineCore::BrowserContextAdapterClient::DownloadItemInfo &info);
};

QT_END_NAMESPACE

#endif // QWEBENGINEDOWNLOADITEM_P_H

