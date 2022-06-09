// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef WEBUIHANDLER_H
#define WEBUIHANDLER_H

#include <QWebEngineUrlSchemeHandler>

class WebUiHandler : public QWebEngineUrlSchemeHandler
{
    Q_OBJECT
public:
    explicit WebUiHandler(QObject *parent = nullptr);

    void requestStarted(QWebEngineUrlRequestJob *job) override;

    static void registerUrlScheme();

    const static QByteArray schemeName;
    const static QUrl aboutUrl;
};

#endif // !WEBUIHANDLER_H
