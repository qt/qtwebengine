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

#ifndef PROTOCOL_HANDLER_REGISTRY_QT_H_
#define PROTOCOL_HANDLER_REGISTRY_QT_H_

#include <map>
#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/sequenced_task_runner_helpers.h"
#include "base/values.h"

#include "components/browser_context_keyed_service/browser_context_keyed_service.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/notification_service.h"
#include "net/url_request/url_request.h"
#include "net/url_request/url_request_job.h"
#include "net/url_request/url_request_job_factory.h"

#include "custom_protocol_handler_qt.h"

namespace content {
class BrowserContext;
}

class ProtocolHandlerRegistryQt : public BrowserContextKeyedService {

public:

    // Forward declaration of the internal implementation class.
    class IOThreadDelegate;

    class JobInterceptorFactory : public net::URLRequestJobFactory {

    public:
        // |m_ioThreadDelegate| is used to perform actual job creation work.
        explicit JobInterceptorFactory(IOThreadDelegate* ioThreadDelegate);
        virtual ~JobInterceptorFactory();
        // |m_jobFactory| is set as the URLRequestJobFactory where requests are
        // forwarded if JobInterceptorFactory decides to pass on them.
        void chain(scoped_ptr<net::URLRequestJobFactory> jobFactory);

        // URLRequestJobFactory implementation.
        virtual net::URLRequestJob *MaybeCreateJobWithProtocolHandler(
            const std::string &scheme,
            net::URLRequest *request,
            net::NetworkDelegate *networkDelegate) const OVERRIDE;
        virtual bool IsHandledProtocol(const std::string &scheme) const OVERRIDE;
        virtual bool IsHandledURL(const GURL &url) const OVERRIDE;
        virtual bool IsSafeRedirectTarget(const GURL &location) const OVERRIDE;
    private:
        // When JobInterceptorFactory decides to pass on particular requests,
        // they're forwarded to the chained URLRequestJobFactory, |m_jobFactory|.
        scoped_ptr<net::URLRequestJobFactory> m_jobFactory;
        // |m_ioThreadDelegate| performs the actual job creation decisions by
        // mirroring the ProtocolHandlerRegistry on the IO thread.
        scoped_refptr<IOThreadDelegate> m_ioThreadDelegate;
        DISALLOW_COPY_AND_ASSIGN(JobInterceptorFactory);
    };

    typedef std::map<std::string, CustomProtocolHandlerQt> CustomProtocolHandlerMap;
    typedef std::vector<CustomProtocolHandlerQt> CustomProtocolHandlerList;
    typedef std::map<std::string, CustomProtocolHandlerList> CustomProtocolHandlerMultiMap;

    ProtocolHandlerRegistryQt(content::BrowserContext *context);
    virtual ~ProtocolHandlerRegistryQt();

    // Returns a net::URLRequestJobFactory suitable for use on the IO thread.
    scoped_ptr<JobInterceptorFactory> createJobInterceptorFactory();

    // Called when the user accepts the registration of a given protocol handler.
    void onAcceptRegisterProtocolHandler(const CustomProtocolHandlerQt &handler);

    // Called when the user denies the registration of a given protocol handler.
    void onDenyRegisterProtocolHandler(const CustomProtocolHandlerQt &handler);

    // Called when the user indicates that they don't want to be asked about the
    // given protocol handler again.
    void onIgnoreRegisterProtocolHandler(const CustomProtocolHandlerQt &handler);

    // Returns true if we allow websites to register handlers for the given scheme.
    bool canSchemeBeOverridden(const std::string &scheme) const;

    // Returns true if an identical protocol handler has already been registered.
    bool isRegistered(const CustomProtocolHandlerQt &handler) const;

    // Returns true if an identical protocol handler is being ignored.
    bool isIgnored(const CustomProtocolHandlerQt &handler) const;

    // Removes the given protocol handler from the registry.
    void removeHandler(const CustomProtocolHandlerQt &handler);

    // This is called by the UI thread when the system is shutting down. This
    // does finalization which must be done on the UI thread.
    virtual void Shutdown() OVERRIDE;

private:
    // Makes this ProtocolHandler the default handler for its protocol.
    void setDefault(const CustomProtocolHandlerQt &handler);

    // Insert the given CustomProtocolHandlerQt into the registry.
    void insertHandler(const CustomProtocolHandlerQt &handler);

    // Puts the given handler at the top of the list of handlers for its protocol.
    void promoteHandler(const CustomProtocolHandlerQt &handler);

    // Registers a new custom handler.
    void registerProtocolHandler(const CustomProtocolHandlerQt &handler);

    // Ignores future requests to register the given protocol handler.
    void ignoreProtocolHandler(const CustomProtocolHandlerQt &handler);

    // Returns a pointer to the list of handlers registered for the given scheme,
    // or Q_NULLPTR if there are none.
    const CustomProtocolHandlerList *getHandlerList(const std::string &scheme) const;

    content::BrowserContext *m_browserContext;

    // Map from protocols (strings) to custom protocol handlers.
    CustomProtocolHandlerMultiMap m_protocolHandlers;

    // Custom protocol handlers that the user has told us to ignore.
    CustomProtocolHandlerList m_ignoredProtocolHandlers;

    // Custom protocol handlers that are the defaults for a given protocol.
    CustomProtocolHandlerMap m_defaultHandlers;

    // Copy of registry data for use on the IO thread. Changes to the registry
    // are posted to the IO thread where updates are applied to this object.
    scoped_refptr<IOThreadDelegate> m_ioThreadDelegate;

    DISALLOW_COPY_AND_ASSIGN(ProtocolHandlerRegistryQt);
};

#endif // PROTOCOL_HANDLER_REGISTRY_QT_H_
