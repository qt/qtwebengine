// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "mainwindow.h"
#include <QWebEngineCookieStore>
#include <QWebEngineProfile>

CookieDialog::CookieDialog(const QNetworkCookie &cookie, QWidget *parent): QDialog(parent)
{
    setupUi(this);
    m_nameLineEdit->setText(cookie.name());
    m_domainLineEdit->setText(cookie.domain());
    m_valueLineEdit->setText(cookie.value());
    m_pathLineEdit->setText(cookie.path());
    m_dateEdit->setDate(cookie.expirationDate().date());
    m_isSecureComboBox->addItem(cookie.isSecure() ? tr("yes") : tr("no"));
    m_isHttpOnlyComboBox->addItem(cookie.isHttpOnly() ? tr("yes") : tr("no"));
    m_addButton->setVisible(false);
    m_cancelButton->setText(tr("Close"));
}

CookieDialog::CookieDialog(QWidget *parent): QDialog(parent)
{
    setupUi(this);
    m_nameLineEdit->setReadOnly(false);
    m_domainLineEdit->setReadOnly(false);
    m_valueLineEdit->setReadOnly(false);
    m_pathLineEdit->setReadOnly(false);
    m_dateEdit->setReadOnly(false);
    m_dateEdit->setDate(QDateTime::currentDateTime().addYears(1).date());
    m_isSecureComboBox->addItem(tr("no"));
    m_isSecureComboBox->addItem(tr("yes"));
    m_isHttpOnlyComboBox->addItem(tr("no"));
    m_isHttpOnlyComboBox->addItem(tr("yes"));
}

QNetworkCookie CookieDialog::cookie()
{
    QNetworkCookie cookie;
    cookie.setDomain(m_domainLineEdit->text());
    cookie.setName(m_nameLineEdit->text().toLatin1());
    cookie.setValue(m_valueLineEdit->text().toLatin1());
    cookie.setExpirationDate(QDateTime(m_dateEdit->date(), QTime::currentTime()));
    cookie.setPath(m_pathLineEdit->text());
    cookie.setSecure(m_isSecureComboBox->currentText() == tr("yes"));
    cookie.setHttpOnly(m_isHttpOnlyComboBox->currentText() == tr("yes"));
    return cookie;
}

CookieWidget::CookieWidget(const QNetworkCookie &cookie, QWidget *parent): QWidget(parent)
{
    setupUi(this);
    setAutoFillBackground(true);
    m_nameLabel->setText(cookie.name());
    m_domainLabel->setText(cookie.domain());
    connect(m_viewButton, &QPushButton::clicked, this, &CookieWidget::viewClicked);
    connect(m_deleteButton, &QPushButton::clicked, this, &CookieWidget::deleteClicked);
}

void CookieWidget::setHighlighted(bool enabled)
{
    QPalette p = palette();
    p.setColor(backgroundRole(), enabled ? p.alternateBase().color() : p.base().color());
    setPalette(p);
}

MainWindow::MainWindow(const QUrl &url) :
    QMainWindow(),
    m_store(nullptr),
    m_layout(new QVBoxLayout)
{
    setupUi(this);
    m_urlLineEdit->setText(url.toString());

    m_layout->addItem(new QSpacerItem(0,0, QSizePolicy::Minimum, QSizePolicy::Expanding));
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    QWidget *w = new QWidget();
    QPalette p = w->palette();
    p.setColor(widget->backgroundRole(), Qt::white);
    w->setPalette(p);
    w->setLayout(m_layout);

    m_scrollArea->setWidget(w);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    connect(m_urlButton, &QPushButton::clicked, this, &MainWindow::handleUrlClicked);
    connect(m_deleteAllButton, &QPushButton::clicked, this, &MainWindow::handleDeleteAllClicked);
    connect(m_newButton, &QPushButton::clicked, this, &MainWindow::handleNewClicked);

    m_store = m_webview->page()->profile()->cookieStore();
    connect(m_store, &QWebEngineCookieStore::cookieAdded, this, &MainWindow::handleCookieAdded);
    m_store->loadAllCookies();
    m_webview->load(url);
}

bool MainWindow::containsCookie(const QNetworkCookie &cookie)
{
    for (auto c: m_cookies) {
        if (c.hasSameIdentifier(cookie))
            return true;
    }
    return false;
}

void MainWindow::handleCookieAdded(const QNetworkCookie &cookie)
{
    // only new cookies
    if (containsCookie(cookie))
        return;

    CookieWidget *widget = new CookieWidget(cookie);
    widget->setHighlighted(m_cookies.count() % 2);
    m_cookies.append(cookie);
    m_layout->insertWidget(0,widget);

    connect(widget, &CookieWidget::deleteClicked, [this, cookie, widget]() {
        m_store->deleteCookie(cookie);
        delete widget;
        m_cookies.removeOne(cookie);
        for (int i = 0; i < m_layout->count() - 1; i++) {
            // fix background colors
            auto widget = qobject_cast<CookieWidget*>(m_layout->itemAt(i)->widget());
            widget->setHighlighted(i % 2);
        }
    });

    connect(widget, &CookieWidget::viewClicked, [cookie]() {
        CookieDialog dialog(cookie);
        dialog.exec();
    });
}

void MainWindow::handleDeleteAllClicked()
{
    m_store->deleteAllCookies();
    for (int i = m_layout->count() - 1; i >= 0; i--)
        delete m_layout->itemAt(i)->widget();
    m_cookies.clear();
}

void MainWindow::handleNewClicked()
{
    CookieDialog dialog;
    if (dialog.exec() == QDialog::Accepted)
        m_store->setCookie(dialog.cookie());
}

void MainWindow::handleUrlClicked()
{
    m_webview->load(QUrl::fromUserInput(m_urlLineEdit->text()));
}
