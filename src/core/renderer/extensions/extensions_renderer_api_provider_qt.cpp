// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "extensions_renderer_api_provider_qt.h"

#include "chrome/grit/renderer_resources.h"
#include "extensions/renderer/resource_bundle_source_map.h"

namespace extensions {

void ExtensionsRendererAPIProviderQt::PopulateSourceMap(ResourceBundleSourceMap *source_map) const
{
    source_map->RegisterSource("webrtcDesktopCapturePrivate",
                               IDR_WEBRTC_DESKTOP_CAPTURE_PRIVATE_CUSTOM_BINDINGS_JS);
}

} // namespace extensions
