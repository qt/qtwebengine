// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef EXTENSIONS_RENDERER_API_PROVIDER_QT_H
#define EXTENSIONS_RENDERER_API_PROVIDER_QT_H

#include "extensions/renderer/extensions_renderer_api_provider.h"

namespace extensions {

class ExtensionsRendererAPIProviderQt : public ExtensionsRendererAPIProvider
{
public:
    ExtensionsRendererAPIProviderQt() = default;
    ExtensionsRendererAPIProviderQt(const ExtensionsRendererAPIProviderQt &) = delete;
    ExtensionsRendererAPIProviderQt &operator=(const ExtensionsRendererAPIProviderQt &) = delete;
    ~ExtensionsRendererAPIProviderQt() override = default;

    void PopulateSourceMap(ResourceBundleSourceMap *source_map) const override;
    void RegisterNativeHandlers(ModuleSystem *module_system,
                                NativeExtensionBindingsSystem *bindings_system,
                                V8SchemaRegistry *v8_schema_registry,
                                ScriptContext *context) const override {}
    void AddBindingsSystemHooks(Dispatcher *dispatcher,
                                NativeExtensionBindingsSystem *bindings_system) const override {}
    void EnableCustomElementAllowlist() const override { }
    void RequireWebViewModules(ScriptContext *context) const override { }
};
}
#endif // EXTENSIONS_RENDERER_API_PROVIDER_QT_H
