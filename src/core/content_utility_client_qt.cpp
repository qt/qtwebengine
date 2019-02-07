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

#include "content_utility_client_qt.h"

#include "base/bind.h"
#include "content/public/utility/utility_thread.h"
#include "services/proxy_resolver/proxy_resolver_service.h"

namespace QtWebEngineCore {

ContentUtilityClientQt::ContentUtilityClientQt()
{
}

ContentUtilityClientQt::~ContentUtilityClientQt() = default;

namespace {

std::unique_ptr<service_manager::Service> CreateProxyResolverService(service_manager::mojom::ServiceRequest request)
{
    return std::make_unique<proxy_resolver::ProxyResolverService>(std::move(request));
}

using ServiceFactory = base::OnceCallback<std::unique_ptr<service_manager::Service>()>;
void RunServiceOnIOThread(ServiceFactory factory)
{
    base::OnceClosure terminate_process = base::BindOnce(
                base::IgnoreResult(&base::SequencedTaskRunner::PostTask),
                base::SequencedTaskRunnerHandle::Get(), FROM_HERE,
                base::BindOnce([] { content::UtilityThread::Get()->ReleaseProcess(); }));
    content::ChildThread::Get()->GetIOTaskRunner()->PostTask(
                FROM_HERE,
                base::BindOnce(
                    [](ServiceFactory factory, base::OnceClosure terminate_process) {
                        service_manager::Service::RunAsyncUntilTermination(
                                std::move(factory).Run(), std::move(terminate_process));
                    },
                    std::move(factory), std::move(terminate_process)));
}

}  // namespace

bool ContentUtilityClientQt::HandleServiceRequest(const std::string &service_name,
                                                  service_manager::mojom::ServiceRequest request)
{
    if (service_name == proxy_resolver::mojom::kProxyResolverServiceName) {
        RunServiceOnIOThread(base::BindOnce(&CreateProxyResolverService, std::move(request)));
        return true;
    }

    return false;
}

} // namespace
