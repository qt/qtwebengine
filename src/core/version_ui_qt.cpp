// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "version_ui_qt.h"
#include "api/qtwebenginecoreglobal.h"
#include "build/build_config.h"
#include "base/command_line.h"
#include "chrome/common/url_constants.h"
#include "chrome/browser/profiles/profile.h"
#include "qtwebengine/grit/qt_webengine_resources.h"
#include "services/network/public/cpp/content_security_policy/content_security_policy.h"

namespace {
const char kQtWebEngineVersion[] = "qtwebengine_version";
const char kQtWebEngineChromiumVersion[] = "qtwebengine_chromium_version";
const char kQtWebEngineChromiumSecurityPatchVersion[] =
        "qtwebengine_chromium_security_patch_version";
const char kCommandLine[] = "command_line";
const char kQtVersionCSS[] = "qt_version.css";
const char kQtLogo[] = "images/qt.png";
const char kQtWebEngineLogo[] = "images/qtwebengine.png";
}

VersionUIQt::VersionUIQt(content::WebUI *web_ui) : content::WebUIController(web_ui)
{

    Profile *profile = Profile::FromWebUI(web_ui);
    content::WebUIDataSource *html_source =
            content::WebUIDataSource::Create(chrome::kChromeUIVersionQtHost);
    html_source->OverrideContentSecurityPolicy(
            network::mojom::CSPDirectiveName::ScriptSrc,
            "script-src chrome://resources 'self' 'unsafe-inline';");
    html_source->SetDefaultResource(IDR_VERSION_UI_QT_HTML);
    html_source->AddResourcePath(kQtVersionCSS, IDR_VERSION_UI_QT_CSS);
    html_source->AddResourcePath(kQtLogo, IDR_QT_LOGO);
    html_source->AddResourcePath(kQtWebEngineLogo, IDR_QTWEBENGINE_LOGO);

    html_source->AddString(kQtWebEngineVersion, qWebEngineVersion());
    html_source->AddString(kQtWebEngineChromiumVersion, qWebEngineChromiumVersion());
    html_source->AddString(kQtWebEngineChromiumSecurityPatchVersion,
                           qWebEngineChromiumSecurityPatchVersion());
#if BUILDFLAG(IS_WIN)
    html_source->AddString(
            kCommandLine,
            base::AsString16(base::CommandLine::ForCurrentProcess()->GetCommandLineString()));
#else
    std::string command_line;
    typedef std::vector<std::string> ArgvList;
    const ArgvList &argv = base::CommandLine::ForCurrentProcess()->argv();
    for (auto iter = argv.begin(); iter != argv.end(); iter++)
        command_line += " " + *iter;
    html_source->AddString(kCommandLine, command_line);
#endif
    content::WebUIDataSource::Add(profile, html_source);
}

VersionUIQt::~VersionUIQt() { }
