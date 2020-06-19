/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
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
    bool CanAddURL(const GURL &url) override;
    void NotifyProfileError(sql::InitStatus init_status, const std::string &diagnostics) override;
    std::unique_ptr<history::HistoryBackendClient> CreateBackendClient() override;
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
