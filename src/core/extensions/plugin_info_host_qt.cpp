// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

// based on chrome/browser/plugin/plugin_host_impl.cc
// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "plugin_info_host_qt.h"

#include <stddef.h>

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "base/functional/bind.h"
#include "base/i18n/rtl.h"
#include "base/memory/singleton.h"
#include "build/build_config.h"
#include "components/keyed_service/content/browser_context_keyed_service_shutdown_notifier_factory.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/plugin_service.h"
#include "content/public/browser/plugin_service_filter.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/common/buildflags.h"
#include "content/public/common/content_constants.h"
#include "extensions/buildflags/buildflags.h"
#include "net/base/registry_controlled_domains/registry_controlled_domain.h"
#include "qtwebengine/common/plugin.mojom.h"
#include "url/gurl.h"
#include "url/origin.h"

#if BUILDFLAG(ENABLE_EXTENSIONS)
#include "components/guest_view/browser/guest_view_base.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/browser/guest_view/web_view/web_view_renderer_state.h"
#include "extensions/common/constants.h"
#include "extensions/common/extension.h"
#include "extensions/common/manifest_handlers/webview_info.h"
#endif

using content::PluginService;
using content::WebPluginInfo;

namespace {

const base::FilePath::CharType kPDFExtensionPluginPath[] =
    FILE_PATH_LITERAL("chrome-extension://mhjfbmdgcfjbbpaeojofohoefgiehjai/");
const base::FilePath::CharType kPDFInternalPluginPath[] =
    FILE_PATH_LITERAL("internal-pdf-viewer");

class PluginInfoHostQtShutdownNotifierFactory
    : public BrowserContextKeyedServiceShutdownNotifierFactory {
 public:
  static PluginInfoHostQtShutdownNotifierFactory* GetInstance() {
    return base::Singleton<PluginInfoHostQtShutdownNotifierFactory>::get();
  }

  PluginInfoHostQtShutdownNotifierFactory(
      const PluginInfoHostQtShutdownNotifierFactory&) = delete;
  PluginInfoHostQtShutdownNotifierFactory& operator=(
      const PluginInfoHostQtShutdownNotifierFactory&) = delete;

 private:
  friend struct base::DefaultSingletonTraits<
      PluginInfoHostQtShutdownNotifierFactory>;

  PluginInfoHostQtShutdownNotifierFactory()
      : BrowserContextKeyedServiceShutdownNotifierFactory(
            "PluginInfoHostQt") {}

  ~PluginInfoHostQtShutdownNotifierFactory() override = default;
};

std::unique_ptr<PluginMetadata> GetPluginMetadata(const WebPluginInfo& plugin) {
  // Gets the base name of the file path as the identifier.
  std::string identifier = plugin.path.BaseName().AsUTF8Unsafe();

  // Gets the plugin group name as the plugin name if it is not empty, or the
  // filename without extension if the name is empty.
  std::u16string group_name = plugin.name;
  if (group_name.empty()) {
    group_name = plugin.path.BaseName().RemoveExtension().AsUTF16Unsafe();
  } else {
    // Remove any unwanted locale direction characters from the group name.
    // For extension-based plugins, the plugin name is derived from the
    // extension name, and `extensions::Extension::LoadName()` may add locale
    // direction characters to the extension name.
    base::i18n::UnadjustStringForLocaleDirection(&group_name);
  }

  // Treat plugins as requiring authorization by default.
  PluginMetadata::SecurityStatus security_status =
      PluginMetadata::SECURITY_STATUS_REQUIRES_AUTHORIZATION;

  // Handle the PDF plugins specially.
  if (plugin.path.value() == kPDFExtensionPluginPath) {
    identifier = "chromium-pdf";
    security_status = PluginMetadata::SECURITY_STATUS_FULLY_TRUSTED;
  } else if (plugin.path.value() == kPDFInternalPluginPath) {
    identifier = "chromium-pdf-plugin";
    security_status = PluginMetadata::SECURITY_STATUS_FULLY_TRUSTED;
  }

  return std::make_unique<PluginMetadata>(identifier, group_name,
                                          security_status);
}

}  // namespace

namespace extensions {

PluginInfoHostQt::Context::Context(int render_process_id, int render_frame_id, content::BrowserContext* profile)
    : render_process_id_(render_process_id)
    , render_frame_id_(render_frame_id)
#if BUILDFLAG(ENABLE_EXTENSIONS)
    , extension_registry_(extensions::ExtensionRegistry::Get(profile))
#endif
{
}

PluginInfoHostQt::Context::~Context() = default;

PluginInfoHostQt::PluginInfoHostQt(int render_process_id, int render_frame_id, content::BrowserContext* profile)
    : context_(render_process_id, render_frame_id, profile) {
  shutdown_subscription_ =
      PluginInfoHostQtShutdownNotifierFactory::GetInstance()
          ->Get(profile)
          ->Subscribe(base::BindRepeating(
              &PluginInfoHostQt::ShutdownOnUIThread, base::Unretained(this)));
}

void PluginInfoHostQt::ShutdownOnUIThread() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  shutdown_subscription_ = {};
}

PluginInfoHostQt::~PluginInfoHostQt() = default;

struct PluginInfoHostQt::GetPluginInfo_Params {
  GURL url;
  url::Origin main_frame_origin;
  std::string mime_type;
};

void PluginInfoHostQt::GetPluginInfo(const GURL& url,
                                       const url::Origin& origin,
                                       const std::string& mime_type,
                                       GetPluginInfoCallback callback) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  GetPluginInfo_Params params = {url, origin, mime_type};
  PluginService::GetInstance()->GetPlugins(
      base::BindOnce(&PluginInfoHostQt::PluginsLoaded,
                     weak_factory_.GetWeakPtr(), params, std::move(callback)));
}

void PluginInfoHostQt::PluginsLoaded(
    const GetPluginInfo_Params& params,
    GetPluginInfoCallback callback,
    const std::vector<WebPluginInfo>& plugins) {
  qtwebengine::mojom::PluginInfoPtr output = qtwebengine::mojom::PluginInfo::New();
  // This also fills in |actual_mime_type|.
  std::unique_ptr<PluginMetadata> plugin_metadata;
  if (context_.FindEnabledPlugin(params.url, params.mime_type, &output->status,
                                 &output->plugin, &output->actual_mime_type,
                                 &plugin_metadata)) {
    if (plugin_metadata->security_status() == PluginMetadata::SECURITY_STATUS_FULLY_TRUSTED)
      output->status = qtwebengine::mojom::PluginStatus::kAllowed;
    else
      output->status = qtwebengine::mojom::PluginStatus::kUnauthorized;
  }

  GetPluginInfoFinish(params, std::move(output), std::move(callback),
                      std::move(plugin_metadata));
}

bool PluginInfoHostQt::Context::FindEnabledPlugin(
    const GURL& url,
    const std::string& mime_type,
    qtwebengine::mojom::PluginStatus* status,
    WebPluginInfo* plugin,
    std::string* actual_mime_type,
    std::unique_ptr<PluginMetadata>* plugin_metadata) const {
  *status = qtwebengine::mojom::PluginStatus::kAllowed;

  bool allow_wildcard = true;
  std::vector<WebPluginInfo> matching_plugins;
  std::vector<std::string> mime_types;
  PluginService::GetInstance()->GetPluginInfoArray(
      url, mime_type, allow_wildcard, &matching_plugins, &mime_types);
  if (matching_plugins.empty()) {
    *status = qtwebengine::mojom::PluginStatus::kNotFound;
    return false;
  }

  content::PluginServiceFilter* filter =
      PluginService::GetInstance()->GetFilter();
  content::RenderProcessHost* rph =
      content::RenderProcessHost::FromID(render_process_id_);
  content::BrowserContext* browser_context =
      rph ? rph->GetBrowserContext() : nullptr;
  size_t i = 0;
  for (; i < matching_plugins.size(); ++i) {
    if (!filter ||
        filter->IsPluginAvailable(render_process_id_, render_frame_id_, browser_context, matching_plugins[i])) {
      break;
    }
  }

  // If we broke out of the loop, we have found an enabled plugin.
  bool enabled = i < matching_plugins.size();
  if (!enabled) {
    // Otherwise, we only found disabled plugins, so we take the first one.
    i = 0;
    *status = qtwebengine::mojom::PluginStatus::kDisabled;
  }

  *plugin = matching_plugins[i];
  *actual_mime_type = mime_types[i];
  if (plugin_metadata)
    *plugin_metadata = GetPluginMetadata(*plugin);

  return enabled;
}

void PluginInfoHostQt::GetPluginInfoFinish(
    const GetPluginInfo_Params& params,
    qtwebengine::mojom::PluginInfoPtr output,
    GetPluginInfoCallback callback,
    std::unique_ptr<PluginMetadata> plugin_metadata) {
  if (plugin_metadata) {
    output->group_identifier = plugin_metadata->identifier();
    output->group_name = plugin_metadata->name();
  }

  std::move(callback).Run(std::move(output));
}

// static
void PluginInfoHostQt::EnsureFactoryBuilt() {
  PluginInfoHostQtShutdownNotifierFactory::GetInstance();
}

} // namespace extensions
