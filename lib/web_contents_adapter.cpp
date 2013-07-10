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
#include "web_contents_adapter.h"

#include "web_contents_view_qt.h"

#include "content/public/browser/web_contents.h"
#include "content/public/browser/navigation_entry.h"

bool WebContentsAdapter::canGoBack() const
{
    return webContents->GetController().CanGoBack();
}

bool WebContentsAdapter::canGoForward() const
{
    return webContents->GetController().CanGoForward();
}
bool WebContentsAdapter::isLoading() const
{
    return webContents->IsLoading();
}

void WebContentsAdapter::navigateHistory(int offset)
{
    webContents->GetController().GoToOffset(offset);
    webContents->GetView()->Focus();
}

void WebContentsAdapter::stop()
{
    content::NavigationController& controller = webContents->GetController();

    int index = controller.GetPendingEntryIndex();
    if (index != -1)
        controller.RemoveEntryAtIndex(index);

    webContents->GetView()->Focus();
}

void WebContentsAdapter::reload()
{
    webContents->GetController().Reload(/*checkRepost = */false);
    webContents->GetView()->Focus();
}

void WebContentsAdapter::load(const QUrl &url)
{
    QString urlString = url.toString();
    GURL gurl(urlString.toStdString());
    if (!gurl.has_scheme())
        gurl = GURL(std::string("http://") + urlString.toStdString());

    content::NavigationController::LoadURLParams params(gurl);
    params.transition_type = content::PageTransitionFromInt(content::PAGE_TRANSITION_TYPED | content::PAGE_TRANSITION_FROM_ADDRESS_BAR);
    webContents->GetController().LoadURLWithParams(params);
    webContents->GetView()->Focus();
}

QUrl WebContentsAdapter::activeUrl() const
{
    GURL gurl = webContents->GetVisibleURL();
    return QUrl(QString::fromStdString(gurl.spec()));
}

QString WebContentsAdapter::pageTitle() const
{
    content::NavigationEntry* entry = webContents->GetController().GetVisibleEntry();
    if (!entry)
        return QString();
    return QString::fromUtf16(entry->GetTitle().data());
}
