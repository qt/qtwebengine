// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef FAVICON_SERVICE_FACTORY_QT_H
#define FAVICON_SERVICE_FACTORY_QT_H

#include "qtwebenginecoreglobal_p.h"

#include "base/memory/singleton.h"
#include "base/task/cancelable_task_tracker.h"
#include "components/favicon/core/favicon_client.h"
#include "components/history/core/browser/history_client.h"
#include "components/keyed_service/content/browser_context_keyed_service_factory.h"

class GURL;

namespace content {
class BrowserContext;
}

namespace favicon {
class FaviconService;
}

namespace history {
class HistoryBackendClient;
class HistoryService;
}

namespace QtWebEngineCore {

class HistoryClientQt : public history::HistoryClient
{
public:
    HistoryClientQt() { }
    virtual ~HistoryClientQt() { }

    void OnHistoryServiceCreated(history::HistoryService *history_service) override;
    void Shutdown() override;
    history::CanAddURLCallback GetThreadSafeCanAddURLCallback() const override;
    void NotifyProfileError(sql::InitStatus init_status, const std::string &diagnostics) override;
    std::unique_ptr<history::HistoryBackendClient> CreateBackendClient() override;
    void UpdateBookmarkLastUsedTime(int64_t bookmark_node_id, base::Time time) override;
};

class HistoryServiceFactoryQt : public BrowserContextKeyedServiceFactory
{
public:
    static history::HistoryService *GetForBrowserContext(content::BrowserContext *context);
    static HistoryServiceFactoryQt *GetInstance();

private:
    friend struct base::DefaultSingletonTraits<HistoryServiceFactoryQt>;

    HistoryServiceFactoryQt();
    ~HistoryServiceFactoryQt() override;

    // BrowserContextKeyedServiceFactory:
    content::BrowserContext *
    GetBrowserContextToUse(content::BrowserContext *context) const override;
    KeyedService *BuildServiceInstanceFor(content::BrowserContext *context) const override;
};

class FaviconClientQt : public favicon::FaviconClient
{
public:
    FaviconClientQt() { }
    virtual ~FaviconClientQt() { }

    bool IsNativeApplicationURL(const GURL &url) override;
    bool IsReaderModeURL(const GURL &url) override;
    const GURL GetOriginalUrlFromReaderModeUrl(const GURL &url) override;
    base::CancelableTaskTracker::TaskId
    GetFaviconForNativeApplicationURL(const GURL &url,
                                      const std::vector<int> &desired_sizes_in_pixel,
                                      favicon_base::FaviconResultsCallback callback,
                                      base::CancelableTaskTracker *tracker) override;
};

class FaviconServiceFactoryQt : public BrowserContextKeyedServiceFactory
{
public:
    static favicon::FaviconService *GetForBrowserContext(content::BrowserContext *context);
    static FaviconServiceFactoryQt *GetInstance();

private:
    friend struct base::DefaultSingletonTraits<FaviconServiceFactoryQt>;

    FaviconServiceFactoryQt();
    ~FaviconServiceFactoryQt() override;

    // BrowserContextKeyedServiceFactory:
    content::BrowserContext *
    GetBrowserContextToUse(content::BrowserContext *context) const override;
    KeyedService *BuildServiceInstanceFor(content::BrowserContext *context) const override;
};

} // namespace QtWebEngineCore

#endif // FAVICON_SERVICE_FACTORY_QT_H
