// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINEFULLSCREENREQUEST_H
#define QWEBENGINEFULLSCREENREQUEST_H

#include <QtWebEngineCore/qtwebenginecoreglobal.h>

#include <QtCore/qglobal.h>
#include <QtCore/qshareddata.h>
#include <QtCore/qurl.h>

#include <functional>

QT_BEGIN_NAMESPACE

class QWebEngineFullScreenRequestPrivate;

class Q_WEBENGINECORE_EXPORT QWebEngineFullScreenRequest
{
    Q_GADGET
    Q_PROPERTY(bool toggleOn READ toggleOn CONSTANT)
    Q_PROPERTY(QUrl origin READ origin CONSTANT)

public:
    QWebEngineFullScreenRequest(const QWebEngineFullScreenRequest &other);
    QWebEngineFullScreenRequest &operator=(const QWebEngineFullScreenRequest &other);
    QWebEngineFullScreenRequest(QWebEngineFullScreenRequest &&other);
    QWebEngineFullScreenRequest &operator=(QWebEngineFullScreenRequest &&other);
    ~QWebEngineFullScreenRequest();

    Q_INVOKABLE void reject();
    Q_INVOKABLE void accept();
    bool toggleOn() const;
    QUrl origin() const;

private:
    friend class QWebEnginePagePrivate;
    friend class QQuickWebEngineViewPrivate;
    QWebEngineFullScreenRequest(const QUrl &origin, bool toggleOn, const std::function<void (bool)> &setFullScreenCallback);
    QExplicitlySharedDataPointer<QWebEngineFullScreenRequestPrivate> d_ptr;
};

QT_END_NAMESPACE

#endif
