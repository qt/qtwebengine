// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "extensions_dispatcher_delegate_qt.h"

#include "chrome/grit/renderer_resources.h"
#include "extensions/renderer/resource_bundle_source_map.h"

namespace QtWebEngineCore {

ExtensionsDispatcherDelegateQt::ExtensionsDispatcherDelegateQt()
{
}

ExtensionsDispatcherDelegateQt::~ExtensionsDispatcherDelegateQt()
{
}

void ExtensionsDispatcherDelegateQt::PopulateSourceMap(extensions::ResourceBundleSourceMap *source_map)
{
    // Custom binding for hangout services extension.
    source_map->RegisterSource("webrtcDesktopCapturePrivate", IDR_WEBRTC_DESKTOP_CAPTURE_PRIVATE_CUSTOM_BINDINGS_JS);
}

} //namespace QtWebEngineCore
