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

#include "extension_system_qt.h"

#include <algorithm>

#include "base/base_paths.h"
#include "base/base_switches.h"
#include "base/bind.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/json/json_string_value_serializer.h"
#include "base/memory/ptr_util.h"
#include "base/memory/weak_ptr.h"
#include "base/path_service.h"
#include "base/strings/string_tokenizer.h"
#include "base/strings/utf_string_conversions.h"
#include "base/task/post_task.h"
#include "base/time/time.h"
#include "base/trace_event/trace_event.h"
#include "build/build_config.h"
#include "components/crx_file/id_util.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/plugin_service.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/url_data_source.h"
#include "content/public/common/webplugininfo.h"
#include "extensions/browser/content_verifier.h"
#include "extensions/browser/content_verifier_delegate.h"
#include "extensions/browser/extension_pref_store.h"
#include "extensions/browser/extension_pref_value_map.h"
#include "extensions/browser/extension_pref_value_map_factory.h"
#include "extensions/browser/extension_prefs.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/browser/info_map.h"
#include "extensions/browser/notification_types.h"
#include "extensions/browser/null_app_sorting.h"
#include "extensions/browser/quota_service.h"
#include "extensions/browser/renderer_startup_helper.h"
#include "extensions/browser/runtime_data.h"
#include "extensions/browser/shared_user_script_master.h"
#include "extensions/browser/service_worker_manager.h"
#include "extensions/browser/value_store/value_store_factory_impl.h"
#include "extensions/common/constants.h"
#include "extensions/common/extension_messages.h"
#include "extensions/common/manifest_constants.h"
#include "extensions/common/manifest_handlers/mime_types_handler.h"
#include "extensions/common/manifest_url_handlers.h"
#include "net/base/mime_util.h"
#include "qtwebengine/grit/qt_webengine_resources.h"
#include "ui/base/resource/resource_bundle.h"

using content::BrowserThread;

namespace extensions {

namespace {

std::string GenerateId(const base::DictionaryValue *manifest, const base::FilePath &path)
{
    std::string raw_key;
    std::string id_input;
    CHECK(manifest->GetString(manifest_keys::kPublicKey, &raw_key));
    CHECK(Extension::ParsePEMKeyBytes(raw_key, &id_input));
    std::string id = crx_file::id_util::GenerateId(id_input);
    return id;
}

// Implementation based on ComponentLoader::ParseManifest.
std::unique_ptr<base::DictionaryValue> ParseManifest(const std::string &manifest_contents)
{
    JSONStringValueDeserializer deserializer(manifest_contents);
    std::unique_ptr<base::Value> manifest(deserializer.Deserialize(NULL, NULL));

    if (!manifest.get() || !manifest->is_dict()) {
        LOG(ERROR) << "Failed to parse extension manifest.";
        return NULL;
    }
    // Transfer ownership to the caller.
    return base::DictionaryValue::From(std::move(manifest));
}

} // namespace

// Dummy Content Verifier Delegate. Added to prevent crashes.
class ContentVerifierDelegateQt : public ContentVerifierDelegate
{
public:
    ~ContentVerifierDelegateQt() override {}

    // This should return what verification mode is appropriate for the given
    // extension, if any.
    VerifierSourceType GetVerifierSourceType(const Extension &extension) override
    { return VerifierSourceType::NONE; }

    // Should return the public key to use for validating signatures via the two
    // out parameters.
    ContentVerifierKey GetPublicKey() override { return ContentVerifierKey(); }
    // This should return a URL that can be used to fetch the
    // verified_contents.json containing signatures for the given extension
    // id/version pair.
    GURL GetSignatureFetchUrl(const std::string &extension_id, const base::Version &version) override { return GURL(); }

    // This should return the set of file paths for images used within the
    // browser process. (These may get transcoded during the install process).
    std::set<base::FilePath> GetBrowserImagePaths(const extensions::Extension *extension) override
    {
        return std::set<base::FilePath>();
    }

    // Called when the content verifier detects that a read of a file inside
    // an extension did not match its expected hash.
    void VerifyFailed(const std::string &extension_id, ContentVerifyJob::FailureReason reason) override {}

    // Called when ExtensionSystem is shutting down.
    void Shutdown() override {}
};

void ExtensionSystemQt::LoadExtension(std::string extension_id, std::unique_ptr<base::DictionaryValue> manifest, const base::FilePath &directory)
{
    int flags = Extension::REQUIRE_KEY;
    std::string error;
    scoped_refptr<const Extension> extension = Extension::Create(
            directory,
            Manifest::COMPONENT,
            *manifest,
            flags,
            &error);
    if (!extension.get())
        LOG(ERROR) << error;

    base::PostTask(FROM_HERE, {content::BrowserThread::IO},
            base::Bind(&InfoMap::AddExtension,
                       base::Unretained(info_map()),
                       base::RetainedRef(extension),
                       base::Time::Now(),
                       true,
                       false));
    extension_registry_->AddEnabled(extension.get());

    NotifyExtensionLoaded(extension.get());
}

void ExtensionSystemQt::OnExtensionRegisteredWithRequestContexts(scoped_refptr<const extensions::Extension> extension)
{
    extension_registry_->AddReady(extension);
    if (extension_registry_->enabled_extensions().Contains(extension->id()))
        extension_registry_->TriggerOnReady(extension.get());
}

// Implementation based on ExtensionService::NotifyExtensionLoaded.
void ExtensionSystemQt::NotifyExtensionLoaded(const Extension *extension)
{
    // The URLRequestContexts need to be first to know that the extension
    // was loaded, otherwise a race can arise where a renderer that is created
    // for the extension may try to load an extension URL with an extension id
    // that the request context doesn't yet know about. The profile is responsible
    // for ensuring its URLRequestContexts appropriately discover the loaded
    // extension.
    RegisterExtensionWithRequestContexts(
            extension,
            base::Bind(&ExtensionSystemQt::OnExtensionRegisteredWithRequestContexts,
                       weak_ptr_factory_.GetWeakPtr(),
                       base::WrapRefCounted(extension)));

    // Tell renderers about the loaded extension.
    renderer_helper_->OnExtensionLoaded(*extension);

    // Tell subsystems that use the ExtensionRegistryObserver::OnExtensionLoaded
    // about the new extension.
    //
    // NOTE: It is important that this happen after notifying the renderers about
    // the new extensions so that if we navigate to an extension URL in
    // ExtensionRegistryObserver::OnExtensionLoaded the renderer is guaranteed to
    // know about it.
    extension_registry_->TriggerOnLoaded(extension);

    // Register plugins included with the extension.
    // Implementation based on PluginManager::OnExtensionLoaded.
    const MimeTypesHandler *handler = MimeTypesHandler::GetHandler(extension);
    if (handler && !handler->handler_url().empty()) {
        content::WebPluginInfo info;
        info.type = content::WebPluginInfo::PLUGIN_TYPE_BROWSER_PLUGIN;
        info.name = base::UTF8ToUTF16(extension->name());
        info.path = base::FilePath::FromUTF8Unsafe(extension->url().spec());
        for (std::set<std::string>::const_iterator mime_type = handler->mime_type_set().begin();
             mime_type != handler->mime_type_set().end(); ++mime_type) {
            content::WebPluginMimeType mime_type_info;
            mime_type_info.mime_type = *mime_type;
            base::FilePath::StringType file_extension;
            if (net::GetPreferredExtensionForMimeType(*mime_type, &file_extension)) {
                mime_type_info.file_extensions.push_back(
                        base::FilePath(file_extension).AsUTF8Unsafe());
            }
            info.mime_types.push_back(mime_type_info);
        }
        content::PluginService *plugin_service =
                content::PluginService::GetInstance();
        plugin_service->RefreshPlugins();
        plugin_service->RegisterInternalPlugin(info, true);
    }
}

bool ExtensionSystemQt::FinishDelayedInstallationIfReady(const std::string &extension_id, bool install_immediately)
{
    // TODO mibrunin
    return false;
}

void ExtensionSystemQt::Shutdown()
{
    if (content_verifier_.get())
        content_verifier_->Shutdown();
}

ServiceWorkerManager *ExtensionSystemQt::service_worker_manager()
{
    return service_worker_manager_.get();
}

ExtensionService *ExtensionSystemQt::extension_service()
{
    return nullptr;
}

RuntimeData *ExtensionSystemQt::runtime_data()
{
    return runtime_data_.get();
}

ManagementPolicy *ExtensionSystemQt::management_policy()
{
    return nullptr;
}

SharedUserScriptMaster *ExtensionSystemQt::shared_user_script_master()
{
    return shared_user_script_master_.get();
}

StateStore *ExtensionSystemQt::state_store()
{
    return nullptr;
}

StateStore *ExtensionSystemQt::rules_store()
{
    return nullptr;
}

scoped_refptr<ValueStoreFactory> ExtensionSystemQt::store_factory()
{
    return store_factory_;
}

InfoMap *ExtensionSystemQt::info_map()
{
    if (!info_map_.get())
        info_map_ = new InfoMap;
    return info_map_.get();
}

QuotaService *ExtensionSystemQt::quota_service()
{
    return quota_service_.get();
}

AppSorting *ExtensionSystemQt::app_sorting()
{
    return app_sorting_.get();
}

ContentVerifier *ExtensionSystemQt::content_verifier()
{
    if (!content_verifier_.get()) {
        content_verifier_ = new ContentVerifier(browser_context_, std::make_unique<ContentVerifierDelegateQt>());
    }
    return content_verifier_.get();
}

ExtensionSystemQt::ExtensionSystemQt(content::BrowserContext *browserContext)
    : browser_context_(browserContext)
    , store_factory_(new ValueStoreFactoryImpl(browserContext->GetPath()))
    , extension_registry_(ExtensionRegistry::Get(browserContext))
    , renderer_helper_(extensions::RendererStartupHelperFactory::GetForBrowserContext(browserContext))
    , initialized_(false)
    , weak_ptr_factory_(this)
{
}

ExtensionSystemQt::~ExtensionSystemQt()
{
}

void ExtensionSystemQt::Init(bool extensions_enabled)
{
    if (initialized_)
        return;

    initialized_ = true;

    service_worker_manager_.reset(new ServiceWorkerManager(browser_context_));
    runtime_data_.reset(new RuntimeData(extension_registry_));
    quota_service_.reset(new QuotaService);
    app_sorting_.reset(new NullAppSorting);

    shared_user_script_master_ =
        std::make_unique<SharedUserScriptMaster>(browser_context_);

    // Make the chrome://extension-icon/ resource available.
    // content::URLDataSource::Add(browser_context_, new ExtensionIconSource(browser_context_));

    if (extensions_enabled) {
        // Inform the rest of the extensions system to start.
        ready_.Signal();
        content::NotificationService::current()->Notify(
            NOTIFICATION_EXTENSIONS_READY_DEPRECATED,
            content::Source<content::BrowserContext>(browser_context_),
            content::NotificationService::NoDetails());

        std::string pdf_manifest = ui::ResourceBundle::GetSharedInstance().GetRawDataResource(IDR_PDF_MANIFEST).as_string();
        base::ReplaceFirstSubstringAfterOffset(&pdf_manifest, 0, "<NAME>", "chromium-pdf");

        std::unique_ptr<base::DictionaryValue> pdfManifestDict = ParseManifest(pdf_manifest);
        base::FilePath path;
        base::PathService::Get(base::DIR_QT_LIBRARY_DATA, &path);
        path = path.Append(base::FilePath(FILE_PATH_LITERAL("pdf")));
        std::string id = GenerateId(pdfManifestDict.get(), path);
        LoadExtension(id, std::move(pdfManifestDict), path);
    }
}

void ExtensionSystemQt::InitForRegularProfile(bool extensions_enabled)
{
    if (initialized_)
        return; // Already initialized.
    // The InfoMap needs to be created before the ProcessManager.
    info_map();

    Init(extensions_enabled);
}

std::unique_ptr<ExtensionSet> ExtensionSystemQt::GetDependentExtensions(const Extension *extension)
{
    return base::WrapUnique(new ExtensionSet());
}

#if !defined(TOOLKIT_QT)
void ExtensionSystemQt::InstallUpdate(const std::string &extension_id,
                                      const std::string &public_key,
                                      const base::FilePath &unpacked_dir,
                                      bool install_immediately,
                                      InstallUpdateCallback install_update_callback)
{
    NOTREACHED() << "Not yet implemented";
    base::DeleteFile(unpacked_dir, true /* recursive */);
    std::move(install_update_callback).Run(CrxInstallError(CrxInstallErrorType::DECLINED, CrxInstallErrorDetail::DISALLOWED_BY_POLICY));
}
#endif

void ExtensionSystemQt::RegisterExtensionWithRequestContexts(const Extension *extension,
                                                             base::OnceClosure callback)
{
    base::Time install_time = base::Time::Now();

    bool incognito_enabled = false;
    bool notifications_disabled = false;

    base::PostTaskAndReply(
            FROM_HERE, {BrowserThread::IO},
            base::Bind(&InfoMap::AddExtension, info_map(),
                       base::RetainedRef(extension), install_time, incognito_enabled,
                       notifications_disabled),
            std::move(callback));
}

void ExtensionSystemQt::UnregisterExtensionWithRequestContexts(const std::string &extension_id,
                                                               const UnloadedExtensionReason reason)
{
    base::PostTask(
        FROM_HERE, {BrowserThread::IO},
        base::Bind(&InfoMap::RemoveExtension, info_map(), extension_id, reason));
}
} // namespace extensions
