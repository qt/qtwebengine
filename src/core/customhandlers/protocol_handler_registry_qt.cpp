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

#include "protocol_handler_registry_qt.h"
#include "content/public/browser/browser_context.h"
#include "net/url_request/url_request_redirect_job.h"

using content::BrowserThread;

namespace {

const CustomProtocolHandlerQt &lookupHandler(
        const ProtocolHandlerRegistryQt::CustomProtocolHandlerMap &handlerMap,
        const std::string &scheme)
{
    ProtocolHandlerRegistryQt::CustomProtocolHandlerMap::const_iterator p = handlerMap.find(scheme);

    if (p != handlerMap.end())
        return p->second;

    return CustomProtocolHandlerQt::EmptyProtocolHandler();
}

}  // namespace

// IOThreadDelegate ------------------------------------------------------------

// IOThreadDelegate is an IO thread specific object. Access to the class should
// all be done via the IO thread. The registry living on the UI thread makes
// a best effort to update the IO object after local updates are completed.
class ProtocolHandlerRegistryQt::IOThreadDelegate: public base::RefCountedThreadSafe<
        ProtocolHandlerRegistryQt::IOThreadDelegate> {

public:
    // Creates a new instance. If |enabled| is true the registry is considered
    // enabled on the IO thread.
    explicit IOThreadDelegate(bool enabled);

    // Returns true if the protocol has a default protocol handler.
    // Should be called only from the IO thread.
    bool isHandledProtocol(const std::string &scheme) const;

    // Clears the default for the provided protocol.
    // Should be called only from the IO thread.
    void clearDefault(const std::string &scheme);

    // Makes this CustomProtocolHandlerQt the default handler for its protocol.
    // Should be called only from the IO thread.
    void setDefault(const CustomProtocolHandlerQt &handler);

    // Creates a URL request job for the given request if there is a matching
    // protocol handler, returns Q_NULLPTR otherwise.
    net::URLRequestJob *maybeCreateJob(net::URLRequest *request,
            net::NetworkDelegate *networkDelegate) const;

    // Indicate that the registry has been enabled in the IO thread's
    // copy of the data.
    void enable()
    {
        m_enabled = true;
    }

    // Indicate that the registry has been disabled in the IO thread's copy of
    // the data.
    void disable()
    {
        m_enabled = false;
    }

private:
    friend class base::RefCountedThreadSafe<IOThreadDelegate>;
    virtual ~IOThreadDelegate();

    // Copy of protocol handlers use only on the IO thread.
    ProtocolHandlerRegistryQt::CustomProtocolHandlerMap m_defaultHandlers;

    // Is the registry enabled on the IO thread.
    bool m_enabled;

    DISALLOW_COPY_AND_ASSIGN(IOThreadDelegate);
};

ProtocolHandlerRegistryQt::IOThreadDelegate::IOThreadDelegate(bool)
    : m_enabled(true)
{}

ProtocolHandlerRegistryQt::IOThreadDelegate::~IOThreadDelegate()
{}

bool ProtocolHandlerRegistryQt::IOThreadDelegate::isHandledProtocol(const std::string &scheme) const
{
    DCHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));
    return m_enabled && !lookupHandler(m_defaultHandlers, scheme).IsEmpty();
}

void ProtocolHandlerRegistryQt::IOThreadDelegate::clearDefault(const std::string &scheme)
{
    DCHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));
    m_defaultHandlers.erase(scheme);
}

void ProtocolHandlerRegistryQt::IOThreadDelegate::setDefault(const CustomProtocolHandlerQt &handler)
{
    DCHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));
    clearDefault(handler.protocol());
    m_defaultHandlers.insert(std::make_pair(handler.protocol(), handler));
}

// Create a new job for the supplied |URLRequest| if a default handler
// is registered and the associated handler is able to interpret
// the url from |request|.
net::URLRequestJob *ProtocolHandlerRegistryQt::IOThreadDelegate::maybeCreateJob(
        net::URLRequest *request, net::NetworkDelegate *networkDelegate) const
{
    DCHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));

    CustomProtocolHandlerQt handler = lookupHandler(m_defaultHandlers, request->url().scheme());
    if (handler.IsEmpty())
        return NULL;

    GURL translatedUrl(handler.TranslateUrl(request->url()));
    if (!translatedUrl.is_valid())
        return NULL;

    return new net::URLRequestRedirectJob(request, networkDelegate, translatedUrl,
            net::URLRequestRedirectJob::REDIRECT_307_TEMPORARY_REDIRECT);
}

// JobInterceptorFactory -------------------------------------------------------

// Instances of JobInterceptorFactory are produced for ownership by the IO
// thread where it handler URL requests. We should never hold
// any pointers on this class, only produce them in response to
// requests via |ProtocolHandlerRegistry::CreateJobInterceptorFactory|.
ProtocolHandlerRegistryQt::JobInterceptorFactory::JobInterceptorFactory(
        IOThreadDelegate *ioThreadDelegate)
        : m_ioThreadDelegate(ioThreadDelegate)
{
    DCHECK(m_ioThreadDelegate.get());
    DetachFromThread();
}

ProtocolHandlerRegistryQt::JobInterceptorFactory::~JobInterceptorFactory()
{
}

void ProtocolHandlerRegistryQt::JobInterceptorFactory::chain(scoped_ptr<net::URLRequestJobFactory> jobFactory)
{
    m_jobFactory = jobFactory.Pass();
}

net::URLRequestJob *ProtocolHandlerRegistryQt::JobInterceptorFactory::MaybeCreateJobWithProtocolHandler(
        const std::string &scheme, net::URLRequest *request,
        net::NetworkDelegate *networkDelegate) const
{
    DCHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));
    net::URLRequestJob *job = m_ioThreadDelegate->maybeCreateJob(request, networkDelegate);
    if (job)
        return job;
    return m_jobFactory->MaybeCreateJobWithProtocolHandler(scheme, request, networkDelegate);
}

bool ProtocolHandlerRegistryQt::JobInterceptorFactory::IsHandledProtocol(
        const std::string &scheme) const
{
    DCHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));
    return m_ioThreadDelegate->isHandledProtocol(scheme) || m_jobFactory->IsHandledProtocol(scheme);
}

bool ProtocolHandlerRegistryQt::JobInterceptorFactory::IsHandledURL(const GURL &url) const
{
    DCHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));
    return (url.is_valid() && m_ioThreadDelegate->isHandledProtocol(url.scheme()))
            || m_jobFactory->IsHandledURL(url);
}

bool ProtocolHandlerRegistryQt::JobInterceptorFactory::IsSafeRedirectTarget(
        const GURL &location) const
{
    DCHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));
    return m_jobFactory->IsSafeRedirectTarget(location);
}

// ProtocolHandlerRegistryQt -----------------------------------------------------

ProtocolHandlerRegistryQt::ProtocolHandlerRegistryQt(content::BrowserContext *context)
    : m_browserContext(context)
    , m_ioThreadDelegate(new IOThreadDelegate(true))
{
}

ProtocolHandlerRegistryQt::~ProtocolHandlerRegistryQt()
{
}

void ProtocolHandlerRegistryQt::registerProtocolHandler(const CustomProtocolHandlerQt &handler)
{
    DCHECK(!handler.IsEmpty());
    if (isRegistered(handler))
        return;

    insertHandler(handler);
}

void ProtocolHandlerRegistryQt::onAcceptRegisterProtocolHandler(const CustomProtocolHandlerQt &handler)
{
    registerProtocolHandler(handler);
    setDefault(handler);
}

void ProtocolHandlerRegistryQt::onDenyRegisterProtocolHandler(
        const CustomProtocolHandlerQt &handler)
{
    registerProtocolHandler(handler);
}

void ProtocolHandlerRegistryQt::onIgnoreRegisterProtocolHandler(
        const CustomProtocolHandlerQt &handler)
{
    ignoreProtocolHandler(handler);
}

void ProtocolHandlerRegistryQt::ignoreProtocolHandler(const CustomProtocolHandlerQt &handler)
{
    m_ignoredProtocolHandlers.push_back(handler);
}

void ProtocolHandlerRegistryQt::insertHandler(const CustomProtocolHandlerQt &handler)
{
    CustomProtocolHandlerMultiMap::iterator p = m_protocolHandlers.find(handler.protocol());

    if (p != m_protocolHandlers.end()) {
        p->second.push_back(handler);
        return;
    }

    CustomProtocolHandlerList newList;
    newList.push_back(handler);
    m_protocolHandlers[handler.protocol()] = newList;
}

const ProtocolHandlerRegistryQt::CustomProtocolHandlerList *ProtocolHandlerRegistryQt::getHandlerList(const std::string &scheme) const
{
    CustomProtocolHandlerMultiMap::const_iterator p = m_protocolHandlers.find(scheme);
    if (p == m_protocolHandlers.end())
        return NULL;

    return &p->second;
}

bool ProtocolHandlerRegistryQt::canSchemeBeOverridden(const std::string& scheme) const
{
    const CustomProtocolHandlerList* handlers = getHandlerList(scheme);
    // If we already have a handler for this scheme, we can add more.
    if (handlers != NULL && !handlers->empty())
        return true;
    return false;
}

bool ProtocolHandlerRegistryQt::isRegistered(const CustomProtocolHandlerQt &handler) const
{
    const CustomProtocolHandlerList* handlers = getHandlerList(handler.protocol());
    if (!handlers)
        return false;

    return std::find(handlers->begin(), handlers->end(), handler) != handlers->end();
}

bool ProtocolHandlerRegistryQt::isIgnored(const CustomProtocolHandlerQt &handler) const
{
    CustomProtocolHandlerList::const_iterator i;
    for (i = m_ignoredProtocolHandlers.begin(); i != m_ignoredProtocolHandlers.end(); ++i) {
        if (*i == handler)
            return true;
    }
    return false;
}

void ProtocolHandlerRegistryQt::removeHandler(const CustomProtocolHandlerQt &handler)
{
    CustomProtocolHandlerList &handlers = m_protocolHandlers[handler.protocol()];
    CustomProtocolHandlerList::iterator p = std::find(handlers.begin(), handlers.end(), handler);
    if (p != handlers.end())
        handlers.erase(p);

    CustomProtocolHandlerMap::iterator q = m_defaultHandlers.find(handler.protocol());
    if (q != m_defaultHandlers.end() && q->second == handler) {
        // Make the new top handler in the list the default.
        if (!handlers.empty()) {
            // NOTE We pass a copy because SetDefault() modifies handlers.
            setDefault(CustomProtocolHandlerQt(handlers[0]));
        } else {
            BrowserThread::PostTask(BrowserThread::IO, FROM_HERE,
                    base::Bind(&IOThreadDelegate::clearDefault, m_ioThreadDelegate,
                            q->second.protocol()));

            m_defaultHandlers.erase(q);
        }
    }
}

void ProtocolHandlerRegistryQt::promoteHandler(const CustomProtocolHandlerQt &handler)
{
    DCHECK(isRegistered(handler));
    CustomProtocolHandlerMultiMap::iterator p = m_protocolHandlers.find(handler.protocol());
    CustomProtocolHandlerList& list = p->second;
    list.erase(std::find(list.begin(), list.end(), handler));
    list.insert(list.begin(), handler);
}

void ProtocolHandlerRegistryQt::setDefault(const CustomProtocolHandlerQt& handler)
{
    m_defaultHandlers.erase(handler.protocol());
    m_defaultHandlers.insert(std::make_pair(handler.protocol(), handler));
    promoteHandler(handler);
    BrowserThread::PostTask(BrowserThread::IO, FROM_HERE,
            base::Bind(&IOThreadDelegate::setDefault, m_ioThreadDelegate, handler));
}

void ProtocolHandlerRegistryQt::Shutdown()
{

}

scoped_ptr<ProtocolHandlerRegistryQt::JobInterceptorFactory> ProtocolHandlerRegistryQt::createJobInterceptorFactory()
{
    return scoped_ptr<JobInterceptorFactory>(new JobInterceptorFactory(m_ioThreadDelegate.get()));
}
