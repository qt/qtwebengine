// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "notificationpopup.h"

#include <QApplication>
#include <QDesktopServices>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineView>

class WebEnginePage : public QWebEnginePage
{
public:
    WebEnginePage(QWidget *parent) : QWebEnginePage(parent) { }

    bool acceptNavigationRequest(const QUrl &url, NavigationType, bool) override
    {
        if (url.scheme() != "https")
            return true;
        QDesktopServices::openUrl(url);
        return false;
    }
};

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("QtExamples");
    QApplication app(argc, argv);

    QWebEngineView view;

    // set custom page to open all page's links for https scheme in system browser
    view.setPage(new WebEnginePage(&view));

    QObject::connect(view.page(), &QWebEnginePage::featurePermissionRequested,
                     [&] (const QUrl &origin, QWebEnginePage::Feature feature) {
                         if (feature != QWebEnginePage::Notifications)
                             return;
                         view.page()->setFeaturePermission(origin, feature, QWebEnginePage::PermissionGrantedByUser);
                     });

    auto profile = view.page()->profile();
    auto popup = new NotificationPopup(&view);
    profile->setNotificationPresenter([&] (std::unique_ptr<QWebEngineNotification> notification)
                                      { popup->present(notification); });

    view.resize(640, 480);
    view.show();
    view.setUrl(QStringLiteral("qrc:/index.html"));
    return app.exec();
}

