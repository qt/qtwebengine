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

// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Large parts of this file are based on the source code from the file
// chrome/browser/extensions/extension_system_impl.h from the Chromium sources.

#ifndef EXTENSION_SYSTEM_QT_H
#define EXTENSION_SYSTEM_QT_H

#include <string>

#include "base/one_shot_event.h"
#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "build/build_config.h"
#include "extensions/browser/extension_system.h"
#include "extensions/common/extension_set.h"

namespace extensions {

class ExtensionRegistry;
class InfoMap;
class RendererStartupHelper;
class ServiceWorkerManager;
class StateStoreNotificationObserver;
class ValueStoreFactory;
class ValueStoreFactoryImpl;

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
    RuntimeData *runtime_data() override;
    ManagementPolicy *management_policy() override;
    ServiceWorkerManager *service_worker_manager() override;
    SharedUserScriptMaster *shared_user_script_master() override;
    StateStore *state_store() override;
    StateStore *rules_store() override;
    scoped_refptr<ValueStoreFactory> store_factory() override;
    InfoMap *info_map() override;
    QuotaService *quota_service() override;
    AppSorting *app_sorting() override;

    void RegisterExtensionWithRequestContexts(const Extension *extension,
                                              base::OnceClosure callback) override;

    void UnregisterExtensionWithRequestContexts(const std::string &extension_id,
                                                const UnloadedExtensionReason reason) override;

    ContentVerifier *content_verifier() override;
    std::unique_ptr<ExtensionSet> GetDependentExtensions(const Extension *extension) override;

#if !defined(TOOLKIT_QT)
    void InstallUpdate(const std::string &extension_id,
                       const std::string &public_key,
                       const base::FilePath &unpacked_dir,
                       bool install_immediately,
                       InstallUpdateCallback install_update_callback) override;
#endif // TOOLKIT_QT
    //friend class ExtensionSystemSharedFactory;

    bool FinishDelayedInstallationIfReady(const std::string &extension_id, bool install_immediately) override;

    void Init(bool extensions_enabled);

    const base::OneShotEvent &ready() const override { return ready_; }

private:
    void OnExtensionRegisteredWithRequestContexts(scoped_refptr<const extensions::Extension> extension);

    void NotifyExtensionLoaded(const Extension *extension);
    void LoadExtension(std::string extension_id, std::unique_ptr<base::DictionaryValue> manifest, const base::FilePath &directory);
    // The services that are shared between normal and incognito profiles.

    // Data to be accessed on the IO thread. Must outlive process_manager_.
    scoped_refptr<InfoMap> info_map_;

    std::unique_ptr<ServiceWorkerManager> service_worker_manager_;
    std::unique_ptr<RuntimeData> runtime_data_;
    std::unique_ptr<QuotaService> quota_service_;
    std::unique_ptr<AppSorting> app_sorting_;
    std::unique_ptr<SharedUserScriptMaster> shared_user_script_master_;


    // For verifying the contents of extensions read from disk.
    scoped_refptr<ContentVerifier> content_verifier_;
    base::OneShotEvent ready_;

    content::BrowserContext *browser_context_;
    scoped_refptr<ValueStoreFactory> store_factory_;
    ExtensionRegistry *extension_registry_;
    extensions::RendererStartupHelper *renderer_helper_;
    bool initialized_;

    base::WeakPtrFactory<ExtensionSystemQt> weak_ptr_factory_;
    DISALLOW_COPY_AND_ASSIGN(ExtensionSystemQt);
};

} // namespace extensions

#endif // EXTENSION_SYSTEM_QT_H
