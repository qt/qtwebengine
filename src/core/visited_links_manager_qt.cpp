// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "visited_links_manager_qt.h"

#include "content_browser_client_qt.h"
#include "profile_adapter.h"
#include "profile_qt.h"
#include "type_conversion.h"

#include <base/files/file_util.h>
#include "components/visitedlink/browser/visitedlink_delegate.h"
#include "components/visitedlink/browser/visitedlink_writer.h"

namespace QtWebEngineCore {

namespace {
class BasicUrlIterator : public visitedlink::VisitedLinkWriter::URLIterator {
public:
    BasicUrlIterator(const QList<QUrl> &urls) : m_urls(urls) {}
    const GURL &NextURL() override { m_currentUrl = toGurl(m_urls.takeFirst()); return m_currentUrl; }
    bool HasNextURL() const override { return !m_urls.isEmpty(); }
private:
    QList<QUrl> m_urls;
    GURL m_currentUrl;

};
} // Anonymous namespace

// Due to the design of the visitedLink component, it seems safer to provide a
// basic delegate that will simply rebuild an empty visitedLink table if needed
// from the application history rather than crashing. This is functionality
// that was covered by QWebHistoryInterface in QtWebKitWidgets.

class VisitedLinkDelegateQt : public visitedlink::VisitedLinkDelegate
{
public:
    ~VisitedLinkDelegateQt() {}
    void RebuildTable(const scoped_refptr<URLEnumerator> &enumerator) override { enumerator->OnComplete(true); }
};

void VisitedLinksManagerQt::deleteAllVisitedLinkData()
{
    m_visitedLinkWriter->DeleteAllURLs();
}

void VisitedLinksManagerQt::deleteVisitedLinkDataForUrls(const QList<QUrl> &urlsToDelete)
{
    BasicUrlIterator iterator(urlsToDelete);
    m_visitedLinkWriter->DeleteURLs(&iterator);
}

bool VisitedLinksManagerQt::containsUrl(const QUrl &url) const
{
    return m_visitedLinkWriter->IsVisited(toGurl(url));
}

VisitedLinksManagerQt::VisitedLinksManagerQt(ProfileQt *profile, bool persistVisitedLinks)
    : m_delegate(new VisitedLinkDelegateQt)
{
    Q_ASSERT(profile);
    if (persistVisitedLinks)
        profile->profileAdapter()->ensureDataPathExists();
    m_visitedLinkWriter.reset(new visitedlink::VisitedLinkWriter(profile, m_delegate.data(), persistVisitedLinks));
    m_visitedLinkWriter->Init();
}

VisitedLinksManagerQt::~VisitedLinksManagerQt()
{
}

void VisitedLinksManagerQt::addUrl(const GURL &urlToAdd)
{
    Q_ASSERT(m_visitedLinkWriter);
    m_visitedLinkWriter->AddURL(urlToAdd);
}

} // namespace QtWebEngineCore
