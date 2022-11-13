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

inline QString questionForFeature(QWebEnginePage::Feature feature)
{
    switch (feature) {
    case QWebEnginePage::Geolocation:
        return WebPage::tr("Allow %1 to access your location information?");
    case QWebEnginePage::MediaAudioCapture:
        return WebPage::tr("Allow %1 to access your microphone?");
    case QWebEnginePage::MediaVideoCapture:
        return WebPage::tr("Allow %1 to access your webcam?");
    case QWebEnginePage::MediaAudioVideoCapture:
        return WebPage::tr("Allow %1 to access your microphone and webcam?");
    case QWebEnginePage::MouseLock:
        return WebPage::tr("Allow %1 to lock your mouse cursor?");
    case QWebEnginePage::DesktopVideoCapture:
        return WebPage::tr("Allow %1 to capture video of your desktop?");
    case QWebEnginePage::DesktopAudioVideoCapture:
        return WebPage::tr("Allow %1 to capture audio and video of your desktop?");
    case QWebEnginePage::Notifications:
        return WebPage::tr("Allow %1 to show notification on your desktop?");
    }
    return QString();
}

void WebPage::handleSelectClientCertificate(QWebEngineClientCertificateSelection selection)
{
    // Just select one.
    selection.select(selection.certificates().at(0));
}
