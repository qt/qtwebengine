// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// based on //chrome/browser/plugins/pdf_iframe_navigation_throttle.cc
// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#include "extensions/pdf_iframe_navigation_throttle_qt.h"

#include "base/task/sequenced_task_runner.h"
#include "chrome/grit/renderer_resources.h"
#include "components/strings/grit/components_strings.h"
#include "content/public/browser/download_utils.h"
#include "content/public/browser/navigation_handle.h"
#include "content/public/browser/plugin_service.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_user_data.h"
#include "content/public/common/webplugininfo.h"
#include "base/strings/escape.h"
#include "net/http/http_response_headers.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/webui/jstemplate_builder.h"
#include "ui/base/webui/web_ui_util.h"

#include <QtGlobal>

namespace extensions {

constexpr char kPDFMimeType[] = "application/pdf";

// Used to scope the posted navigation task to the lifetime of |web_contents|.
class PdfWebContentsLifetimeHelper : public content::WebContentsUserData<PdfWebContentsLifetimeHelper>
{
public:
    explicit PdfWebContentsLifetimeHelper(content::WebContents *web_contents)
        : content::WebContentsUserData<PdfWebContentsLifetimeHelper>(*web_contents)
    {}

    base::WeakPtr<PdfWebContentsLifetimeHelper> GetWeakPtr()
    {
        return weak_factory_.GetWeakPtr();
    }

    void NavigateIFrameToPlaceholder(const content::OpenURLParams &url_params)
    {
        GetWebContents().OpenURL(url_params);
    }

private:
    friend class content::WebContentsUserData<PdfWebContentsLifetimeHelper>;

    base::WeakPtrFactory<PdfWebContentsLifetimeHelper> weak_factory_{this};

    WEB_CONTENTS_USER_DATA_KEY_DECL();
};

WEB_CONTENTS_USER_DATA_KEY_IMPL(PdfWebContentsLifetimeHelper);

bool IsPDFPluginEnabled(content::NavigationHandle *navigation_handle, bool *is_stale)
{
    content::WebContents *web_contents = navigation_handle->GetWebContents();
    Q_ASSERT(web_contents);

    if (web_contents->IsInnerWebContentsForGuest())
        web_contents = web_contents->GetOuterWebContents();

    int process_id = web_contents->GetPrimaryMainFrame()->GetProcess()->GetID();
    int routing_id = web_contents->GetPrimaryMainFrame()->GetRoutingID();
    content::WebPluginInfo plugin_info;
    // Will check WebEngineSettings by PluginServiceFilterQt
    return content::PluginService::GetInstance()->GetPluginInfo(
                process_id, routing_id,
                navigation_handle->GetWebContents()->GetBrowserContext(),
                navigation_handle->GetURL(),
                kPDFMimeType, false /* allow_wildcard */,
                is_stale, &plugin_info, nullptr /* actual_mime_type */);
}

std::string GetPDFPlaceholderHTML(const GURL &pdf_url)
{
    std::string template_html = ui::ResourceBundle::GetSharedInstance().LoadDataResourceString(IDR_PDF_PLUGIN_HTML);
    webui::AppendWebUiCssTextDefaults(&template_html);

    base::Value::Dict values;
    values.Set("fileName", pdf_url.ExtractFileName());
    values.Set("open", l10n_util::GetStringUTF8(IDS_ACCNAME_OPEN));
    values.Set("pdfUrl", pdf_url.spec());

    return webui::GetI18nTemplateHtml(template_html, std::move(values));
}

// static
std::unique_ptr<content::NavigationThrottle>
PDFIFrameNavigationThrottleQt::MaybeCreateThrottleFor(content::NavigationHandle *handle)
{
    if (handle->IsInMainFrame())
        return nullptr;
    return std::make_unique<PDFIFrameNavigationThrottleQt>(handle);
}

PDFIFrameNavigationThrottleQt::PDFIFrameNavigationThrottleQt(content::NavigationHandle *handle)
    : content::NavigationThrottle(handle)
{
}

PDFIFrameNavigationThrottleQt::~PDFIFrameNavigationThrottleQt()
{
}

content::NavigationThrottle::ThrottleCheckResult PDFIFrameNavigationThrottleQt::WillProcessResponse()
{
    const net::HttpResponseHeaders *response_headers = navigation_handle()->GetResponseHeaders();
    if (!response_headers)
        return content::NavigationThrottle::PROCEED;

    std::string mime_type;
    response_headers->GetMimeType(&mime_type);
    if (mime_type != kPDFMimeType)
        return content::NavigationThrottle::PROCEED;

    // We MUST download responses marked as attachments rather than showing
    // a placeholder.
    if (content::download_utils::MustDownload(navigation_handle()->GetURL(), response_headers, mime_type))
        return content::NavigationThrottle::PROCEED;

    bool is_stale = false;
    bool pdf_plugin_enabled = IsPDFPluginEnabled(navigation_handle(), &is_stale);

    if (is_stale) {
        // On browser start, the plugin list may not be ready yet.
        content::PluginService::GetInstance()->GetPlugins(
            base::BindOnce(&PDFIFrameNavigationThrottleQt::OnPluginsLoaded,
                           weak_factory_.GetWeakPtr()));
        return content::NavigationThrottle::DEFER;
    }

    // If the plugin was found, proceed on the navigation. Otherwise fall through
    // to the placeholder case.
    if (pdf_plugin_enabled)
        return content::NavigationThrottle::PROCEED;

    LoadPlaceholderHTML();
    return content::NavigationThrottle::CANCEL_AND_IGNORE;
}

const char *PDFIFrameNavigationThrottleQt::GetNameForLogging()
{
    return "PDFIFrameNavigationThrottleQt";
}

void PDFIFrameNavigationThrottleQt::OnPluginsLoaded(
                        const std::vector<content::WebPluginInfo> &plugins)
{
    if (IsPDFPluginEnabled(navigation_handle(), nullptr /* is_stale */)) {
        Resume();
    } else {
        LoadPlaceholderHTML();
        CancelDeferredNavigation(content::NavigationThrottle::CANCEL_AND_IGNORE);
    }
}

void PDFIFrameNavigationThrottleQt::LoadPlaceholderHTML()
{
    // Prepare the params to navigate to the placeholder.
    std::string html = GetPDFPlaceholderHTML(navigation_handle()->GetURL());
    GURL data_url("data:text/html," + base::EscapePath(html));
    content::OpenURLParams params = content::OpenURLParams::FromNavigationHandle(navigation_handle());
    params.url = data_url;
    params.transition = ui::PAGE_TRANSITION_AUTO_SUBFRAME;

    // Post a task to navigate to the placeholder HTML. We don't navigate
    // synchronously here, as starting a navigation within a navigation is
    // an antipattern. Use a helper object scoped to the WebContents lifetime to
    // scope the navigation task to the WebContents lifetime.
    content::WebContents *web_contents = navigation_handle()->GetWebContents();
    if (!web_contents)
        return;

    PdfWebContentsLifetimeHelper::CreateForWebContents(web_contents);
    PdfWebContentsLifetimeHelper *helper = PdfWebContentsLifetimeHelper::FromWebContents(web_contents);
    base::SequencedTaskRunner::GetCurrentDefault()->PostTask(
        FROM_HERE,
        base::BindOnce(&PdfWebContentsLifetimeHelper::NavigateIFrameToPlaceholder,
                       helper->GetWeakPtr(), std::move(params)));
}

} // namespace extensions
