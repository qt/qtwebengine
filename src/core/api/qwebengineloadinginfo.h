// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINELOADINGINFO_H
#define QWEBENGINELOADINGINFO_H

#include <QtWebEngineCore/qtwebenginecoreglobal.h>

#include <QtCore/qmap.h>
#include <QtCore/qobject.h>
#include <QtCore/qshareddata.h>
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
    Q_PROPERTY(QMultiMap<QByteArray,QByteArray> responseHeaders READ responseHeaders CONSTANT REVISION(6,6) FINAL)

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
         DnsErrorDomain,
         HttpStatusCodeDomain
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
    QMultiMap<QByteArray,QByteArray> responseHeaders() const;

private:
    QWebEngineLoadingInfo(const QUrl &url, LoadStatus status, bool isErrorPage = false,
                          const QString &errorString = QString(), int errorCode = 0,
                          ErrorDomain errorDomain = NoErrorDomain,
                          const QMultiMap<QByteArray,QByteArray> &responseHeaders = {});
    class QWebEngineLoadingInfoPrivate;
    Q_DECLARE_PRIVATE(QWebEngineLoadingInfo)
    QExplicitlySharedDataPointer<QWebEngineLoadingInfoPrivate> d_ptr;
    friend class QQuickWebEngineViewPrivate;
    friend class QtWebEngineCore::WebContentsAdapter;
    friend class QtWebEngineCore::WebContentsDelegateQt;
};

QT_END_NAMESPACE

#endif // QWEBENGINELOADINGINFO_H
