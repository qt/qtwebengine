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

#include "web_engine_visited_links_manager.h"

#include "content_browser_client_qt.h"
#include "browser_context_qt.h"
#include "type_conversion.h"

#include "base/memory/scoped_ptr.h"
#include "components/visitedlink/browser/visitedlink_delegate.h"
#include "components/visitedlink/browser/visitedlink_master.h"

namespace {
class BasicVisitedLinkDelegate : public visitedlink::VisitedLinkDelegate {
public:
    virtual void RebuildTable(const scoped_refptr<URLEnumerator>& enumerator)
    {
        Q_FOREACH (const QUrl &url, WebEngineVisitedLinksManager::instance()->visitedUrlsFromHistoryBackend())
            enumerator->OnURL(toGurl(url));
        enumerator->OnComplete(true);
    }
};

class BasicUrlIterator : public visitedlink::VisitedLinkMaster::URLIterator {
public:
    BasicUrlIterator(const QList<QUrl> &urls) : m_urls(urls) {}
    virtual const GURL& NextURL() { m_currentUrl = toGurl(m_urls.takeFirst()); return m_currentUrl; }
    virtual bool HasNextURL() const { return !m_urls.isEmpty(); }
private:
    QList<QUrl> m_urls;
    GURL m_currentUrl;

};
} // Anonymous namespace

WebEngineVisitedLinksManager *WebEngineVisitedLinksManager::instance()
{
    static WebEngineVisitedLinksManager *manager = 0;
    if (!manager)
        manager = new WebEngineVisitedLinksManager;
    return manager;
}

void WebEngineVisitedLinksManager::setDelegate(WebEngineVisitedLinksDelegate *delegate)
{
    if (delegate == m_visitedLinksDelegate.data())
        return;
    m_visitedLinksDelegate.reset(delegate);
}

void WebEngineVisitedLinksManager::deleteAllVisitedLinkData()
{
    if (m_visitedLinkMaster.isNull()) {
        m_pendingDeleteAll = true;
        return;
    }
    m_visitedLinkMaster->DeleteAllURLs();
}

void WebEngineVisitedLinksManager::deleteVisitedLinkDataForUrls(const QList<QUrl> &urlsToDelete)
{
    if (m_visitedLinkMaster.isNull()) {
        m_pendingUrlsToDelete.append(urlsToDelete);
        return;
    }
    BasicUrlIterator iterator(urlsToDelete);
    m_visitedLinkMaster->DeleteURLs(&iterator);
}

QList<QUrl> WebEngineVisitedLinksManager::visitedUrlsFromHistoryBackend() const
{
    if (m_visitedLinksDelegate.isNull())
        return QList<QUrl>();
    return m_visitedLinksDelegate->visitedUrlsFromHistoryBackend();
}

WebEngineVisitedLinksManager::WebEngineVisitedLinksManager()
    : m_pendingDeleteAll(false)
{
}

WebEngineVisitedLinksManager::~WebEngineVisitedLinksManager()
{
}

void WebEngineVisitedLinksManager::ensureInitialized()
{
    Q_ASSERT(ContentBrowserClientQt::Get() && ContentBrowserClientQt::Get()->browser_context());
    if (!m_visitedLinkMaster.isNull())
        return;
    BrowserContextQt *browserContext = ContentBrowserClientQt::Get()->browser_context();
    m_visitedLinkMaster.reset(new visitedlink::VisitedLinkMaster(browserContext, new BasicVisitedLinkDelegate, /* persist to disk = */true));
    m_visitedLinkMaster->Init();

    if (m_pendingDeleteAll)
        deleteAllVisitedLinkData();
    else if (!m_pendingUrlsToDelete.isEmpty()) {
        deleteVisitedLinkDataForUrls(m_pendingUrlsToDelete);
        m_pendingUrlsToDelete.clear();
    }
}

void WebEngineVisitedLinksManager::addUrl(const GURL &urlToAdd)
{
    Q_ASSERT(!m_visitedLinkMaster.isNull());
    m_visitedLinkMaster->AddURL(urlToAdd);
    if (!m_visitedLinksDelegate.isNull())
        m_visitedLinksDelegate->addVisitedUrl(toQt(urlToAdd));
}
