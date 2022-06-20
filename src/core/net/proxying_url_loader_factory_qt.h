// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef PROXYING_URL_LOADER_FACTORY_QT_H_
#define PROXYING_URL_LOADER_FACTORY_QT_H_

#include "base/memory/weak_ptr.h"
#include "mojo/public/cpp/bindings/receiver_set.h"
#include "net/traffic_annotation/network_traffic_annotation.h"
#include "services/network/public/mojom/url_loader.mojom.h"
#include "services/network/public/mojom/url_loader_factory.mojom.h"

#include <QPointer>
// based on aw_proxying_url_loader_factory.h:
// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

namespace network {
struct ResourceRequest;
}

namespace QtWebEngineCore {

class ProfileAdapter;

class ProxyingURLLoaderFactoryQt : public network::mojom::URLLoaderFactory
{
public:
    ProxyingURLLoaderFactoryQt(ProfileAdapter *adapter, int frameTreeNodeId,
                               mojo::PendingReceiver<network::mojom::URLLoaderFactory> loader_receiver,
                               mojo::PendingRemote<network::mojom::URLLoaderFactory> pending_target_factory_remote);

    ~ProxyingURLLoaderFactoryQt() override;

    void CreateLoaderAndStart(mojo::PendingReceiver<network::mojom::URLLoader> loader,
                              int32_t request_id,
                              uint32_t options, const network::ResourceRequest &request,
                              mojo::PendingRemote<network::mojom::URLLoaderClient> client,
                              const net::MutableNetworkTrafficAnnotationTag &traffic_annotation) override;

    void Clone(mojo::PendingReceiver<network::mojom::URLLoaderFactory> receiver) override;

private:
    void OnTargetFactoryError();
    void OnProxyBindingError();

    QPointer<ProfileAdapter> m_profileAdapter;
    int m_frameTreeNodeId;
    mojo::ReceiverSet<network::mojom::URLLoaderFactory> m_proxyReceivers;
    mojo::Remote<network::mojom::URLLoaderFactory> m_targetFactory;
    base::WeakPtrFactory<ProxyingURLLoaderFactoryQt> m_weakFactory;
};

} // namespace QtWebEngineCore

#endif // PROXYING_URL_LOADER_FACTORY_QT_H_
