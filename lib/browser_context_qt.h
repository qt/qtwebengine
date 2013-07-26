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

#ifndef BROWSER_CONTEXT_QT
#define BROWSER_CONTEXT_QT

#include "content/public/browser/browser_context.h"

#include "base/files/scoped_temp_dir.h"

#include "shared/shared_globals.h"

#include "base/time/time.h"
#include "content/public/browser/content_browser_client.h"
#include "content/public/browser/resource_context.h"
#include "content/public/browser/storage_partition.h"
#include "net/url_request/url_request_context.h"
#include "net/proxy/proxy_config_service.h"

#include <qglobal.h>
#include <QByteArray>
#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>
#include <QString>
#include <QStringBuilder>

#include "resource_context_qt.h"
#include "url_request_context_getter_qt.h"

class BrowserContextQt : public content::BrowserContext
{
public:
   explicit BrowserContextQt()
   {
       resourceContext.reset(new ResourceContextQt(this));
   }

   virtual ~BrowserContextQt() Q_DECL_OVERRIDE {}

   virtual base::FilePath GetPath() const Q_DECL_OVERRIDE
   {
       QString dataLocation = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
       if (dataLocation.isEmpty())
          dataLocation = QDir::homePath() % QDir::separator() % QChar::fromLatin1('.') % QCoreApplication::applicationName();

       dataLocation.append(QDir::separator() % QStringLiteral("QtWebEngine"));
       return base::FilePath(qStringToStringType(dataLocation));
   }

   virtual bool IsOffTheRecord() const Q_DECL_OVERRIDE
   {
       return false;
   }

   virtual net::URLRequestContextGetter* GetRequestContext() Q_DECL_OVERRIDE
   {
       return GetDefaultStoragePartition(this)->GetURLRequestContext();
   }
   virtual net::URLRequestContextGetter* GetRequestContextForRenderProcess(int) Q_DECL_OVERRIDE { return GetRequestContext(); }
   virtual net::URLRequestContextGetter* GetMediaRequestContext() Q_DECL_OVERRIDE { return GetRequestContext(); }
   virtual net::URLRequestContextGetter* GetMediaRequestContextForRenderProcess(int) Q_DECL_OVERRIDE { return GetRequestContext(); }
   virtual net::URLRequestContextGetter* GetMediaRequestContextForStoragePartition(const base::FilePath&, bool) Q_DECL_OVERRIDE { return GetRequestContext(); }

   virtual void RequestMIDISysExPermission(int render_process_id, int render_view_id, const GURL& requesting_frame, const MIDISysExPermissionCallback& callback) Q_DECL_OVERRIDE
   {
       // Always reject requests for testing.
       callback.Run(false);
   }

   virtual content::ResourceContext* GetResourceContext() Q_DECL_OVERRIDE
   {
       return resourceContext.get();
   }

   virtual content::DownloadManagerDelegate* GetDownloadManagerDelegate() Q_DECL_OVERRIDE
   {
       QT_NOT_YET_IMPLEMENTED
       return 0;
   }
   virtual content::GeolocationPermissionContext* GetGeolocationPermissionContext() Q_DECL_OVERRIDE
   {
       QT_NOT_YET_IMPLEMENTED
       return 0;
   }
   virtual quota::SpecialStoragePolicy* GetSpecialStoragePolicy() Q_DECL_OVERRIDE
   {
       QT_NOT_YET_IMPLEMENTED
       return 0;
   }

   net::URLRequestContextGetter *CreateRequestContext(content::ProtocolHandlerMap* protocol_handlers) Q_DECL_OVERRIDE
   {
       url_request_getter_ = new URLRequestContextGetterQt(GetPath());
       static_cast<ResourceContextQt*>(resourceContext.get())->set_url_request_context_getter(url_request_getter_.get());
       return url_request_getter_.get();
   }

private:
   scoped_ptr<content::ResourceContext> resourceContext;
   scoped_refptr<net::URLRequestContextGetter> url_request_getter_;

   DISALLOW_COPY_AND_ASSIGN(BrowserContextQt);
};

#endif //BROWSER_CONTEXT_QT
