// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Large parts of this file are based on the source code from the file
// chrome/browser/extensions/extension_system_impl.h from the Chromium sources.

#ifndef EXTENSION_SYSTEM_QT_H
#define EXTENSION_SYSTEM_QT_H

#include <string>

#include "base/one_shot_event.h"
#include "base/memory/weak_ptr.h"
#include "build/build_config.h"
#include "extensions/browser/extension_system.h"
#include "extensions/common/extension_set.h"

namespace value_store {
class ValueStoreFactory;
}

namespace extensions {

class ExtensionRegistry;
class InfoMap;
class RendererStartupHelper;
class ServiceWorkerManager;
class StateStoreNotificationObserver;

// The ExtensionSystem for ProfileImpl and OffTheRecordProfileImpl.
// Implementation details: non-shared services are owned by
// ExtensionSystemImpl, a KeyedService with separate incognito
// instances. A private Shared class (also a KeyedService,
// but with a shared instance for incognito) keeps the common services.
class ExtensionSystemQt : public ExtensionSystem
{
public:
    explicit ExtensionSystemQt(content::BrowserContext *browserContext);
    ~ExtensionSystemQt() override;

    // Initializes the extension system.
    void Initialize();

    // KeyedService implementation:
    void Shutdown() override;

    // ExtensionSystem implementation:
    void InitForRegularProfile(bool extensions_enabled) override;
    ExtensionService *extension_service() override;
    ManagementPolicy *management_policy() override;
    ServiceWorkerManager *service_worker_manager() override;
    UserScriptManager *user_script_manager() override;
    StateStore *state_store() override;
    StateStore *rules_store() override;
    StateStore *dynamic_user_scripts_store() override;
    scoped_refptr<value_store::ValueStoreFactory> store_factory() override;
    InfoMap *info_map() override;
    QuotaService *quota_service() override;
    AppSorting *app_sorting() override;

    void RegisterExtensionWithRequestContexts(const Extension *extension,
                                              base::OnceClosure callback) override;

    void UnregisterExtensionWithRequestContexts(const std::string &extension_id) override;

    ContentVerifier *content_verifier() override;
    std::unique_ptr<ExtensionSet> GetDependentExtensions(const Extension *extension) override;

    bool FinishDelayedInstallationIfReady(const std::string &extension_id, bool install_immediately) override;

    void Init(bool extensions_enabled);

    const base::OneShotEvent &ready() const override { return ready_; }
    bool is_ready() const override;

    void PerformActionBasedOnOmahaAttributes(const std::string &, const base::Value &) override { /* fixme? */}

private:
    void OnExtensionRegisteredWithRequestContexts(scoped_refptr<const extensions::Extension> extension);

    void NotifyExtensionLoaded(const Extension *extension);
    void LoadExtension(std::string extension_id, const base::Value::Dict &manifest, const base::FilePath &directory);
    // The services that are shared between normal and incognito profiles.

    // Data to be accessed on the IO thread. Must outlive process_manager_.
    scoped_refptr<InfoMap> info_map_;

    std::unique_ptr<ServiceWorkerManager> service_worker_manager_;
    std::unique_ptr<QuotaService> quota_service_;
    std::unique_ptr<UserScriptManager> user_script_manager_;


    // For verifying the contents of extensions read from disk.
    scoped_refptr<ContentVerifier> content_verifier_;
    base::OneShotEvent ready_;

    content::BrowserContext *browser_context_;
    scoped_refptr<value_store::ValueStoreFactory> store_factory_;
    ExtensionRegistry *extension_registry_;
    extensions::RendererStartupHelper *renderer_helper_;
    bool initialized_;

    base::WeakPtrFactory<ExtensionSystemQt> weak_ptr_factory_;
};

} // namespace extensions

#endif // EXTENSION_SYSTEM_QT_H
