// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// Portions copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions_client_qt.h"

#include "extensions/common/alias.h"
#include "extensions/common/core_extensions_api_provider.h"
#include "extensions/common/extension_urls.h"

#include "extensions/common/features/simple_feature.h"
#include "extensions/common/permissions/permissions_info.h"

#include "extensions_api_provider_qt.h"


namespace extensions {

template<class FeatureClass> SimpleFeature *CreateFeature()
{
    return new FeatureClass;
}

static base::LazyInstance<ExtensionsClientQt>::Leaky g_client = LAZY_INSTANCE_INITIALIZER;

ExtensionsClientQt *ExtensionsClientQt::GetInstance()
{
    return g_client.Pointer();
}

ExtensionsClientQt::ExtensionsClientQt() : ExtensionsClient()
{
    AddAPIProvider(std::make_unique<CoreExtensionsAPIProvider>());
    AddAPIProvider(std::make_unique<ExtensionsAPIProviderQt>());
}

// Initializes global state. Not done in the constructor because unit tests
// can create additional ExtensionsClients because the utility thread runs
// in-process.
void ExtensionsClientQt::Initialize()
{
}

void ExtensionsClientQt::InitializeWebStoreUrls(base::CommandLine *command_line)
{
}

// Returns the global PermissionMessageProvider to use to provide permission
// warning strings.
const PermissionMessageProvider &ExtensionsClientQt::GetPermissionMessageProvider() const
{
    return permission_message_provider_;
}

// Returns the application name. For example, "Chromium" or "app_shell".
const std::string ExtensionsClientQt::GetProductName()
{
    return "Qt WebEngine"; // return Qt WebEngine for now, consider returning the application name if possible.
}

// Takes the list of all hosts and filters out those with special
// permission strings. Adds the regular hosts to |new_hosts|,
// and adds any additional permissions to |permissions|.
// TODO(sashab): Split this function in two: One to filter out ignored host
// permissions, and one to get permissions for the given hosts.
void ExtensionsClientQt::FilterHostPermissions(const URLPatternSet &hosts,
                                               URLPatternSet *new_hosts,
                                               PermissionIDSet *permissions) const
{
}

// Replaces the scripting allowlist with |allowlist|. Used in the renderer{}
// only used for testing in the browser process.
void ExtensionsClientQt::SetScriptingAllowlist(const ExtensionsClient::ScriptingAllowlist &allowlist)
{
    scripting_allowlist_ = allowlist;
}

// Return the allowlist of extensions that can run content scripts on
// any origin.
const ExtensionsClient::ScriptingAllowlist &ExtensionsClientQt::GetScriptingAllowlist() const
{
    return scripting_allowlist_;
}

// Get the set of chrome:// hosts that |extension| can run content scripts on.
URLPatternSet ExtensionsClientQt::GetPermittedChromeSchemeHosts(const Extension *extension,
                                                                const APIPermissionSet &api_permissions) const
{
    return URLPatternSet();
}

// Returns false if content scripts are forbidden from running on |url|.
bool ExtensionsClientQt::IsScriptableURL(const GURL &url, std::string *error) const
{
    return true;
}

// Returns the base webstore URL prefix.
const GURL &ExtensionsClientQt::GetWebstoreBaseURL() const
{
    if (base_url_.is_empty())
        base_url_ = GURL(extension_urls::kChromeWebstoreBaseURL);
    return base_url_;
}

// Returns the URL to use for update manifest queries.
const GURL &ExtensionsClientQt::GetWebstoreUpdateURL() const
{
    if (update_url_.is_empty())
        update_url_ = GURL(extension_urls::GetWebstoreUpdateUrl());
    return update_url_;
}

// Returns a flag indicating whether or not a given URL is a valid
// extension blacklist URL.
bool ExtensionsClientQt::IsBlocklistUpdateURL(const GURL &url) const
{
    return true;
}

const GURL &ExtensionsClientQt::GetNewWebstoreBaseURL() const
{
    static GURL dummy;
    return dummy;
}

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
std::set<base::FilePath> ExtensionsClientQt::GetBrowserImagePaths(const Extension *extension)
{
    return ExtensionsClient::GetBrowserImagePaths(extension);
}

} // namespace extensions
