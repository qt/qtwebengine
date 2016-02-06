/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "favicon_manager.h"
#include "favicon_manager_p.h"

#include "type_conversion.h"
#include "web_contents_adapter_client.h"

#include "base/bind.h"
#include "content/public/browser/web_contents.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "third_party/skia/include/core/SkPixelRef.h"
#include "ui/gfx/geometry/size.h"

#include <QtCore/QUrl>
#include <QtGui/QIcon>

namespace QtWebEngineCore {

static inline bool isResourceUrl(const QUrl &url)
{
    return !url.scheme().compare(QLatin1String("qrc"));
}

static inline unsigned area(const QSize &size)
{
    return size.width() * size.height();
}


FaviconManagerPrivate::FaviconManagerPrivate(content::WebContents *webContents, WebContentsAdapterClient *viewClient)
    : m_webContents(webContents)
    , m_viewClient(viewClient)
    , m_weakFactory(this)
{
}

FaviconManagerPrivate::~FaviconManagerPrivate()
{
}

int FaviconManagerPrivate::downloadIcon(const QUrl &url, bool candidate)
{
    static int fakeId = 0;
    int id;

    bool cached = candidate && m_icons.contains(url);
    if (isResourceUrl(url) || cached) {
        id = --fakeId;
        m_pendingRequests.insert(id, url);
    } else {
        id = m_webContents->DownloadImage(
             toGurl(url),
             true, // is_favicon
             0,    // no max size
             false, // normal cache policy
             base::Bind(&FaviconManagerPrivate::iconDownloadFinished, m_weakFactory.GetWeakPtr()));
    }

    if (candidate) {
        Q_ASSERT(!m_inProgressCandidateRequests.contains(id));
        m_inProgressCandidateRequests.insert(id, url);
    } else {
        Q_ASSERT(!m_inProgressCustomRequests.contains(id));
        m_inProgressCustomRequests.insert(id, url);
    }

    return id;
}

void FaviconManagerPrivate::iconDownloadFinished(int id,
                                                 int status,
                                                 const GURL &url,
                                                 const std::vector<SkBitmap> &bitmaps,
                                                 const std::vector<gfx::Size> &original_bitmap_sizes)
{
    Q_UNUSED(status);
    Q_UNUSED(url);
    Q_UNUSED(original_bitmap_sizes);

    storeIcon(id, toQIcon(bitmaps));
}

/* Pending requests are used as a workaround for avoiding signal iconChanged when
 * accessing each cached icons or icons stored in qrc. They don't have to be downloaded
 * thus the m_inProgressCustomRequests maybe emptied right before the next icon is added to
 * in-progress-requests queue. The m_pendingRequests stores these requests until all
 * candidates are added to the queue then pending requests should be cleaned up by this
 * function.
 */
void FaviconManagerPrivate::downloadPendingRequests()
{
    for (auto it = m_pendingRequests.cbegin(), end = m_pendingRequests.cend(); it != end; ++it) {
        QIcon icon;

        QUrl requestUrl = it.value();
        if (isResourceUrl(requestUrl) && !m_icons.contains(requestUrl))
            icon = QIcon(requestUrl.toString().remove(0, 3));

        storeIcon(it.key(), icon);
    }

    m_pendingRequests.clear();
}

void FaviconManagerPrivate::storeIcon(int id, const QIcon &icon)
{
    Q_Q(FaviconManager);

    bool candidate = m_inProgressCandidateRequests.contains(id);

    QUrl requestUrl = candidate ? m_inProgressCandidateRequests[id] : m_inProgressCustomRequests[id];
    FaviconInfo &faviconInfo = q->m_faviconInfoMap[requestUrl];

    unsigned iconCount = 0;
    if (!icon.isNull())
        iconCount = icon.availableSizes().count();

    if (iconCount > 0) {
        m_icons.insert(requestUrl, icon);

        faviconInfo.size = icon.availableSizes().at(0);
        if (iconCount > 1) {
            faviconInfo.multiSize = true;
            unsigned bestArea = area(faviconInfo.size);
            for (unsigned i = 1; i < iconCount; ++i) {
                QSize iconSize = icon.availableSizes().at(i);
                if (bestArea < area(iconSize)) {
                    faviconInfo.size = iconSize;
                    bestArea = area(iconSize);
                }
            }
        }

        q->m_hasDownloadedIcon = true;
    } else if (id < 0) {
        // Icon is cached
        q->m_hasDownloadedIcon = true;
    } else {
        // Reset size if icon cannot be downloaded
        faviconInfo.size = QSize(0, 0);
    }

    if (candidate) {
        m_inProgressCandidateRequests.remove(id);
        if (m_inProgressCandidateRequests.isEmpty())
            m_viewClient->iconChanged(q->getProposedFaviconInfo().url);
    } else {
        m_inProgressCustomRequests.remove(id);
    }

    Q_EMIT q->iconDownloaded(requestUrl);
}

FaviconManager::FaviconManager(FaviconManagerPrivate *d)
    : m_hasDownloadedIcon(false)
{
    Q_ASSERT(d);
    d_ptr.reset(d);

    d->q_ptr = this;
}

FaviconManager::~FaviconManager()
{
}

QIcon FaviconManager::getIcon(const QUrl &url) const
{
    Q_D(const FaviconManager);
    return d->m_icons[url];
}

FaviconInfo FaviconManager::getFaviconInfo(const QUrl &url) const
{
    return m_faviconInfoMap[url];
}

QList<FaviconInfo> FaviconManager::getFaviconInfoList(bool candidatesOnly) const
{
    QList<FaviconInfo> faviconInfoList = m_faviconInfoMap.values();

    if (candidatesOnly) {
        QMutableListIterator<FaviconInfo> it(faviconInfoList);
        while (it.hasNext()) {
            if (!it.next().candidate)
                it.remove();
        }
    }

    return faviconInfoList;
}


void FaviconManager::downloadIcon(const QUrl &url, FaviconInfo::FaviconType iconType)
{
    Q_D(FaviconManager);

    // If the favicon cannot be found in the list that means that it is not a candidate
    // for any visited page (including the current one). In this case the type of the icon
    // is unknown: it has to be specified explicitly.
    if (!m_faviconInfoMap.contains(url)) {
        FaviconInfo newFaviconInfo(url, iconType);
        m_faviconInfoMap.insert(url, newFaviconInfo);
    }

    d->downloadIcon(url, false);
    d->downloadPendingRequests();
}

void FaviconManager::removeIcon(const QUrl &url)
{
    Q_D(FaviconManager);
    int removed = d->m_icons.remove(url);

    if (removed) {
        Q_ASSERT(removed == 1);
        Q_ASSERT(m_faviconInfoMap.contains(url));
        m_faviconInfoMap[url].size = QSize(0, 0);
    }
}

bool FaviconManager::hasAvailableCandidateIcon() const
{
    Q_D(const FaviconManager);
    return m_hasDownloadedIcon || !d->m_inProgressCandidateRequests.isEmpty();
}

void FaviconManager::update(const QList<FaviconInfo> &candidates)
{
    Q_D(FaviconManager);
    updateCandidates(candidates);

    for (auto it = m_faviconInfoMap.cbegin(), end = m_faviconInfoMap.cend(); it != end; ++it) {
        const FaviconInfo &faviconInfo = it.value();
        if (!faviconInfo.candidate || faviconInfo.type != FaviconInfo::Favicon)
            continue;

        if (faviconInfo.isValid())
            d->downloadIcon(faviconInfo.url, true);
    }

    d->downloadPendingRequests();
}

void FaviconManager::updateCandidates(const QList<FaviconInfo> &candidates)
{
    for (FaviconInfo candidateFaviconInfo : candidates) {
        QUrl candidateUrl = candidateFaviconInfo.url;
        if (m_faviconInfoMap.contains(candidateUrl)) {
            m_faviconInfoMap[candidateUrl].candidate = true;
            // Update type in case of the icon was downloaded manually
            m_faviconInfoMap[candidateUrl].type = candidateFaviconInfo.type;
            continue;
        }

        candidateFaviconInfo.candidate = true;
        m_faviconInfoMap.insert(candidateUrl, candidateFaviconInfo);
    }
}

void FaviconManager::resetCandidates()
{
    m_hasDownloadedIcon = false;
    for (auto it = m_faviconInfoMap.begin(), end = m_faviconInfoMap.end(); it != end; ++it)
        it->candidate = false;
}


FaviconInfo FaviconManager::getProposedFaviconInfo() const
{
    FaviconInfo proposedFaviconInfo = getFirstFaviconInfo();

    // If nothing has been downloaded yet return the first favicon
    // if there is available for dev-tools
    if (!m_hasDownloadedIcon)
        return proposedFaviconInfo;

    unsigned bestArea = area(proposedFaviconInfo.size);
    for (auto it = m_faviconInfoMap.cbegin(), end = m_faviconInfoMap.cend(); it != end; ++it) {
        const FaviconInfo &faviconInfo = it.value();
        if (!faviconInfo.candidate || faviconInfo.type != FaviconInfo::Favicon)
            continue;

        if (faviconInfo.isValid() && bestArea < area(faviconInfo.size)) {
            proposedFaviconInfo = faviconInfo;
            bestArea = area(proposedFaviconInfo.size);
        }
    }

    return proposedFaviconInfo;
}

FaviconInfo FaviconManager::getFirstFaviconInfo() const
{
    for (auto it = m_faviconInfoMap.cbegin(), end = m_faviconInfoMap.cend(); it != end; ++it) {
        const FaviconInfo &faviconInfo = it.value();
        if (!faviconInfo.candidate || faviconInfo.type != FaviconInfo::Favicon)
            continue;

        if (faviconInfo.isValid())
            return faviconInfo;
    }

    return FaviconInfo();
}



FaviconInfo::FaviconInfo()
    : url(QUrl())
    , type(FaviconInfo::InvalidIcon)
    , size(QSize(0, 0))
    , candidate(false)
    , multiSize(false)
{
}

FaviconInfo::FaviconInfo(const FaviconInfo &other)
    : url(other.url)
    , type(other.type)
    , size(other.size)
    , candidate(other.candidate)
{
}

FaviconInfo::FaviconInfo(const QUrl &url, FaviconInfo::FaviconType type)
    : url(url)
    , type(type)
    , size(QSize(0, 0))
    , candidate(false)
    , multiSize(false)
{
}

FaviconInfo::~FaviconInfo()
{
}

bool FaviconInfo::isValid() const
{
    if (type == FaviconInfo::InvalidIcon)
        return false;

    if (url.isEmpty() || !url.isValid())
        return false;

    return true;
}

bool FaviconInfo::isDownloaded() const
{
    return area(size) > 0;
}

} // namespace QtWebEngineCore
