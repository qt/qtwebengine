// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINEURLRESPONSEINTERCEPTOR_H
#define QWEBENGINEURLRESPONSEINTERCEPTOR_H

#include <QtWebEngineCore/qtwebenginecoreglobal.h>

#include <QtCore/QObject>

QT_BEGIN_NAMESPACE

class QWebEngineUrlResponseInfo;

class Q_WEBENGINECORE_EXPORT QWebEngineUrlResponseInterceptor : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(QWebEngineUrlResponseInterceptor)
public:
    explicit QWebEngineUrlResponseInterceptor(QObject *p = nullptr) : QObject(p) { }

    virtual void interceptResponseHeaders(QWebEngineUrlResponseInfo &info) = 0;
};

QT_END_NAMESPACE

#endif // QWEBENGINEURLRESPONSEINTERCEPTOR_H
