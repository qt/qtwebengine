// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

// based on chrome/browser/plugin/plugin_host_impl.h
// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PLUGIN_INFO_HOST_QT_H_
#define PLUGIN_INFO_HOST_QT_H_

#include <memory>
#include <string>
#include <vector>

#include "base/memory/raw_ptr.h"
#include "base/memory/scoped_refptr.h"
#include "base/task/sequenced_task_runner_helpers.h"
#include "chrome/browser/plugins/plugin_metadata.h"
#include "components/content_settings/core/common/content_settings.h"
#include "components/keyed_service/core/keyed_service_shutdown_notifier.h"
#include "content/public/browser/browser_thread.h"
#include "extensions/buildflags/buildflags.h"
#include "media/media_buildflags.h"
#include "qtwebengine/common/plugin.mojom.h"

class GURL;
class Profile;

namespace content {
class BrowserContext;
struct WebPluginInfo;
}  // namespace content

namespace extensions {
class ExtensionRegistry;
}

namespace url {
class Origin;
}

namespace extensions {

// Implements PluginInfoHost interface.
class PluginInfoHostQt : public qtwebengine::mojom::PluginInfoHost {
 public:
  struct GetPluginInfo_Params;

  // Contains all the information needed by the PluginInfoHostQt.
  class Context {
   public:
    Context(int render_process_id, int render_frame_id, content::BrowserContext* profile);

    ~Context();

    int render_process_id() { return render_process_id_; }

    bool FindEnabledPlugin(
        const GURL& url,
        const std::string& mime_type,
        qtwebengine::mojom::PluginStatus* status,
        content::WebPluginInfo* plugin,
        std::string* actual_mime_type,
        std::unique_ptr<PluginMetadata>* plugin_metadata) const;

   private:
    int render_process_id_;
    int render_frame_id_;
#if BUILDFLAG(ENABLE_EXTENSIONS)
    raw_ptr<extensions::ExtensionRegistry, DanglingUntriaged>
        extension_registry_;
#endif
  };

  PluginInfoHostQt(int render_process_id, int render_frame_id, content::BrowserContext* profile);

  PluginInfoHostQt(const PluginInfoHostQt&) = delete;
  PluginInfoHostQt& operator=(const PluginInfoHostQt&) = delete;

  ~PluginInfoHostQt() override;

  // qtwebengine::mojom::PluginInfoHost
  void GetPluginInfo(const GURL& url,
                     const url::Origin& origin,
                     const std::string& mime_type,
                     GetPluginInfoCallback callback) override;

  static void EnsureFactoryBuilt();

 private:
  void ShutdownOnUIThread();

  // |params| wraps the parameters passed to |OnGetPluginInfo|, because
  // |base::Bind| doesn't support the required arity <http://crbug.com/98542>.
  void PluginsLoaded(const GetPluginInfo_Params& params,
                     GetPluginInfoCallback callback,
                     const std::vector<content::WebPluginInfo>& plugins);

  void GetPluginInfoFinish(const GetPluginInfo_Params& params,
                           qtwebengine::mojom::PluginInfoPtr output,
                           GetPluginInfoCallback callback,
                           std::unique_ptr<PluginMetadata> plugin_metadata);

  Context context_;
  base::CallbackListSubscription shutdown_subscription_;

  base::WeakPtrFactory<PluginInfoHostQt> weak_factory_{this};
};

} // namespace extensions

#endif  // PLUGIN_INFO_HOST_QT_H_
