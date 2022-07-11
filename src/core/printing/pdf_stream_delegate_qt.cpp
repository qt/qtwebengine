// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// based on chrome/browser/pdf/chrome_pdf_stream_delegate.cc:

#include "pdf_stream_delegate_qt.h"

#include "base/no_destructor.h"
#include "chrome/grit/pdf_resources.h"
#include "extensions/browser/guest_view/mime_handler_view/mime_handler_view_guest.h"
#include "extensions/common/constants.h"
#include "ui/base/resource/resource_bundle.h"

// Associates a `pdf::PdfStreamDelegate::StreamInfo` with a `WebContents`.
// `PdfStreamDelegateQt::MapToOriginalUrl()` initializes this in
// `PdfNavigationThrottle`, and then `PdfStreamDelegateQt::GetStreamInfo()`
// returns the stashed result to `PdfURLLoaderRequestInterceptor`.
class StreamInfoHelper : public content::WebContentsUserData<StreamInfoHelper>
{
public:
    absl::optional<pdf::PdfStreamDelegate::StreamInfo> TakeStreamInfo()
    { return std::move(stream_info_); }

private:
    friend class content::WebContentsUserData<StreamInfoHelper>;
    WEB_CONTENTS_USER_DATA_KEY_DECL();

    StreamInfoHelper(content::WebContents *contents,
                     pdf::PdfStreamDelegate::StreamInfo stream_info)
        : content::WebContentsUserData<StreamInfoHelper>(*contents),
          stream_info_(std::move(stream_info)) {}

    absl::optional<pdf::PdfStreamDelegate::StreamInfo> stream_info_;
};

WEB_CONTENTS_USER_DATA_KEY_IMPL(StreamInfoHelper);

PdfStreamDelegateQt::PdfStreamDelegateQt() = default;
PdfStreamDelegateQt::~PdfStreamDelegateQt() = default;

absl::optional<GURL> PdfStreamDelegateQt::MapToOriginalUrl(content::WebContents *contents, const GURL &stream_url)
{
    StreamInfoHelper *helper = StreamInfoHelper::FromWebContents(contents);
    if (helper) {
        // PDF viewer and Print Preview only do this once per WebContents.
        return absl::nullopt;
    }

    GURL original_url;
    StreamInfo info;

    extensions::MimeHandlerViewGuest *guest =
            extensions::MimeHandlerViewGuest::FromWebContents(contents);
    if (guest) {
        base::WeakPtr<extensions::StreamContainer> stream = guest->GetStreamWeakPtr();
        if (!stream || stream->extension_id() != extension_misc::kPdfExtensionId ||
            stream->stream_url() != stream_url ||
            !stream->pdf_plugin_attributes()) {
            return absl::nullopt;
        }

        original_url = stream->original_url();
        info.background_color = base::checked_cast<SkColor>(stream->pdf_plugin_attributes()->background_color);
        info.full_frame = !stream->embedded();
        info.allow_javascript = stream->pdf_plugin_attributes()->allow_javascript;
    } else {
        return absl::nullopt;
    }

    static const base::NoDestructor<std::string> injected_script(
                ui::ResourceBundle::GetSharedInstance().LoadDataResourceString(
                    IDR_PDF_PDF_INTERNAL_PLUGIN_WRAPPER_ROLLUP_JS));

    info.stream_url = stream_url;
    info.original_url = original_url;
    info.injected_script = injected_script.get();
    StreamInfoHelper::CreateForWebContents(contents, std::move(info));
    return original_url;
}

absl::optional<pdf::PdfStreamDelegate::StreamInfo>
PdfStreamDelegateQt::GetStreamInfo(content::WebContents *contents)
{
    StreamInfoHelper *helper = StreamInfoHelper::FromWebContents(contents);
    if (!helper)
        return absl::nullopt;

    // Only the call immediately following `MapToOriginalUrl()` requires a valid
    // `StreamInfo`; subsequent calls should just get nothing.
    return helper->TakeStreamInfo();
}
