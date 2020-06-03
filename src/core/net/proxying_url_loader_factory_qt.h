/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
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

#ifndef PROXYING_URL_LOADER_FACTORY_QT_H_
#define PROXYING_URL_LOADER_FACTORY_QT_H_

#include "base/callback.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/ref_counted_delete_on_sequence.h"
#include "base/memory/weak_ptr.h"
#include "base/optional.h"
#include "mojo/public/cpp/bindings/receiver_set.h"
#include "net/traffic_annotation/network_traffic_annotation.h"
#include "services/network/public/cpp/resource_request.h"
#include "services/network/public/mojom/url_loader.mojom.h"
#include "services/network/public/mojom/url_loader_factory.mojom.h"
#include "url/gurl.h"

#include <QPointer>
// based on aw_proxying_url_loader_factory.h:
// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

QT_FORWARD_DECLARE_CLASS(QWebEngineUrlRequestInterceptor)

namespace content {
class ResourceContext;
}

namespace QtWebEngineCore {

class ProxyingURLLoaderFactoryQt : public network::mojom::URLLoaderFactory
{
public:
    ProxyingURLLoaderFactoryQt(int processId, QWebEngineUrlRequestInterceptor *profile,
                               QWebEngineUrlRequestInterceptor *page,
                               mojo::PendingReceiver<network::mojom::URLLoaderFactory> loader_receiver,
                               mojo::PendingRemote<network::mojom::URLLoaderFactory> pending_target_factory_remote);

    ~ProxyingURLLoaderFactoryQt() override;

    void CreateLoaderAndStart(mojo::PendingReceiver<network::mojom::URLLoader> loader,
                              int32_t routing_id, int32_t request_id,
                              uint32_t options, const network::ResourceRequest &request,
                              mojo::PendingRemote<network::mojom::URLLoaderClient> client,
                              const net::MutableNetworkTrafficAnnotationTag &traffic_annotation) override;

    void Clone(mojo::PendingReceiver<network::mojom::URLLoaderFactory> receiver) override;

private:
    void OnTargetFactoryError();
    void OnProxyBindingError();

    int m_processId;
    mojo::ReceiverSet<network::mojom::URLLoaderFactory> m_proxyReceivers;
    mojo::Remote<network::mojom::URLLoaderFactory> m_targetFactory;
    QPointer<QWebEngineUrlRequestInterceptor> m_profileRequestInterceptor;
    QPointer<QWebEngineUrlRequestInterceptor> m_pageRequestInterceptor;
    base::WeakPtrFactory<ProxyingURLLoaderFactoryQt> m_weakFactory;

    DISALLOW_COPY_AND_ASSIGN(ProxyingURLLoaderFactoryQt);
};

} // namespace QtWebEngineCore

#endif // PROXYING_URL_LOADER_FACTORY_QT_H_
