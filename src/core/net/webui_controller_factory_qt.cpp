// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// Based on chrome/browser/ui/webui/chrome_web_ui_controller_factory.cc:
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "webui_controller_factory_qt.h"

#include "build_config_qt.h"
#include "devtools_frontend_qt.h"
#include "base/functional/bind.h"
#include "build/build_config.h"
#include "chrome/browser/accessibility/accessibility_ui.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/webui/device_log_ui.h"
#include "chrome/browser/ui/webui/devtools_ui.h"
#include "chrome/browser/ui/webui/net_internals/net_internals_ui.h"
#include "chrome/browser/ui/webui/user_actions/user_actions_ui.h"
#include "chrome/common/url_constants.h"
#include "content/public/browser/web_ui.h"
#include "content/public/common/url_utils.h"
#include "extensions/buildflags/buildflags.h"
#include "media/media_buildflags.h"
#include "printing/buildflags/buildflags.h"
#include "url/gurl.h"
#include "version_ui_qt.h"

#if BUILDFLAG(IS_WIN) || BUILDFLAG(IS_LINUX)
#include "chrome/browser/ui/webui/sandbox/sandbox_internals_ui.h"
#endif

#if QT_CONFIG(webengine_webrtc) && QT_CONFIG(webengine_extensions)
#include "chrome/browser/ui/webui/media/webrtc_logs_ui.h"
#endif

// The Following WebUIs are disabled because they currently doesn't build
// or doesn't work, but would be interesting for us if they did:

// #include "chrome/browser/ui/webui/inspect_ui.h"

// #if BUILDFLAG(ENABLE_PRINT_PREVIEW)
// #include "chrome/browser/ui/webui/print_preview/print_preview_ui.h"
// #endif

// #if defined(USE_NSS_CERTS) && defined(USE_AURA)
// #include "chrome/browser/ui/webui/certificate_viewer_ui.h"
// #endif

// #if BUILDFLAG(ENABLE_EXTENSIONS)
// #include "chrome/browser/extensions/extension_web_ui.h"
// #include "chrome/browser/ui/webui/extensions/extensions_ui.h"
// #include "chrome/common/extensions/extension_constants.h"
// #include "extensions/browser/extension_registry.h"
// #include "extensions/browser/extension_system.h"
// #include "extensions/common/constants.h"
// #include "extensions/common/extension.h"
// #include "extensions/common/feature_switch.h"
// #include "extensions/common/manifest.h"
// #endif

using content::WebUI;
using content::WebUIController;

namespace {

// A function for creating a new WebUI. The caller owns the return value, which
// may be NULL (for example, if the URL refers to an non-existent extension).
typedef std::unique_ptr<WebUIController> (*WebUIFactoryFunction)(WebUI *web_ui, const GURL &url);

// Template for defining WebUIFactoryFunction.
template<class T>
std::unique_ptr<WebUIController> NewWebUI(WebUI *web_ui, const GURL & /*url*/)
{
    return std::unique_ptr<WebUIController>(new T(web_ui));
}

// Returns a function that can be used to create the right type of WebUI for a
// tab, based on its URL. Returns NULL if the URL doesn't have WebUI associated
// with it.
WebUIFactoryFunction GetWebUIFactoryFunction(WebUI *web_ui, Profile *profile, const GURL &url)
{
    Q_UNUSED(web_ui);
    Q_UNUSED(profile);
    // This will get called a lot to check all URLs, so do a quick check of other
    // schemes to filter out most URLs.
    if (!content::HasWebUIScheme(url))
        return nullptr;

    // We must compare hosts only since some of the Web UIs append extra stuff
    // after the host name.
    if (url.host_piece() == chrome::kChromeUINetInternalsHost)
        return &NewWebUI<NetInternalsUI>;

    if (url.SchemeIs(content::kChromeDevToolsScheme)) {
        if (!QtWebEngineCore::DevToolsFrontendQt::IsValidFrontendURL(url))
            return nullptr;
        return &NewWebUI<DevToolsUI>;
    }
    if (url.host_piece() == chrome::kChromeUIAccessibilityHost)
        return &NewWebUI<AccessibilityUI>;

    if (url.host_piece() == chrome::kChromeUIUserActionsHost)
        return &NewWebUI<UserActionsUI>;

    if (url.host_piece() == chrome::kChromeUIDeviceLogHost)
        return &NewWebUI<chromeos::DeviceLogUI>;

    if (url.host_piece() == chrome::kChromeUIVersionQtHost)
        return &NewWebUI<VersionUIQt>;

//    if (url.host_piece() == chrome::kChromeUIInspectHost)
//        return &NewWebUI<InspectUI>;
//
//#if defined(USE_NSS_CERTS) && defined(USE_AURA)
//    if (url.host_piece() == chrome::kChromeUICertificateViewerHost)
//        return &NewWebUI<CertificateViewerUI>;
//#endif  // USE_NSS_CERTS && USE_AURA
//#if BUILDFLAG(ENABLE_EXTENSIONS)
//    if (url.host_piece() == chrome::kChromeUIExtensionsFrameHost)
//        return &NewWebUI<extensions::ExtensionsUI>;
//#endif
//#if BUILDFLAG(ENABLE_PRINT_PREVIEW)
//    if (url.host_piece() == chrome::kChromeUIPrintHost &&
//        !profile->GetPrefs()->GetBoolean(prefs::kPrintPreviewDisabled)) {
//        return &NewWebUI<PrintPreviewUI>;
//    }
//#endif
#if QT_CONFIG(webengine_webrtc) && QT_CONFIG(webengine_extensions)
    if (url.host_piece() == chrome::kChromeUIWebRtcLogsHost)
        return &NewWebUI<WebRtcLogsUI>;
#endif
#if BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_WIN)
    if (url.host_piece() == chrome::kChromeUISandboxHost)
        return &NewWebUI<SandboxInternalsUI>;
#endif
    return nullptr;
}

} // namespace

namespace QtWebEngineCore {

WebUI::TypeID WebUIControllerFactoryQt::GetWebUIType(content::BrowserContext *browser_context, const GURL &url)
{
    Profile *profile = Profile::FromBrowserContext(browser_context);
    WebUIFactoryFunction function = GetWebUIFactoryFunction(nullptr, profile, url);
    return function ? reinterpret_cast<WebUI::TypeID>(function) : WebUI::kNoWebUI;
}

bool WebUIControllerFactoryQt::UseWebUIForURL(content::BrowserContext *browser_context, const GURL &url)
{
    return GetWebUIType(browser_context, url) != WebUI::kNoWebUI;
}

std::unique_ptr<WebUIController> WebUIControllerFactoryQt::CreateWebUIControllerForURL(WebUI *web_ui, const GURL &url)
{
    Profile *profile = Profile::FromWebUI(web_ui);
    WebUIFactoryFunction function = GetWebUIFactoryFunction(web_ui, profile, url);
    if (!function)
        return nullptr;

    return (*function)(web_ui, url);
}

// static
WebUIControllerFactoryQt *WebUIControllerFactoryQt::GetInstance()
{
    return base::Singleton<WebUIControllerFactoryQt>::get();
}

WebUIControllerFactoryQt::WebUIControllerFactoryQt()
{
}

WebUIControllerFactoryQt::~WebUIControllerFactoryQt()
{
}

} // namespace QtWebEngineCore
