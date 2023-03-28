// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef WEBPAGE_H
#define WEBPAGE_H

#include <QWebEnginePage>
#include <QWebEngineRegisterProtocolHandlerRequest>
#include <QWebEngineCertificateError>

class WebPage : public QWebEnginePage
{
    Q_OBJECT

public:
    explicit WebPage(QWebEngineProfile *profile, QObject *parent = nullptr);

signals:
    void createCertificateErrorDialog(QWebEngineCertificateError error);

private slots:
    void handleCertificateError(QWebEngineCertificateError error);
    void handleSelectClientCertificate(QWebEngineClientCertificateSelection clientCertSelection);
};

#endif // WEBPAGE_H
