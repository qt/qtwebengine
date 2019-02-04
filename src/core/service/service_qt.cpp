/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

// based on chrome/browser/chrome_service.cc:
// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "service_qt.h"

#include "base/no_destructor.h"
#include "base/task/post_task.h"
#include "components/spellcheck/spellcheck_buildflags.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/content_browser_client.h"
#include "services/service_manager/public/cpp/binder_registry.h"
#include "services/service_manager/public/cpp/connector.h"
#include "services/service_manager/public/cpp/service.h"
#include "services/service_manager/public/cpp/service_binding.h"

#if BUILDFLAG(ENABLE_SPELLCHECK)
#include "chrome/browser/spellchecker/spell_check_host_chrome_impl.h"
#endif

class ServiceQt::IOThreadContext : public service_manager::Service {
public:
    IOThreadContext();
    ~IOThreadContext() override = default;

    void BindServiceRequest(service_manager::mojom::ServiceRequest request);
    void BindConnector(service_manager::mojom::ConnectorRequest connector_request);

private:
    void BindConnectorOnIOThread(service_manager::mojom::ConnectorRequest connector_request);

    // service_manager::Service:
    void OnStart() override;
    void OnBindInterface(const service_manager::BindSourceInfo &remote_info,
                         const std::string &name,
                         mojo::ScopedMessagePipeHandle handle) override;

    service_manager::mojom::ConnectorRequest m_connectorRequest;
    service_manager::ServiceBinding m_serviceBinding{this};
    service_manager::BinderRegistry m_registry;
    service_manager::BinderRegistryWithArgs<const service_manager::BindSourceInfo&> m_registry_with_source_info;

    DISALLOW_COPY_AND_ASSIGN(IOThreadContext);
};

ServiceQt::IOThreadContext::IOThreadContext()
{
#if BUILDFLAG(ENABLE_SPELLCHECK)
    scoped_refptr<base::SingleThreadTaskRunner> ui_task_runner =
            base::CreateSingleThreadTaskRunnerWithTraits({content::BrowserThread::UI});
    m_registry_with_source_info.AddInterface(base::BindRepeating(&SpellCheckHostChromeImpl::Create), ui_task_runner);
#endif
}

void ServiceQt::IOThreadContext::BindServiceRequest(service_manager::mojom::ServiceRequest request)
{
    m_serviceBinding.Bind(std::move(request));
}

void ServiceQt::IOThreadContext::BindConnector(service_manager::mojom::ConnectorRequest connector_request)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

    // NOTE: It's not safe to modify |connector_request_| here since it's read
    // on the IO thread. Post a task instead. As long as this task is posted
    // before any code attempts to connect to the chrome service, there's no
    // race.
    base::CreateSingleThreadTaskRunnerWithTraits({content::BrowserThread::IO})->PostTask(
                FROM_HERE,
                base::BindOnce(&IOThreadContext::BindConnectorOnIOThread,
                               base::Unretained(this),
                               std::move(connector_request)));
}

void ServiceQt::IOThreadContext::BindConnectorOnIOThread(service_manager::mojom::ConnectorRequest connector_request)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    m_connectorRequest = std::move(connector_request);
}

void ServiceQt::IOThreadContext::OnStart()
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    DCHECK(m_connectorRequest.is_pending());
    m_serviceBinding.GetConnector()->BindConnectorRequest(std::move(m_connectorRequest));
}

void ServiceQt::IOThreadContext::OnBindInterface(const service_manager::BindSourceInfo &remote_info,
                                                 const std::string &name,
                                                 mojo::ScopedMessagePipeHandle handle)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    content::OverrideOnBindInterface(remote_info, name, &handle);
    if (!handle.is_valid())
        return;

    if (!m_registry.TryBindInterface(name, &handle))
        m_registry_with_source_info.TryBindInterface(name, &handle, remote_info);
}

ServiceQt *ServiceQt::GetInstance()
{
    static base::NoDestructor<ServiceQt> service;
    return service.get();
}

content::ServiceManagerConnection::ServiceRequestHandler ServiceQt::CreateServiceQtRequestHandler()
{
    return base::BindRepeating(&ServiceQt::BindServiceQtRequest, base::Unretained(this));
}

ServiceQt::ServiceQt() : m_ioThreadContext(std::make_unique<IOThreadContext>())
{}

ServiceQt::~ServiceQt() = default;

void ServiceQt::InitConnector()
{
    service_manager::mojom::ConnectorRequest request;
    m_connector = service_manager::Connector::Create(&request);
    m_ioThreadContext->BindConnector(std::move(request));
}

void ServiceQt::BindServiceQtRequest(service_manager::mojom::ServiceRequest request)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    m_ioThreadContext->BindServiceRequest(std::move(request));
}
