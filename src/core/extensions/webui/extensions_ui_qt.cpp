// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "extensions_ui_qt.h"

#include "base/containers/span.h"
#include "chrome/common/url_constants.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_ui_data_source.h"
#include "chrome/browser/profiles/profile.h"
#include "qtwebengine/browser/extensions/resources/grit/extensions_ui_qt_resources.h"
#include "qtwebengine/browser/extensions/resources/grit/extensions_ui_qt_resources_map.h"
#include "ui/webui/webui_util.h"

#include "extensions_ui_page_handler_qt.h"

ExtensionsUIQt::ExtensionsUIQt(content::WebUI *web_ui) : ui::MojoWebUIController(web_ui, true)
{
    content::WebUIDataSource *source = content::WebUIDataSource::CreateAndAdd(
            web_ui->GetWebContents()->GetBrowserContext(), chrome::kChromeUIExtensionsHost);

    webui::SetupWebUIDataSource(
            source, base::span(kExtensionsUiQtResources, kExtensionsUiQtResourcesSize),
            IDR_EXTENSIONS_UI_QT_EXTENSIONS_UI_QT_HTML);

    source->OverrideContentSecurityPolicy(network::mojom::CSPDirectiveName::TrustedTypes,
                                          "trusted-types static-types polymer-html-literal "
                                          "polymer-template-event-attribute-policy "
                                          "lit-html-desktop;");
}

void ExtensionsUIQt::BindInterface(
        mojo::PendingReceiver<qtwebengine::mojom::ExtensionsUIHandlerFactory> receiver)
{
    page_factory_receiver_.reset();
    page_factory_receiver_.Bind(std::move(receiver));
}

void ExtensionsUIQt::CreatePageHandler(
        mojo::PendingRemote<qtwebengine::mojom::Page> page,
        mojo::PendingReceiver<qtwebengine::mojom::PageHandler> receiver)
{
    DCHECK(page);
    Profile *profile = Profile::FromWebUI(web_ui());
    page_handler_ = std::make_unique<ExtensionsUIPageHandlerQt>(
            web_ui(), profile, std::move(receiver), std::move(page));
}

WEB_UI_CONTROLLER_TYPE_IMPL(ExtensionsUIQt)
