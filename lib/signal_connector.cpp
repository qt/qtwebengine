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

#include "signal_connector.h"

#include "content/shell/shell.h"
#include <QObject>
#include <QWidget>
#include <QLineEdit>
#include <QToolButton>
#include <QDebug>

SignalConnector::SignalConnector(content::Shell* shell, QWidget* window)
	: m_shell(shell)
	, m_window(window)
{
	setParent(window);
	m_addressLineEdit = m_window->findChild<QLineEdit*>("AddressLineEdit");
	m_backButton = m_window->findChild<QToolButton*>("BackButton");
	m_forwardButton = m_window->findChild<QToolButton*>("ForwardButton");
	m_reloadButton = m_window->findChild<QToolButton*>("ReloadButton");

	connect(m_addressLineEdit, SIGNAL(returnPressed()), this, SLOT(loadAddressFromAddressBar()));
	connect(m_backButton, SIGNAL(clicked()), this, SLOT(goBack()));
	connect(m_forwardButton, SIGNAL(clicked()), this, SLOT(goForward()));
	connect(m_reloadButton, SIGNAL(clicked()), this, SLOT(reload()));
}

void SignalConnector::loadAddressFromAddressBar()
{
	load(m_addressLineEdit->text());
}

void SignalConnector::load(const QString& url) const
{
	GURL gurl(url.toStdString());
	if (!gurl.has_scheme())
		gurl = GURL(std::string("http://") + url.toStdString());
	m_shell->LoadURL(gurl);
}

void SignalConnector::goBack() const
{
	m_shell->GoBackOrForward(-1);
}

void SignalConnector::goForward() const
{
	m_shell->GoBackOrForward(1);
}

void SignalConnector::reload() const
{
	m_shell->Reload();
}

