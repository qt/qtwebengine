/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "widgetwindow.h"

#include "qwebcontentsview.h"
#include "util.h"

static const int margin = 1;

WidgetWindow::WidgetWindow()
: m_webView(new QWebContentsView)
, addressLineEdit(0)
{
    // Use oxygen as a fallback.
    if (QIcon::themeName().isEmpty())
      QIcon::setThemeName("oxygen");

    setGeometry(0, 0, 800, 600);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->setContentsMargins(margin, margin, margin, margin);

    // Create a widget based address bar.
    QHBoxLayout* addressBar = new QHBoxLayout;
    addressBar->setSpacing(margin); // Bigger buttons, less space between them

    QToolButton* backButton = new QToolButton;
    backButton->setIcon(QIcon::fromTheme("go-previous"));
    addressBar->addWidget(backButton);

    QToolButton* forwardButton = new QToolButton;
    forwardButton->setIcon(QIcon::fromTheme("go-next"));
    addressBar->addWidget(forwardButton);

    reloadButton = new QToolButton;
    reloadButton->setIcon(QIcon::fromTheme("view-refresh"));
    addressBar->addWidget(reloadButton);

    addressLineEdit =  new QLineEdit;
    addressBar->addWidget(addressLineEdit);

    layout->addLayout(addressBar);
    layout->addWidget(m_webView.data());

    setLayout(layout);

    connect(addressLineEdit, SIGNAL(returnPressed()), SLOT(loadAddressFromAddressBar()));
    connect(backButton, SIGNAL(clicked()), m_webView.data(), SLOT(back()));
    connect(forwardButton, SIGNAL(clicked()), m_webView.data(), SLOT(forward()));
    connect(reloadButton, SIGNAL(clicked()), m_webView.data(), SLOT(reload()));
    connect(m_webView.data(), SIGNAL(loadStarted()), SLOT(loadStarted()));
    connect(m_webView.data(), SIGNAL(loadFinished(bool)), SLOT(loadFinished(bool)));
    connect(m_webView.data(), SIGNAL(titleChanged(const QString&)), SLOT(setWindowTitle(const QString&)));
    connect(m_webView.data(), SIGNAL(urlChanged(const QUrl&)), SLOT(setAddressBarUrl(const QUrl&)));

    m_webView->load(startupUrl());
}

WidgetWindow::~WidgetWindow()
{
}

void WidgetWindow::loadAddressFromAddressBar()
{
    m_webView->load(addressLineEdit->text());
}

void WidgetWindow::setAddressBarUrl(const QUrl& url)
{
    addressLineEdit->setText(url.toString());
}

void WidgetWindow::loadStarted()
{
    reloadButton->setIcon(QIcon::fromTheme("process-stop"));
}

void WidgetWindow::loadFinished(bool success)
{
    Q_UNUSED(success);
    reloadButton->setIcon(QIcon::fromTheme("view-refresh"));
}
