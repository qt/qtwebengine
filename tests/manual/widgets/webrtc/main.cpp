// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QApplication>
#include <QByteArray>
#include <QDialog>
#include <QFile>
#include <QHttpServer>
#include <QListView>
#include <QMessageBox>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QWebEngineView>

#include "ui_mediaPicker.h"
#include <QWebEngineDesktopMediaRequest>

// Test the screen/window selection and capturing APIs using QWebEngineDesktopMediaRequest,
// getDisplayMedia (js) and chooseDesktopMedia (hangouts)

// Note: Wayland compositors require Pipewire support in QWE

class Page : public QWebEnginePage
{
    Q_OBJECT

public:
    Page(QWebEngineProfile *profile, QObject *parent = nullptr);
private slots:
    void handlePermissionRequest(const QUrl &origin, Feature feature);
    void handleDesktopMediaRequest(const QWebEngineDesktopMediaRequest &request);
};

Page::Page(QWebEngineProfile *profile, QObject *parent) : QWebEnginePage(profile, parent)
{
    settings()->setAttribute(QWebEngineSettings::ScreenCaptureEnabled, true);
    connect(this, &QWebEnginePage::permissionRequested, this,
            &Page::handlePermissionRequest);
    connect(this, &QWebEnginePage::desktopMediaRequested, this, &Page::handleDesktopMediaRequest);
}

void Page::handlePermissionRequest(QWebEnginePermission permission)
{
    if (QMessageBox::question(QApplication::activeWindow(), tr("Permission request"),
                              tr("allow access?"))
        == QMessageBox::Yes)
        permission.grant();
    else
        permission.deny();
}

void Page::handleDesktopMediaRequest(const QWebEngineDesktopMediaRequest &request)
{
    Ui::MediaPickerDialog mediaPickerDialog;
    QDialog dialog;
    dialog.setModal(true);
    mediaPickerDialog.setupUi(&dialog);

    auto *screensView = mediaPickerDialog.screensView;
    auto *windowsView = mediaPickerDialog.windowsView;
    auto *screensModel = request.screensModel();
    auto *windowsModel = request.windowsModel();

    screensView->setModel(screensModel);
    windowsView->setModel(windowsModel);

    if (dialog.exec() == QDialog::Accepted) {
        if (mediaPickerDialog.tabWidget->currentIndex() == 0)
            request.selectWindow(windowsView->selectionModel()->selectedIndexes().first());
        else
            request.selectScreen(screensView->selectionModel()->selectedIndexes().first());
    } else {
        request.cancel();
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QHttpServer server;

    QFile file(":index.html");

    if (!file.open(QIODeviceBase::ReadOnly)) {
        qWarning("failed to open file!");
        return 0;
    }

    QByteArray data = file.readAll();
    if (data.isEmpty()) {
        qWarning("failed to read file!");
        return 0;
    }

    server.route("/index.html", [data]() {
        return data;
    });

    server.listen(QHostAddress::Any, 3000);

    QWebEngineView view;
    Page *page = new Page(QWebEngineProfile::defaultProfile(), &view);
    view.setPage(page);
    view.resize(1024, 750);
    view.setUrl(QUrl("http://localhost:3000/index.html"));
    view.show();
    return app.exec();
}

#include "main.moc"
