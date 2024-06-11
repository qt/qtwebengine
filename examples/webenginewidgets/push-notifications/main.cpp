// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "notificationpopup.h"

#include <QApplication>
#include <QWebEngineProfile>
#include <QWebEnginePage>
#include <QWebEngineView>

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("QtExamples");
    QApplication app(argc, argv);

    const QString name =
            QString::fromLatin1("push-notifications.%1").arg(qWebEngineChromiumVersion());

    QScopedPointer<QWebEngineProfile> profile(new QWebEngineProfile(name));
    QWebEngineView view(profile.data());
    auto popup = new NotificationPopup(&view);

    QObject::connect(view.page(), &QWebEnginePage::featurePermissionRequested,
                     [&](const QUrl &origin, QWebEnginePage::Feature feature) {
                         if (feature != QWebEnginePage::Notifications)
                             return;

                         view.page()->setFeaturePermission(origin, feature,
                                                           QWebEnginePage::PermissionGrantedByUser);
                     });

    profile->setPushServiceEnabled(true);
    profile->setNotificationPresenter([&](std::unique_ptr<QWebEngineNotification> notification) {
        popup->present(notification);
    });

    view.resize(640, 480);
    view.setUrl(QUrl("http://localhost:5000"));
    view.show();
    return app.exec();
}
