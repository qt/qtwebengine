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

#include "qquickwebcontentsview.h"

// Needed to get access to content::GetContentClient()
#define CONTENT_IMPLEMENTATION

#include "content/public/browser/web_contents.h"
#include "content/shell/shell.h"
#include "content/shell/shell_browser_context.h"

#include "content_browser_client_qt.h"

#include <QWidget>
#include <QUrl>

namespace content {
QQuickWebContentsView* gQuickView = 0;
}

class QQuickWebContentsViewPrivate
{
public:
    scoped_ptr<content::Shell> shell;
};

QQuickWebContentsView::QQuickWebContentsView()
{
    d.reset(new QQuickWebContentsViewPrivate);

    // Cheap hack to allow getting signals from shell_qt.cpp.
    Q_ASSERT(!content::gQuickView);
    content::gQuickView = this;

    content::BrowserContext* browser_context = static_cast<ContentBrowserClientQt*>(content::GetContentClient()->browser())->browser_context();
    d->shell.reset(content::Shell::CreateNewWindow(browser_context,
        GURL(std::string("http://qt-project.org/")),
        NULL,
        MSG_ROUTING_NONE,
        gfx::Size()));
}

QQuickWebContentsView::~QQuickWebContentsView()
{
}

QUrl QQuickWebContentsView::url() const
{
    GURL gurl = d->shell->web_contents()->GetActiveURL();
    return QUrl(QString::fromStdString(gurl.spec()));
}

void QQuickWebContentsView::setUrl(const QUrl& url)
{
    QString urlString = url.toString();
    GURL gurl(urlString.toStdString());
    if (!gurl.has_scheme())
        gurl = GURL(std::string("http://") + urlString.toStdString());
    d->shell->LoadURL(gurl);
}

void QQuickWebContentsView::goBack()
{
    d->shell->GoBackOrForward(-1);
}

void QQuickWebContentsView::goForward()
{
    d->shell->GoBackOrForward(1);
}

void QQuickWebContentsView::reload()
{
    d->shell->Reload();
}
