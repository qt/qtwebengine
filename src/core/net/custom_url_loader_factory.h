// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

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

#ifndef CUSTOM_URL_LOADER_FACTORY_H_
#define CUSTOM_URL_LOADER_FACTORY_H_

#include "mojo/public/cpp/bindings/pending_remote.h"

namespace network {
namespace mojom {
class URLLoaderFactory;
} // namespace mojom
} // namespace network

namespace QtWebEngineCore {
class ProfileAdapter;

mojo::PendingRemote<network::mojom::URLLoaderFactory> CreateCustomURLLoaderFactory(ProfileAdapter *profileAdapter);

} // namespace QtWebEngineCore

#endif // CUSTOM_URL_LOADER_FACTORY_H_
