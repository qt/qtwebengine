// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "ui_mainwindow.h"
#include "ui_permissiondialog.h"
#include "ui_permissionwidget.h"
#include <QDialog>
#include <QMainWindow>
#include <QWebEngineView>
#include <QWebEnginePermission>

QT_BEGIN_NAMESPACE
class QWebEngineView;
class QWebEngineProfile;
class QLineEdit;
QT_END_NAMESPACE

class PermissionDialog : public QDialog, public Ui_PermissionDialog
{
    Q_OBJECT
public:
    PermissionDialog(const QWebEngineProfile *profile, QWidget *parent = nullptr);
    ~PermissionDialog();
    QWebEnginePermission permission();

private:
    const QWebEngineProfile *m_profile;
    QWebEnginePermission *m_permission;
};

class PermissionWidget : public QWidget, public Ui_PermissionWidget
{
    Q_OBJECT
public:
    PermissionWidget(const QWebEnginePermission &permission, QWidget *parent = nullptr);

    QWebEnginePermission m_permission;

signals:
    void permissionModified(PermissionWidget *widget);

private:
    void updateState();
};

class MainWindow : public QMainWindow, public Ui_MainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(const QUrl &url);
    ~MainWindow();

private slots:
    void handlePermissionRequested(QWebEnginePermission permission);
    void handleUrlChanged(const QUrl &url);

    void handlePermissionModified(PermissionWidget *widget);
    void handleDeleteAllClicked();
    void handleNewClicked();
    void handleRefreshClicked();
    void handleBackClicked();
    void handleForwardClicked();
    void handlePolicyComboBoxIndexChanged(int index);

private:
    bool containsPermission(const QWebEnginePermission &permission);
    PermissionWidget *createPermissionWidget(const QWebEnginePermission &permission);
    void loadStoredPermissions();

    QVBoxLayout *m_layout;
    QWebEngineProfile *m_profile;
    QWebEngineView *m_webview;
    PermissionWidget *m_pendingWidget;
};
