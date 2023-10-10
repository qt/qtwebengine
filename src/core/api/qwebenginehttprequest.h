// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINEHTTPREQUEST_H
#define QWEBENGINEHTTPREQUEST_H

#include <QtWebEngineCore/qtwebenginecoreglobal.h>
#include <QtCore/qlist.h>
#include <QtCore/qmap.h>
#include <QtCore/qshareddata.h>
#include <QtCore/qstring.h>
#include <QtCore/qurl.h>

QT_BEGIN_NAMESPACE

class QWebEngineHttpRequestPrivate;

class Q_WEBENGINECORE_EXPORT QWebEngineHttpRequest
{
public:
    enum Method {
        Get,
        Post
    };

    explicit QWebEngineHttpRequest(const QUrl &url = QUrl(),
                                   const QWebEngineHttpRequest::Method &method = QWebEngineHttpRequest::Get);
    QWebEngineHttpRequest(const QWebEngineHttpRequest &other);
    ~QWebEngineHttpRequest();
#ifdef Q_COMPILER_RVALUE_REFS
    QWebEngineHttpRequest &operator=(QWebEngineHttpRequest &&other) Q_DECL_NOTHROW
    {
        swap(other);
        return *this;
    }
#endif
    QWebEngineHttpRequest &operator=(const QWebEngineHttpRequest &other);

    static QWebEngineHttpRequest postRequest(const QUrl &url, const QMap<QString, QString> &postData);
    void swap(QWebEngineHttpRequest &other) noexcept { d.swap(other.d); }

    bool operator==(const QWebEngineHttpRequest &other) const;
    inline bool operator!=(const QWebEngineHttpRequest &other) const { return !operator==(other); }

    Method method() const;
    void setMethod(QWebEngineHttpRequest::Method method);

    QUrl url() const;
    void setUrl(const QUrl &url);

    QByteArray postData() const;
    void setPostData(const QByteArray &postData);

    bool hasHeader(const QByteArray &headerName) const;
    QList<QByteArray> headers() const;
    QByteArray header(const QByteArray &headerName) const;
    void setHeader(const QByteArray &headerName, const QByteArray &value);
    void unsetHeader(const QByteArray &headerName);

private:
    QSharedDataPointer<QWebEngineHttpRequestPrivate> d;
};

Q_DECLARE_SHARED(QWebEngineHttpRequest)

QT_END_NAMESPACE

#endif
