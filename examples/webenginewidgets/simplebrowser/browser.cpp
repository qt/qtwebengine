// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "browser.h"
#include "browserwindow.h"

#include <QWebEngineSettings>

Browser::Browser()
{
    // Quit application if the download manager window is the only remaining window
    m_downloadManagerWidget.setAttribute(Qt::WA_QuitOnClose, false);

    QObject::connect(
        QWebEngineProfile::defaultProfile(), &QWebEngineProfile::downloadRequested,
        &m_downloadManagerWidget, &DownloadManagerWidget::downloadRequested);
}

BrowserWindow *Browser::createWindow(bool offTheRecord)
{
    if (!offTheRecord && !m_profile) {
        m_profile.reset(new QWebEngineProfile(
                QString::fromLatin1("simplebrowser.%1").arg(qWebEngineChromiumVersion())));
        m_profile->settings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
        m_profile->settings()->setAttribute(QWebEngineSettings::DnsPrefetchEnabled, true);
        QObject::connect(m_profile.get(), &QWebEngineProfile::downloadRequested,
                         &m_downloadManagerWidget, &DownloadManagerWidget::downloadRequested);
    }
    auto profile = !offTheRecord ? m_profile.get() : QWebEngineProfile::defaultProfile();
    auto mainWindow = new BrowserWindow(this, profile, false);
    m_windows.append(mainWindow);
    QObject::connect(mainWindow, &QObject::destroyed, [this, mainWindow]() {
        m_windows.removeOne(mainWindow);
    });
    mainWindow->show();
    return mainWindow;
}

BrowserWindow *Browser::createDevToolsWindow()
{
    auto profile = m_profile ? m_profile.get() : QWebEngineProfile::defaultProfile();
    auto mainWindow = new BrowserWindow(this, profile, true);
    m_windows.append(mainWindow);
    QObject::connect(mainWindow, &QObject::destroyed, [this, mainWindow]() {
        m_windows.removeOne(mainWindow);
    });
    mainWindow->show();
    return mainWindow;
}
