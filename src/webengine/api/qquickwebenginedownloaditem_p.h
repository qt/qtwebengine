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

#ifndef QQUICKWEBENGINEDOWNLOADITEM_P_H
#define QQUICKWEBENGINEDOWNLOADITEM_P_H

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

#include <private/qtwebengineglobal_p.h>
#include <QObject>
#include <QScopedPointer>
#include <QString>

QT_BEGIN_NAMESPACE

class QQuickWebEngineDownloadItemPrivate;
class QQuickWebEngineProfilePrivate;

class Q_WEBENGINE_PRIVATE_EXPORT QQuickWebEngineDownloadItem: public QObject {
    Q_OBJECT
public:
    ~QQuickWebEngineDownloadItem();
    enum DownloadState {
        DownloadRequested,
        DownloadInProgress,
        DownloadCompleted,
        DownloadCancelled,
        DownloadInterrupted
    };
    Q_ENUM(DownloadState)

    Q_PROPERTY(quint32 id READ id CONSTANT FINAL)
    Q_PROPERTY(DownloadState state READ state NOTIFY stateChanged)
    Q_PROPERTY(qint64 totalBytes READ totalBytes NOTIFY totalBytesChanged)
    Q_PROPERTY(qint64 receivedBytes READ receivedBytes NOTIFY receivedBytesChanged)
    Q_PROPERTY(QString mimeType READ mimeType NOTIFY mimeTypeChanged REVISION 1)
    Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged)

    Q_INVOKABLE void accept();
    Q_INVOKABLE void cancel();

    quint32 id() const;
    DownloadState state() const;
    qint64 totalBytes() const;
    qint64 receivedBytes() const;
    QString mimeType() const;
    QString path() const;
    void setPath(QString path);

Q_SIGNALS:
    void stateChanged();
    void receivedBytesChanged();
    void totalBytesChanged();
    void mimeTypeChanged();
    void pathChanged();

private:
    QQuickWebEngineDownloadItem(QQuickWebEngineDownloadItemPrivate*, QObject *parent = 0);
    Q_DISABLE_COPY(QQuickWebEngineDownloadItem)
    Q_DECLARE_PRIVATE(QQuickWebEngineDownloadItem)
    friend class QQuickWebEngineProfilePrivate;

    QScopedPointer<QQuickWebEngineDownloadItemPrivate> d_ptr;
};

QT_END_NAMESPACE

#endif // QQUICKWEBENGINEDOWNLOADITEM_P_H
