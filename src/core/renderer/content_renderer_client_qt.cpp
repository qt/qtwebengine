/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#include "renderer/content_renderer_client_qt.h"

#include "common/qt_messages.h"

#include "base/strings/string_split.h"
#include "base/strings/utf_string_conversions.h"
#include "chrome/common/localized_error.h"
#if defined(ENABLE_SPELLCHECK)
#include "chrome/renderer/spellchecker/spellcheck.h"
#include "chrome/renderer/spellchecker/spellcheck_provider.h"
#endif
#include "components/cdm/renderer/widevine_key_systems.h"
#include "components/error_page/common/error_page_params.h"
#if defined (ENABLE_BASIC_PRINTING)
#include "components/printing/renderer/print_web_view_helper.h"
#endif // if defined(ENABLE_BASIC_PRINTING)
#include "components/visitedlink/renderer/visitedlink_slave.h"
#include "components/web_cache/renderer/web_cache_render_process_observer.h"
#include "content/public/renderer/render_frame.h"
#include "content/public/renderer/render_thread.h"
#include "content/public/renderer/render_view.h"
#include "net/base/net_errors.h"
#include "third_party/WebKit/public/platform/WebString.h"
#include "third_party/WebKit/public/platform/WebURLError.h"
#include "third_party/WebKit/public/platform/WebURLRequest.h"
#include "third_party/WebKit/public/web/WebSecurityPolicy.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/webui/jstemplate_builder.h"
#include "content/public/common/web_preferences.h"

#include "renderer/web_channel_ipc_transport.h"
#if defined (ENABLE_BASIC_PRINTING)
#include "renderer/print_web_view_helper_delegate_qt.h"
#endif // if defined(ENABLE_BASIC_PRINTING)

#include "renderer/render_frame_observer_qt.h"
#include "renderer/render_view_observer_qt.h"
#include "renderer/user_resource_controller.h"

#include "grit/renderer_resources.h"

#include "widevine_cdm_version.h" // In SHARED_INTERMEDIATE_DIR.

namespace QtWebEngineCore {

static const char kHttpErrorDomain[] = "http";
static const char kQrcSchemeQt[] = "qrc";

class RenderProcessObserverQt : public content::RenderProcessObserver {
public:
    void WebKitInitialized() override
    {
        // Can only be done after blink is initialized.
        blink::WebString qrcScheme(base::ASCIIToUTF16(kQrcSchemeQt));
        // mark qrc as a secure scheme (avoids deprecation warnings)
        blink::WebSecurityPolicy::registerURLSchemeAsSecure(qrcScheme);
    }
};

ContentRendererClientQt::ContentRendererClientQt()
{
}

ContentRendererClientQt::~ContentRendererClientQt()
{
}

void ContentRendererClientQt::RenderThreadStarted()
{
    content::RenderThread *renderThread = content::RenderThread::Get();
    m_visitedLinkSlave.reset(new visitedlink::VisitedLinkSlave);
    m_webCacheObserver.reset(new web_cache::WebCacheRenderProcessObserver());
    m_renderProcessObserver.reset(new RenderProcessObserverQt());
    renderThread->AddObserver(m_visitedLinkSlave.data());
    renderThread->AddObserver(m_webCacheObserver.data());
    renderThread->AddObserver(UserResourceController::instance());
    renderThread->AddObserver(m_renderProcessObserver.data());

#if defined(ENABLE_SPELLCHECK)
    m_spellCheck.reset(new SpellCheck());
    renderThread->AddObserver(m_spellCheck.data());
#endif
}

void ContentRendererClientQt::RenderViewCreated(content::RenderView* render_view)
{
    // RenderViewObservers destroy themselves with their RenderView.
    new RenderViewObserverQt(render_view, m_webCacheObserver.data());
    new WebChannelIPCTransport(render_view);
    UserResourceController::instance()->renderViewCreated(render_view);
#if defined(ENABLE_SPELLCHECK)
    new SpellCheckProvider(render_view, m_spellCheck.data());
#endif

#if defined(ENABLE_BASIC_PRINTING)
    new printing::PrintWebViewHelper(
        render_view,
        scoped_ptr<printing::PrintWebViewHelper::Delegate>(
            new PrintWebViewHelperDelegateQt()));
#endif // defined(ENABLE_BASIC_PRINTING)
}

void ContentRendererClientQt::RenderFrameCreated(content::RenderFrame* render_frame)
{
    new QtWebEngineCore::RenderFrameObserverQt(render_frame);
}

bool ContentRendererClientQt::HasErrorPage(int httpStatusCode, std::string *errorDomain)
{
    // Use an internal error page, if we have one for the status code.
    if (!LocalizedError::HasStrings(LocalizedError::kHttpErrorDomain, httpStatusCode)) {
        return false;
    }

    *errorDomain = LocalizedError::kHttpErrorDomain;
    return true;
}

bool ContentRendererClientQt::ShouldSuppressErrorPage(content::RenderFrame *frame, const GURL &)
{
    return !(frame->GetWebkitPreferences().enable_error_page);
}

// To tap into the chromium localized strings. Ripped from the chrome layer (highly simplified).
void ContentRendererClientQt::GetNavigationErrorStrings(content::RenderFrame* renderFrame, const blink::WebURLRequest &failedRequest, const blink::WebURLError &error, std::string *errorHtml, base::string16 *errorDescription)
{
    const bool isPost = QByteArray::fromStdString(failedRequest.httpMethod().utf8()) == QByteArrayLiteral("POST");

    if (errorHtml) {
        // Use a local error page.
        int resourceId;
        base::DictionaryValue errorStrings;

        const std::string locale = content::RenderThread::Get()->GetLocale();
        // TODO(elproxy): We could potentially get better diagnostics here by first calling
        // NetErrorHelper::GetErrorStringsForDnsProbe, but that one is harder to untangle.
        LocalizedError::GetStrings(error.reason, error.domain.utf8(), error.unreachableURL, isPost
                                  , error.staleCopyInCache && !isPost, false, error_page::OfflinePageStatus::NONE, locale, renderFrame->GetRenderView()->GetAcceptLanguages()
                                  , scoped_ptr<error_page::ErrorPageParams>(), &errorStrings);
        resourceId = IDR_NET_ERROR_HTML;


        const base::StringPiece template_html(ui::ResourceBundle::GetSharedInstance().GetRawDataResource(resourceId));
        if (template_html.empty())
            NOTREACHED() << "unable to load template. ID: " << resourceId;
        else // "t" is the id of the templates root node.
            *errorHtml = webui::GetTemplatesHtml(template_html, &errorStrings, "t");
    }

    if (errorDescription)
        *errorDescription = LocalizedError::GetErrorDetails(error.domain.utf8(), error.reason, isPost);
}

unsigned long long ContentRendererClientQt::VisitedLinkHash(const char *canonicalUrl, size_t length)
{
    return m_visitedLinkSlave->ComputeURLFingerprint(canonicalUrl, length);
}

bool ContentRendererClientQt::IsLinkVisited(unsigned long long linkHash)
{
    return m_visitedLinkSlave->IsVisited(linkHash);
}

// The following is based on chrome/renderer/media/chrome_key_systems.cc:
// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if defined(ENABLE_PEPPER_CDMS)
static bool IsPepperCdmAvailable(const std::string& pepper_type,
                                 std::vector<base::string16>* additional_param_names,
                                 std::vector<base::string16>* additional_param_values)
{
    bool is_available = false;
    content::RenderThread::Get()->Send(
        new QtWebEngineHostMsg_IsInternalPluginAvailableForMimeType(
            pepper_type,
            &is_available,
            additional_param_names,
            additional_param_values));

    return is_available;
}

// External Clear Key (used for testing).
static void AddExternalClearKey(std::vector<media::KeySystemInfo>* concrete_key_systems)
{
      static const char kExternalClearKeyKeySystem[] =
          "org.chromium.externalclearkey";
      static const char kExternalClearKeyDecryptOnlyKeySystem[] =
          "org.chromium.externalclearkey.decryptonly";
      static const char kExternalClearKeyFileIOTestKeySystem[] =
          "org.chromium.externalclearkey.fileiotest";
      static const char kExternalClearKeyInitializeFailKeySystem[] =
          "org.chromium.externalclearkey.initializefail";
      static const char kExternalClearKeyCrashKeySystem[] =
          "org.chromium.externalclearkey.crash";
      static const char kExternalClearKeyPepperType[] =
          "application/x-ppapi-clearkey-cdm";

    std::vector<base::string16> additional_param_names;
    std::vector<base::string16> additional_param_values;
    if (!IsPepperCdmAvailable(kExternalClearKeyPepperType,
                              &additional_param_names,
                              &additional_param_values))
        return;

    media::KeySystemInfo info;
    info.key_system = kExternalClearKeyKeySystem;

    info.supported_init_data_types =
        media::kInitDataTypeMaskWebM | media::kInitDataTypeMaskKeyIds;
    info.supported_codecs = media::EME_CODEC_WEBM_ALL;
#if defined(USE_PROPRIETARY_CODECS)
    info.supported_init_data_types |= media::kInitDataTypeMaskCenc;
    info.supported_codecs |= media::EME_CODEC_MP4_ALL;
#endif  // defined(USE_PROPRIETARY_CODECS)

    info.max_audio_robustness = media::EmeRobustness::EMPTY;
    info.max_video_robustness = media::EmeRobustness::EMPTY;

    // Persistent sessions are faked.
    info.persistent_license_support = media::EmeSessionTypeSupport::SUPPORTED;
    info.persistent_release_message_support =
        media::EmeSessionTypeSupport::NOT_SUPPORTED;
    info.persistent_state_support = media::EmeFeatureSupport::REQUESTABLE;
    info.distinctive_identifier_support = media::EmeFeatureSupport::NOT_SUPPORTED;

    info.pepper_type = kExternalClearKeyPepperType;

    concrete_key_systems->push_back(info);

    // Add support of decrypt-only mode in ClearKeyCdm.
    info.key_system = kExternalClearKeyDecryptOnlyKeySystem;
    concrete_key_systems->push_back(info);

    // A key system that triggers FileIO test in ClearKeyCdm.
    info.key_system = kExternalClearKeyFileIOTestKeySystem;
    concrete_key_systems->push_back(info);

    // A key system that Chrome thinks is supported by ClearKeyCdm, but actually
    // will be refused by ClearKeyCdm. This is to test the CDM initialization
    // failure case.
    info.key_system = kExternalClearKeyInitializeFailKeySystem;
    concrete_key_systems->push_back(info);

    // A key system that triggers a crash in ClearKeyCdm.
    info.key_system = kExternalClearKeyCrashKeySystem;
    concrete_key_systems->push_back(info);
}

#if defined(WIDEVINE_CDM_AVAILABLE)

static void AddPepperBasedWidevine(std::vector<media::KeySystemInfo>* concrete_key_systems)
{
//#if defined(WIDEVINE_CDM_MIN_GLIBC_VERSION)
//    Version glibc_version(gnu_get_libc_version());
//    DCHECK(glibc_version.IsValid());
//    if (glibc_version.IsOlderThan(WIDEVINE_CDM_MIN_GLIBC_VERSION))
//        return;
//#endif  // defined(WIDEVINE_CDM_MIN_GLIBC_VERSION)

    std::vector<base::string16> additional_param_names;
    std::vector<base::string16> additional_param_values;
    if (!IsPepperCdmAvailable(kWidevineCdmPluginMimeType,
                              &additional_param_names,
                              &additional_param_values)) {
        DVLOG(1) << "Widevine CDM is not currently available.";
        return;
    }

    media::SupportedCodecs supported_codecs = media::EME_CODEC_NONE;

    supported_codecs |= media::EME_CODEC_WEBM_OPUS;
    supported_codecs |= media::EME_CODEC_WEBM_VORBIS;
    supported_codecs |= media::EME_CODEC_WEBM_VP8;
    supported_codecs |= media::EME_CODEC_WEBM_VP9;
#if defined(USE_PROPRIETARY_CODECS)
    supported_codecs |= media::EME_CODEC_MP4_AVC1;
    supported_codecs |= media::EME_CODEC_MP4_AAC;
#endif  // defined(USE_PROPRIETARY_CODECS)

    cdm::AddWidevineWithCodecs(
        cdm::WIDEVINE, supported_codecs,
        media::EmeRobustness::SW_SECURE_CRYPTO,       // Maximum audio robustness.
        media::EmeRobustness::SW_SECURE_DECODE,       // Maximum video robustness.
        media::EmeSessionTypeSupport::NOT_SUPPORTED,  // persistent-license.
        media::EmeSessionTypeSupport::NOT_SUPPORTED,  // persistent-release-message.
        media::EmeFeatureSupport::REQUESTABLE,    // Persistent state.
        media::EmeFeatureSupport::NOT_SUPPORTED,  // Distinctive identifier.
        concrete_key_systems);
}
#endif  // defined(WIDEVINE_CDM_AVAILABLE)
#endif  // defined(ENABLE_PEPPER_CDMS)

void ContentRendererClientQt::AddKeySystems(std::vector<media::KeySystemInfo>* key_systems_info)
{
#if defined(ENABLE_PEPPER_CDMS)
    AddExternalClearKey(key_systems_info);

#if defined(WIDEVINE_CDM_AVAILABLE)
    AddPepperBasedWidevine(key_systems_info);
#endif  // defined(WIDEVINE_CDM_AVAILABLE)
#endif  // defined(ENABLE_PEPPER_CDMS)
}

} // namespace
