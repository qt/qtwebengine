// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "mainwindow.h"
#include <QWebEngineProfile>
#include <QWebEnginePermission>
#include <QWebEngineSettings>
#include <QMetaEnum>
#include <QUrl>

PermissionDialog::PermissionDialog(const QWebEngineProfile *profile, QWidget *parent)
    : QDialog(parent), m_profile(profile)
{
    setupUi(this);

    auto metaEnum = QMetaEnum::fromType<QWebEnginePermission::PermissionType>();
    for (int i = 0; i < metaEnum.keyCount(); ++i) {
        auto permissionType = QWebEnginePermission::PermissionType(metaEnum.value(i));
        if (QWebEnginePermission::isPersistent(permissionType))
            m_permissionTypeComboBox->addItem(metaEnum.key(i), QVariant::fromValue(permissionType));
    }
}

QWebEnginePermission PermissionDialog::permission() const
{
    return m_profile->queryPermission(
            QUrl(m_originLineEdit->text()),
            m_permissionTypeComboBox->currentData().value<QWebEnginePermission::PermissionType>());
}

PermissionWidget::PermissionWidget(const QWebEnginePermission &permission, QWidget *parent)
    : QWidget(parent)
    , m_permission(permission)
{
    setupUi(this);
    connect(m_deleteButton, &QPushButton::clicked, [this]() {
        m_permission.reset();
        emit permissionModified(this);
        deleteLater();
    });

    connect(m_grantButton, &QPushButton::clicked, [this]() {
        m_permission.grant();
        updateState();
        emit permissionModified(this);
    });

    connect(m_denyButton, &QPushButton::clicked, [this]() {
        m_permission.deny();
        updateState();
        emit permissionModified(this);
    });

    updateState();
}

void PermissionWidget::updateState()
{
    switch (m_permission.state()) {
    case QWebEnginePermission::State::Invalid:
        m_stateLabel->setText("<font color='gray'>Invalid</font>");
        m_grantButton->setEnabled(false);
        m_denyButton->setEnabled(false);
        break;
    case QWebEnginePermission::State::Ask:
        m_stateLabel->setText("<font color='yellow'>Waiting for response</font>");
        break;
    case QWebEnginePermission::State::Granted:
        m_stateLabel->setText("<font color='green'>Granted</font>");
        break;
    case QWebEnginePermission::State::Denied:
        m_stateLabel->setText("<font color='red'>Denied</font>");
        break;
    }

    m_typeLabel->setText(QMetaEnum::fromType<QWebEnginePermission::PermissionType>().valueToKey((quint8)m_permission.permissionType()));
    m_originLabel->setText(m_permission.origin().toDisplayString());
}

MainWindow::MainWindow(const QUrl &url)
    : QMainWindow()
    , m_layout(new QVBoxLayout)
    , m_profile(new QWebEngineProfile("permissionbrowser"))
    , m_webview(new QWebEngineView(m_profile, this))
    , m_pendingWidget(nullptr)
{
    setupUi(this);

    auto metaEnum = QMetaEnum::fromType<QWebEngineProfile::PersistentPermissionsPolicy>();
    for (int i = 0; i < metaEnum.keyCount(); ++i) {
        auto policy = QWebEngineProfile::PersistentPermissionsPolicy(metaEnum.value(i));
        m_policyComboBox->addItem(metaEnum.key(i), QVariant::fromValue(policy));
    }

    m_urlLineEdit->setText(url.toString());

    m_layout->addItem(new QSpacerItem(0,0, QSizePolicy::Minimum, QSizePolicy::Expanding));
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    QWidget *w = new QWidget();
    w->setLayout(m_layout);

    m_storedScrollArea->setWidget(w);
    m_storedScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    (new QVBoxLayout(m_pendingFrame))->setContentsMargins(0, 0, 0, 0);

    loadStoredPermissions();

    connect(m_deleteAllButton, &QPushButton::clicked, this, &MainWindow::handleDeleteAllClicked);
    connect(m_newButton, &QPushButton::clicked, this, &MainWindow::handleNewClicked);
    connect(m_refreshButton, &QPushButton::clicked, this, &MainWindow::handleRefreshClicked);
    connect(m_backButton, &QPushButton::clicked, this, &MainWindow::handleBackClicked);
    connect(m_forwardButton, &QPushButton::clicked, this, &MainWindow::handleForwardClicked);
    connect(m_policyComboBox, &QComboBox::currentIndexChanged, this, &MainWindow::handlePolicyComboBoxIndexChanged);
    connect(m_webview, &QWebEngineView::urlChanged, this, &MainWindow::handleUrlChanged);
    connect(m_webview->page(), &QWebEnginePage::permissionRequested, this, &MainWindow::handlePermissionRequested);

    m_profile->settings()->setAttribute(QWebEngineSettings::ScreenCaptureEnabled, true);
    m_profile->settings()->setAttribute(QWebEngineSettings::JavascriptCanAccessClipboard, true);

    m_frame->layout()->addWidget(m_webview);
    static_cast<QVBoxLayout *>(m_frame->layout())->setStretchFactor(m_webview, 1);
    m_webview->load(url);
}

MainWindow::~MainWindow()
{
    delete m_webview;
    delete m_profile;
}

void MainWindow::handlePermissionRequested(QWebEnginePermission permission)
{
    PermissionWidget *widget = createPermissionWidget(permission);
    if (widget) {
        m_pendingFrame->layout()->addWidget(widget);
        connect(widget, &PermissionWidget::permissionModified, this, &MainWindow::handlePermissionModified);

        if (m_pendingWidget)
            m_pendingWidget->deleteLater();

        m_pendingWidget = widget;
    }
}

void MainWindow::handlePermissionModified(PermissionWidget *widget)
{
    if (!m_pendingWidget || m_pendingWidget != widget)
        return;

    m_pendingFrame->layout()->removeWidget(widget);
    m_pendingWidget = nullptr;

    if (!QWebEnginePermission::isPersistent(widget->m_permission.permissionType())
        || widget->m_permission.state() == QWebEnginePermission::State::Ask
        || m_profile->persistentPermissionsPolicy() == QWebEngineProfile::PersistentPermissionsPolicy::AskEveryTime) {

        widget->deleteLater();
        return;
    }

    m_layout->insertWidget(0, widget);
}

void MainWindow::handleUrlChanged(const QUrl &url)
{
    m_urlLineEdit->setText(url.toString());
}

void MainWindow::handleDeleteAllClicked()
{
    for (int i = m_layout->count() - 1; i >= 0; i--) {
        PermissionWidget *widget = qobject_cast<PermissionWidget *>(m_layout->itemAt(i)->widget());
        if (!widget)
            continue;

        widget->m_permission.reset();
        widget->deleteLater();
    }
}

void MainWindow::handleNewClicked()
{
    PermissionDialog dialog(m_profile);
    if (dialog.exec() == QDialog::Accepted) {
        handlePermissionRequested(dialog.permission());
    }
}

void MainWindow::handleRefreshClicked()
{
    m_webview->load(QUrl::fromUserInput(m_urlLineEdit->text()));
}

void MainWindow::handleBackClicked()
{
    m_webview->triggerPageAction(QWebEnginePage::Back);
}

void MainWindow::handleForwardClicked()
{
    m_webview->triggerPageAction(QWebEnginePage::Forward);
}

void MainWindow::handlePolicyComboBoxIndexChanged(int)
{
    auto policy =
            m_policyComboBox->currentData().value<QWebEngineProfile::PersistentPermissionsPolicy>();
    if (policy == m_profile->persistentPermissionsPolicy())
        return;

    for (int i = m_layout->count() - 1; i >= 0; i--) {
        PermissionWidget *widget = qobject_cast<PermissionWidget *>(m_layout->itemAt(i)->widget());
        if (!widget)
            continue;

        widget->deleteLater();
    }

    m_profile->setPersistentPermissionsPolicy(policy);
    loadStoredPermissions();
}

bool MainWindow::containsPermission(const QWebEnginePermission &permission)
{
    for (const auto *w: std::as_const(m_storedScrollArea->widget()->children())) {
        const PermissionWidget *widget = qobject_cast<const PermissionWidget *>(w);
        if (!widget)
            continue;
        const QWebEnginePermission &widgetPermission = widget->m_permission;
        if (widgetPermission == permission)
            return true;
    }

    if (m_pendingWidget && m_pendingWidget->m_permission == permission)
        return true;

    return false;
}

PermissionWidget *MainWindow::createPermissionWidget(const QWebEnginePermission &permission)
{
    if (containsPermission(permission))
        return nullptr;

    return new PermissionWidget(permission, this);
}

void MainWindow::loadStoredPermissions()
{
    QList<QWebEnginePermission> permissionsList = m_profile->listAllPermissions();
    for (QWebEnginePermission &permission : permissionsList) {
        PermissionWidget *widget = createPermissionWidget(permission);
        if (widget)
            m_layout->insertWidget(0, widget);
    }
}
