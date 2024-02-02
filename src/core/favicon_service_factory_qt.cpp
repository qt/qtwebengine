// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "favicon_service_factory_qt.h"

#include "base/files/file_util.h"
#include "components/favicon/core/favicon_service_impl.h"
#include "components/history/content/browser/content_visit_delegate.h"
#include "components/history/content/browser/history_database_helper.h"
#include "components/history/core/browser/history_backend_client.h"
#include "components/history/core/browser/history_database_params.h"
#include "components/history/core/browser/history_service.h"
#include "components/keyed_service/content/browser_context_dependency_manager.h"
#include "content/public/browser/browser_context.h"
#include "url/gurl.h"

namespace QtWebEngineCore {

void HistoryClientQt::OnHistoryServiceCreated(history::HistoryService *history_service)
{
    Q_UNUSED(history_service);
}

void HistoryClientQt::Shutdown() { }

static bool CanAddURL(const GURL &url)
{
    Q_UNUSED(url);
    return true;
}

history::CanAddURLCallback HistoryClientQt::GetThreadSafeCanAddURLCallback() const
{
    return base::BindRepeating(&CanAddURL);
}

void HistoryClientQt::NotifyProfileError(sql::InitStatus init_status,
                                         const std::string &diagnostics)
{
    Q_UNUSED(init_status);
    Q_UNUSED(diagnostics);
}

std::unique_ptr<history::HistoryBackendClient> HistoryClientQt::CreateBackendClient()
{
    return nullptr;
}

void HistoryClientQt::UpdateBookmarkLastUsedTime(int64_t /*bookmark_node_id*/, base::Time /*time*/)
{
}

// static
history::HistoryService *
HistoryServiceFactoryQt::GetForBrowserContext(content::BrowserContext *context)
{
    if (context->IsOffTheRecord())
        return nullptr;


    return static_cast<history::HistoryService *>(
            GetInstance()->GetServiceForBrowserContext(context, true));
}

// static
HistoryServiceFactoryQt *HistoryServiceFactoryQt::GetInstance()
{
    return base::Singleton<HistoryServiceFactoryQt>::get();
}

HistoryServiceFactoryQt::HistoryServiceFactoryQt()
    : BrowserContextKeyedServiceFactory("HistoryService",
                                        BrowserContextDependencyManager::GetInstance())
{
}

HistoryServiceFactoryQt::~HistoryServiceFactoryQt() { }

content::BrowserContext *
HistoryServiceFactoryQt::GetBrowserContextToUse(content::BrowserContext *context) const
{
    return context;
}

KeyedService *
HistoryServiceFactoryQt::BuildServiceInstanceFor(content::BrowserContext *context) const
{
    Q_ASSERT(!context->IsOffTheRecord());

    std::unique_ptr<history::HistoryService> historyService(
            new history::HistoryService(std::make_unique<HistoryClientQt>(), nullptr));
    if (!historyService->Init(history::HistoryDatabaseParamsForPath(context->GetPath(), version_info::Channel::DEFAULT))) {
        return nullptr;
    }
    return historyService.release();
}

bool FaviconClientQt::IsNativeApplicationURL(const GURL &url)
{
    Q_UNUSED(url);
    return false;
}

bool FaviconClientQt::IsReaderModeURL(const GURL &url)
{
    Q_UNUSED(url)
    return false;
}

const GURL FaviconClientQt::GetOriginalUrlFromReaderModeUrl(const GURL &url)
{
    return url;
}

base::CancelableTaskTracker::TaskId FaviconClientQt::GetFaviconForNativeApplicationURL(
        const GURL &url, const std::vector<int> &desired_sizes_in_pixel,
        favicon_base::FaviconResultsCallback callback, base::CancelableTaskTracker *tracker)
{
    Q_UNUSED(url);
    Q_UNUSED(desired_sizes_in_pixel);
    Q_UNUSED(callback);
    Q_UNUSED(tracker);

    return base::CancelableTaskTracker::kBadTaskId;
}

// static
favicon::FaviconService *
FaviconServiceFactoryQt::GetForBrowserContext(content::BrowserContext *context)
{
    return static_cast<favicon::FaviconService *>(
            GetInstance()->GetServiceForBrowserContext(context, true));
}

// static
FaviconServiceFactoryQt *FaviconServiceFactoryQt::GetInstance()
{
    return base::Singleton<FaviconServiceFactoryQt>::get();
}

FaviconServiceFactoryQt::FaviconServiceFactoryQt()
    : BrowserContextKeyedServiceFactory("FaviconService",
                                        BrowserContextDependencyManager::GetInstance())
{
}

FaviconServiceFactoryQt::~FaviconServiceFactoryQt() { }

content::BrowserContext *
FaviconServiceFactoryQt::GetBrowserContextToUse(content::BrowserContext *context) const
{
    return context;
}

KeyedService *
FaviconServiceFactoryQt::BuildServiceInstanceFor(content::BrowserContext *context) const
{
    history::HistoryService *historyService = static_cast<history::HistoryService *>(
            HistoryServiceFactoryQt::GetInstance()->GetForBrowserContext(context));
    return new favicon::FaviconServiceImpl(std::make_unique<FaviconClientQt>(), historyService);
}

} // namespace QtWebEngineCore
