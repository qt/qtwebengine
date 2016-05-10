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

#ifndef BROWSER_CONTEXT_QT_H
#define BROWSER_CONTEXT_QT_H

#include "content/public/browser/browser_context.h"
#include "content/public/browser/content_browser_client.h"
#include "content/public/browser/resource_context.h"
#include "net/url_request/url_request_context.h"

#include <QtCore/qcompilerdetection.h> // Needed for Q_DECL_OVERRIDE

#if defined(ENABLE_SPELLCHECK)
QT_BEGIN_NAMESPACE
class QStringList;
QT_END_NAMESPACE
class TestingPrefStore;
class PrefService;
#endif

namespace QtWebEngineCore {

class BrowserContextAdapter;
class PermissionManagerQt;
class SSLHostStateDelegateQt;
class URLRequestContextGetterQt;

class BrowserContextQt : public content::BrowserContext
{
public:
    explicit BrowserContextQt(BrowserContextAdapter *);

    virtual ~BrowserContextQt();

    virtual base::FilePath GetPath() const Q_DECL_OVERRIDE;
    base::FilePath GetCachePath() const;
    virtual bool IsOffTheRecord() const Q_DECL_OVERRIDE;

    virtual net::URLRequestContextGetter *GetRequestContext() Q_DECL_OVERRIDE;
    virtual net::URLRequestContextGetter *GetRequestContextForRenderProcess(int) Q_DECL_OVERRIDE;
    virtual net::URLRequestContextGetter *GetMediaRequestContext() Q_DECL_OVERRIDE;
    virtual net::URLRequestContextGetter *GetMediaRequestContextForRenderProcess(int) Q_DECL_OVERRIDE;
    virtual net::URLRequestContextGetter *GetMediaRequestContextForStoragePartition(const base::FilePath&, bool) Q_DECL_OVERRIDE;
    virtual content::ResourceContext *GetResourceContext() Q_DECL_OVERRIDE;
    virtual content::DownloadManagerDelegate *GetDownloadManagerDelegate() Q_DECL_OVERRIDE;
    virtual content::BrowserPluginGuestManager* GetGuestManager() Q_DECL_OVERRIDE;
    virtual storage::SpecialStoragePolicy *GetSpecialStoragePolicy() Q_DECL_OVERRIDE;
    virtual content::PushMessagingService* GetPushMessagingService() Q_DECL_OVERRIDE;
    virtual content::SSLHostStateDelegate* GetSSLHostStateDelegate() Q_DECL_OVERRIDE;
    net::URLRequestContextGetter *CreateRequestContext(content::ProtocolHandlerMap *protocol_handlers, content::URLRequestInterceptorScopedVector request_interceptors);
    virtual scoped_ptr<content::ZoomLevelDelegate> CreateZoomLevelDelegate(const base::FilePath& partition_path) Q_DECL_OVERRIDE;
    virtual content::PermissionManager *GetPermissionManager() Q_DECL_OVERRIDE;
    virtual content::BackgroundSyncController* GetBackgroundSyncController() Q_DECL_OVERRIDE;

    BrowserContextAdapter *adapter() { return m_adapter; }

#if defined(ENABLE_SPELLCHECK)
    void failedToLoadDictionary(const std::string& language) override;
    void setSpellCheckLanguage(const QString &language);
    QString spellCheckLanguage() const;
    void setSpellCheckEnabled(bool enabled);
    bool isSpellCheckEnabled() const;
#endif

private:
    friend class ContentBrowserClientQt;
    friend class WebContentsAdapter;
    scoped_ptr<content::ResourceContext> resourceContext;
    scoped_refptr<URLRequestContextGetterQt> url_request_getter_;
    scoped_ptr<PermissionManagerQt> permissionManager;
    scoped_ptr<SSLHostStateDelegateQt> sslHostStateDelegate;
    BrowserContextAdapter *m_adapter;
#if defined(ENABLE_SPELLCHECK)
    scoped_refptr<TestingPrefStore> m_prefStore;
    scoped_ptr<PrefService> m_prefService;
#endif
    friend class BrowserContextAdapter;

    DISALLOW_COPY_AND_ASSIGN(BrowserContextQt);
};

} // namespace QtWebEngineCore

#endif // BROWSER_CONTEXT_QT_H
