// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"
#include "ui_cookiewidget.h"
#include "ui_cookiedialog.h"
#include <QNetworkCookie>
#include <QMainWindow>

QT_BEGIN_NAMESPACE
class QWebEngineCookieStore;
QT_END_NAMESPACE

class CookieDialog : public QDialog, public Ui_CookieDialog
{
    Q_OBJECT
public:
    CookieDialog(const QNetworkCookie &cookie, QWidget *parent = nullptr);
    CookieDialog(QWidget *parent = 0);
    QNetworkCookie cookie();
};

class CookieWidget : public QWidget,  public Ui_CookieWidget
{
    Q_OBJECT
public:
    CookieWidget(const QNetworkCookie &cookie, QWidget *parent = nullptr);
    void setHighlighted(bool enabled);
signals:
    void deleteClicked();
    void viewClicked();
};

class MainWindow : public QMainWindow, public Ui_MainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(const QUrl &url);

private:
    bool containsCookie(const QNetworkCookie &cookie);

private slots:
    void handleCookieAdded(const QNetworkCookie &cookie);
    void handleDeleteAllClicked();
    void handleNewClicked();
    void handleUrlClicked();

private:
    QWebEngineCookieStore *m_store;
    QList<QNetworkCookie> m_cookies;
    QVBoxLayout *m_layout;
};

#endif // MAINWINDOW_H
