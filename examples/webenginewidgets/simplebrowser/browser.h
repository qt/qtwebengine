// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef BROWSER_H
#define BROWSER_H

#include "downloadmanagerwidget.h"

#include <QList>
#include <QWebEngineProfile>

class BrowserWindow;

class Browser
{
public:
    Browser();

    QList<BrowserWindow*> windows() { return m_windows; }

    BrowserWindow *createHiddenWindow(bool offTheRecord = false);
    BrowserWindow *createWindow(bool offTheRecord = false);
    BrowserWindow *createDevToolsWindow();

    DownloadManagerWidget &downloadManagerWidget() { return m_downloadManagerWidget; }

private:
    QList<BrowserWindow*> m_windows;
    DownloadManagerWidget m_downloadManagerWidget;
    QScopedPointer<QWebEngineProfile> m_profile;
};
#endif // BROWSER_H
