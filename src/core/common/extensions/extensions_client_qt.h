// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// Portions copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXTENSIONS_CLIENT_QT_H
#define EXTENSIONS_CLIENT_QT_H

#include "extensions/common/extensions_client.h"

#include "base/lazy_instance.h"
#include "chrome/common/extensions/permissions/chrome_permission_message_provider.h"
#include "extensions/common/features/feature_provider.h"
#include "extensions/common/features/json_feature_provider_source.h"
#include "extensions/common/permissions/extensions_api_permissions.h"
#include "url/gurl.h"

namespace extensions {

// Sets up global state for the extensions system. Should be Set() once in each
// process. This should be implemented by the client of the extensions system.
class ExtensionsClientQt : public ExtensionsClient
{
public:
    ExtensionsClientQt();
    virtual ~ExtensionsClientQt() {}

    // Initializes global state. Not done in the constructor because unit tests
    // can create additional ExtensionsClients because the utility thread runs
    // in-process.
    void Initialize() override;
    void InitializeWebStoreUrls(base::CommandLine *command_line) override;

    // Returns the global PermissionMessageProvider to use to provide permission
    // warning strings.
    const PermissionMessageProvider &GetPermissionMessageProvider() const override;

    // Returns the application name. For example, "Chromium" or "app_shell".
    const std::string GetProductName() override;

    // Takes the list of all hosts and filters out those with special
    // permission strings. Adds the regular hosts to |new_hosts|,
    // and adds any additional permissions to |permissions|.
    // TODO(sashab): Split this function in two: One to filter out ignored host
    // permissions, and one to get permissions for the given hosts.
    void FilterHostPermissions(const URLPatternSet &hosts,
                               URLPatternSet *new_hosts,
                               PermissionIDSet *permissions) const override;

    // Replaces the scripting allowlist with |allowlist|. Used in the renderer;
    // only used for testing in the browser process.
    void SetScriptingAllowlist(const ScriptingAllowlist &allowlist) override;

    // Return the allowlist of extensions that can run content scripts on
    // any origin.
    const ScriptingAllowlist &GetScriptingAllowlist() const override;

    // Get the set of chrome:// hosts that |extension| can run content scripts on.
    URLPatternSet GetPermittedChromeSchemeHosts(const Extension *extension,
                                                const APIPermissionSet &api_permissions) const override;

    // Returns false if content scripts are forbidden from running on |url|.
    bool IsScriptableURL(const GURL &url, std::string *error) const override;

    // Returns the base webstore URL prefix.
    const GURL &GetWebstoreBaseURL() const override;

    // Returns the URL to use for update manifest queries.
    const GURL &GetWebstoreUpdateURL() const override;

    // Returns a flag indicating whether or not a given URL is a valid
    // extension blacklist URL.
    bool IsBlocklistUpdateURL(const GURL &url) const override;

    const GURL &GetNewWebstoreBaseURL() const override;

    // Returns the set of file paths corresponding to any images within an
    // extension's contents that may be displayed directly within the browser UI
    // or WebUI, such as icons or theme images. This set of paths is used by the
    // extension unpacker to determine which assets should be transcoded safely
    // within the utility sandbox.
    //
    // The default implementation returns the images used as icons for the
    // extension itself, so implementors of ExtensionsClient overriding this may
    // want to call the base class version and then add additional paths to that
    // result.
    std::set<base::FilePath> GetBrowserImagePaths(const Extension *extension) override;
    // Get the LazyInstance for ChromeExtensionsClient.
    static ExtensionsClientQt *GetInstance();

private:
    ScriptingAllowlist scripting_allowlist_;
    const ChromePermissionMessageProvider permission_message_provider_;
    mutable GURL update_url_;
    mutable GURL base_url_;
};

} // namespace extensions

#endif // EXTENSIONS_CLIENT_QT_H
