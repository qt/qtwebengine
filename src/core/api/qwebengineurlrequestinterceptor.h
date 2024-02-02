// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENINGEURLREQUESTINTERCEPTOR_H
#define QWEBENINGEURLREQUESTINTERCEPTOR_H

#include <QtWebEngineCore/qtwebenginecoreglobal.h>
#include <QtWebEngineCore/qwebengineurlrequestinfo.h>

#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE

class Q_WEBENGINECORE_EXPORT QWebEngineUrlRequestInterceptor : public QObject
{
    Q_OBJECT
public:
    explicit QWebEngineUrlRequestInterceptor(QObject *p = nullptr) : QObject(p) {}
    ~QWebEngineUrlRequestInterceptor() override;
    virtual void interceptRequest(QWebEngineUrlRequestInfo &info) = 0;
};

QT_END_NAMESPACE

#endif // QWEBENINGEURLREQUESTINTERCEPTOR_H
