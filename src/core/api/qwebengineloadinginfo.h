/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QWEBENGINELOADINGINFO_H
#define QWEBENGINELOADINGINFO_H

#include <QtWebEngineCore/qtwebenginecoreglobal.h>

#include <QExplicitlySharedDataPointer>
#include <QtCore/qobject.h>
#include <QtCore/qurl.h>

namespace QtWebEngineCore {
class WebContentsAdapter;
class WebContentsDelegateQt;
}

QT_BEGIN_NAMESPACE

class Q_WEBENGINECORE_EXPORT QWebEngineLoadingInfo
{
    Q_GADGET
    Q_PROPERTY(QUrl url READ url CONSTANT FINAL)
    Q_PROPERTY(bool isErrorPage READ isErrorPage CONSTANT FINAL)
    Q_PROPERTY(LoadStatus status READ status CONSTANT FINAL)
    Q_PROPERTY(QString errorString READ errorString CONSTANT FINAL)
    Q_PROPERTY(ErrorDomain errorDomain READ errorDomain CONSTANT FINAL)
    Q_PROPERTY(int errorCode READ errorCode CONSTANT FINAL)

public:
    enum LoadStatus {
        LoadStartedStatus,
        LoadStoppedStatus,
        LoadSucceededStatus,
        LoadFailedStatus
    };
    Q_ENUM(LoadStatus)

    enum ErrorDomain {
         NoErrorDomain,
         InternalErrorDomain,
         ConnectionErrorDomain,
         CertificateErrorDomain,
         HttpErrorDomain,
         FtpErrorDomain,
         DnsErrorDomain
    };
    Q_ENUM(ErrorDomain)

    QWebEngineLoadingInfo(const QWebEngineLoadingInfo &other);
    QWebEngineLoadingInfo &operator=(const QWebEngineLoadingInfo &other);
    QWebEngineLoadingInfo(QWebEngineLoadingInfo &&other);
    QWebEngineLoadingInfo &operator=(QWebEngineLoadingInfo &&other);
    ~QWebEngineLoadingInfo();

    QUrl url() const;
    bool isErrorPage() const;
    LoadStatus status() const;
    QString errorString() const;
    ErrorDomain errorDomain() const;
    int errorCode() const;

private:
    QWebEngineLoadingInfo(const QUrl &url, LoadStatus status, bool isErrorPage = false,
                          const QString &errorString = QString(), int errorCode = 0,
                          ErrorDomain errorDomain = NoErrorDomain);
    class QWebEngineLoadingInfoPrivate;
    Q_DECLARE_PRIVATE(QWebEngineLoadingInfo)
    QExplicitlySharedDataPointer<QWebEngineLoadingInfoPrivate> d_ptr;
    friend class QQuickWebEngineViewPrivate;
    friend class QtWebEngineCore::WebContentsAdapter;
    friend class QtWebEngineCore::WebContentsDelegateQt;
};

QT_END_NAMESPACE

#endif // QWEBENGINELOADINGINFO_H
