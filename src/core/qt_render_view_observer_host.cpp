/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
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

#include "qt_render_view_observer_host.h"

#include "common/qt_messages.h"
#include "type_conversion.h"
#include "web_contents_adapter_client.h"

QtRenderViewObserverHost::QtRenderViewObserverHost(content::WebContents *webContents, WebContentsAdapterClient *adapterClient)
    : content::WebContentsObserver(webContents)
    , m_adapterClient(adapterClient)
{
}

void QtRenderViewObserverHost::fetchDocumentMarkup(quint64 requestId)
{
    Send(new QtRenderViewObserver_FetchDocumentMarkup(routing_id(), requestId));
}

void QtRenderViewObserverHost::fetchDocumentInnerText(quint64 requestId)
{
    Send(new QtRenderViewObserver_FetchDocumentInnerText(routing_id(), requestId));
}

bool QtRenderViewObserverHost::OnMessageReceived(const IPC::Message& message)
{
    bool handled = true;
    IPC_BEGIN_MESSAGE_MAP(QtRenderViewObserverHost, message)
        IPC_MESSAGE_HANDLER(QtRenderViewObserverHost_DidFetchDocumentMarkup,
                            onDidFetchDocumentMarkup)
        IPC_MESSAGE_HANDLER(QtRenderViewObserverHost_DidFetchDocumentInnerText,
                            onDidFetchDocumentInnerText)
        IPC_MESSAGE_HANDLER(QtRenderViewObserverHost_DidFirstVisuallyNonEmptyLayout,
                            onDidFirstVisuallyNonEmptyLayout)
        IPC_MESSAGE_UNHANDLED(handled = false)
    IPC_END_MESSAGE_MAP()
    return handled;

}

void QtRenderViewObserverHost::onDidFetchDocumentMarkup(quint64 requestId, const base::string16& markup)
{
    m_adapterClient->didFetchDocumentMarkup(requestId, toQt(markup));
}

void QtRenderViewObserverHost::onDidFetchDocumentInnerText(quint64 requestId, const base::string16& innerText)
{
    m_adapterClient->didFetchDocumentInnerText(requestId, toQt(innerText));
}

void QtRenderViewObserverHost::onDidFirstVisuallyNonEmptyLayout()
{
    m_adapterClient->didFirstVisuallyNonEmptyLayout();
}
