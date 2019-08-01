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

// Portions copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXTENSIONS_CLIENT_QT_H
#define EXTENSIONS_CLIENT_QT_H

#include "extensions/common/extensions_client.h"

#include "base/compiler_specific.h"
#include "base/lazy_instance.h"
#include "base/macros.h"
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

    // Replaces the scripting whitelist with |whitelist|. Used in the renderer;
    // only used for testing in the browser process.
    void SetScriptingWhitelist(const ScriptingWhitelist &whitelist) override;

    // Return the whitelist of extensions that can run content scripts on
    // any origin.
    const ScriptingWhitelist &GetScriptingWhitelist() const override;

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
    bool IsBlacklistUpdateURL(const GURL &url) const override;

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
    ScriptingWhitelist scripting_whitelist_;
    const ChromePermissionMessageProvider permission_message_provider_;
    mutable GURL update_url_;
    mutable GURL base_url_;
    DISALLOW_COPY_AND_ASSIGN(ExtensionsClientQt);
};

} // namespace extensions

#endif // EXTENSIONS_CLIENT_QT_H
