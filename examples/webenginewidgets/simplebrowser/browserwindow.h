// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef BROWSERWINDOW_H
#define BROWSERWINDOW_H

#include <QMainWindow>
#include <QTime>
#include <QWebEnginePage>

QT_BEGIN_NAMESPACE
class QLineEdit;
class QProgressBar;
QT_END_NAMESPACE

class Browser;
class TabWidget;
class WebView;

class BrowserWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit BrowserWindow(Browser *browser, QWebEngineProfile *profile,
                           bool forDevTools = false);
    QSize sizeHint() const override;
    TabWidget *tabWidget() const;
    WebView *currentTab() const;
    Browser *browser() { return m_browser; }

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void handleNewWindowTriggered();
    void handleNewIncognitoWindowTriggered();
    void handleFileOpenTriggered();
    void handleFindActionTriggered();
    void handleShowWindowTriggered();
    void handleWebViewLoadProgress(int);
    void handleWebViewTitleChanged(const QString &title);
    void handleWebActionEnabledChanged(QWebEnginePage::WebAction action, bool enabled);
    void handleDevToolsRequested(QWebEnginePage *source);
    void handleFindTextFinished(const QWebEngineFindTextResult &result);

private:
    QMenu *createFileMenu(TabWidget *tabWidget);
    QMenu *createEditMenu();
    QMenu *createViewMenu(QToolBar *toolBar);
    QMenu *createWindowMenu(TabWidget *tabWidget);
    QMenu *createHelpMenu();
    QToolBar *createToolBar();

private:
    Browser *m_browser;
    QWebEngineProfile *m_profile;
    TabWidget *m_tabWidget;
    QProgressBar *m_progressBar = nullptr;
    QAction *m_historyBackAction = nullptr;
    QAction *m_historyForwardAction = nullptr;
    QAction *m_stopAction = nullptr;
    QAction *m_reloadAction = nullptr;
    QAction *m_stopReloadAction = nullptr;
    QLineEdit *m_urlLineEdit = nullptr;
    QAction *m_favAction = nullptr;
    QString m_lastSearch;
};

#endif // BROWSERWINDOW_H
