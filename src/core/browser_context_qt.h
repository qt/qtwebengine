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

#ifndef BROWSER_CONTEXT_QT_H
#define BROWSER_CONTEXT_QT_H

#include "content/public/browser/browser_context.h"
#include "content/public/browser/content_browser_client.h"
#include "content/public/browser/resource_context.h"
#include "net/url_request/url_request_context.h"
#include "net/proxy/proxy_config_service.h"

#include "download_manager_delegate_qt.h"
#include "qtwebenginecoreglobal.h"
#include "resource_context_qt.h"
#include "url_request_context_getter_qt.h"

class BrowserContextQt : public content::BrowserContext
{
public:
    explicit BrowserContextQt(bool offTheRecord = false);
    virtual ~BrowserContextQt();

    virtual base::FilePath GetPath() const Q_DECL_OVERRIDE;
    virtual bool IsOffTheRecord() const Q_DECL_OVERRIDE;

    virtual net::URLRequestContextGetter *GetRequestContext() Q_DECL_OVERRIDE;
    virtual net::URLRequestContextGetter *GetRequestContextForRenderProcess(int) Q_DECL_OVERRIDE;
    virtual net::URLRequestContextGetter *GetMediaRequestContext() Q_DECL_OVERRIDE;
    virtual net::URLRequestContextGetter *GetMediaRequestContextForRenderProcess(int) Q_DECL_OVERRIDE;
    virtual net::URLRequestContextGetter *GetMediaRequestContextForStoragePartition(const base::FilePath&, bool) Q_DECL_OVERRIDE;
    virtual void RequestMIDISysExPermission(int render_process_id, int render_view_id, int bridge_id, const GURL &requesting_frame, const MIDISysExPermissionCallback&) Q_DECL_OVERRIDE;
    virtual void CancelMIDISysExPermissionRequest(int render_process_id, int render_view_id, int bridge_id, const GURL &requesting_frame) Q_DECL_OVERRIDE;
    virtual content::ResourceContext *GetResourceContext() Q_DECL_OVERRIDE;
    virtual content::DownloadManagerDelegate *GetDownloadManagerDelegate() Q_DECL_OVERRIDE;
    virtual content::GeolocationPermissionContext *GetGeolocationPermissionContext() Q_DECL_OVERRIDE;
    virtual quota::SpecialStoragePolicy *GetSpecialStoragePolicy() Q_DECL_OVERRIDE;
    net::URLRequestContextGetter *CreateRequestContext(content::ProtocolHandlerMap *protocol_handlers);

private:
    scoped_ptr<content::ResourceContext> resourceContext;
    scoped_refptr<net::URLRequestContextGetter> url_request_getter_;
    scoped_ptr<DownloadManagerDelegateQt> downloadManagerDelegate;
    bool m_offTheRecord;

    DISALLOW_COPY_AND_ASSIGN(BrowserContextQt);
};

#endif // BROWSER_CONTEXT_QT_H
