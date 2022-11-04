// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINEURLSCHEMEHANDLER_H
#define QWEBENGINEURLSCHEMEHANDLER_H

#include <QtWebEngineCore/qtwebenginecoreglobal.h>

#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE

class QWebEngineUrlRequestJob;

class Q_WEBENGINECORE_EXPORT QWebEngineUrlSchemeHandler : public QObject
{
    Q_OBJECT
public:
    QWebEngineUrlSchemeHandler(QObject *parent = nullptr);
    ~QWebEngineUrlSchemeHandler();

    virtual void requestStarted(QWebEngineUrlRequestJob *) = 0;

private:
    Q_DISABLE_COPY(QWebEngineUrlSchemeHandler)
};

QT_END_NAMESPACE

#endif // QWEBENGINEURLSCHEMEHANDLER_H
