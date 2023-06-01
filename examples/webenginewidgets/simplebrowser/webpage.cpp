// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "browserwindow.h"
#include "tabwidget.h"

#include "webpage.h"
#include "webview.h"
#include <QTimer>

WebPage::WebPage(QWebEngineProfile *profile, QObject *parent)
    : QWebEnginePage(profile, parent)
{
    connect(this, &QWebEnginePage::selectClientCertificate, this, &WebPage::handleSelectClientCertificate);
    connect(this, &QWebEnginePage::certificateError, this, &WebPage::handleCertificateError);
}

void WebPage::handleCertificateError(QWebEngineCertificateError error)
{
    error.defer();
    QTimer::singleShot(0, this,
                       [this, error]() mutable { emit createCertificateErrorDialog(error); });
}

void WebPage::handleSelectClientCertificate(QWebEngineClientCertificateSelection selection)
{
    // Just select one.
    selection.select(selection.certificates().at(0));
}
