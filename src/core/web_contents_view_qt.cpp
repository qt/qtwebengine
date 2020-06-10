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

#include "web_contents_view_qt.h"

#include "profile_adapter.h"
#include "content_browser_client_qt.h"
#include "render_widget_host_view_qt.h"
#include "render_widget_host_view_qt_delegate.h"
#include "render_widget_host_view_qt.h"
#include "touch_selection_controller_client_qt.h"
#include "type_conversion.h"
#include "web_contents_adapter_client.h"
#include "web_contents_adapter.h"
#include "web_engine_context.h"
#include "web_contents_delegate_qt.h"

#include "components/spellcheck/spellcheck_buildflags.h"
#include "content/browser/renderer_host/render_view_host_impl.h"
#include "content/browser/renderer_host/render_widget_host_impl.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "content/public/browser/context_menu_params.h"
#include "content/public/browser/web_contents_delegate.h"
#include "third_party/blink/public/common/context_menu_data/edit_flags.h"
#include "third_party/blink/public/common/context_menu_data/media_type.h"
#include "ui/gfx/image/image_skia.h"

#include <QtGui/qpixmap.h>

namespace QtWebEngineCore {

void WebContentsViewQt::setFactoryClient(WebContentsAdapterClient* client)
{
    if (m_factoryClient)
        return;
    m_factoryClient = client;

    // Check if a RWHV was created before the pre-initialization.
    if (auto view = static_cast<RenderWidgetHostViewQt *>(m_webContents->GetRenderWidgetHostView())) {
        view->setDelegate(m_factoryClient->CreateRenderWidgetHostViewQtDelegate(view));
    }
}

void WebContentsViewQt::setClient(WebContentsAdapterClient* client)
{
    m_client = client;
    m_factoryClient = client;

    // Check if a RWHV was created before the initialization.
    if (auto rwhv = static_cast<RenderWidgetHostViewQt *>(m_webContents->GetRenderWidgetHostView())) {
        rwhv->setAdapterClient(client);
        rwhv->SetBackgroundColor(toSk(client->backgroundColor()));
    }
}

content::RenderWidgetHostViewBase* WebContentsViewQt::CreateViewForWidget(content::RenderWidgetHost *render_widget_host)
{
    RenderWidgetHostViewQt *view = new RenderWidgetHostViewQt(render_widget_host);

    if (m_factoryClient) {
        view->setDelegate(m_factoryClient->CreateRenderWidgetHostViewQtDelegate(view));
        if (m_client)
            view->setAdapterClient(m_client);
    }

    return view;
}

content::RenderWidgetHostViewBase* WebContentsViewQt::CreateViewForChildWidget(content::RenderWidgetHost* render_widget_host)
{
    RenderWidgetHostViewQt *view = new RenderWidgetHostViewQt(render_widget_host);

    Q_ASSERT(m_client);
    view->setDelegate(m_client->CreateRenderWidgetHostViewQtDelegateForPopup(view));
    view->setAdapterClient(m_client);

    return view;
}

void WebContentsViewQt::CreateView(gfx::NativeView context)
{
}

gfx::NativeView WebContentsViewQt::GetNativeView() const
{
    return nullptr;
}

void WebContentsViewQt::GetContainerBounds(gfx::Rect* out) const
{
    if (m_client) {
        const QRectF r(m_client->viewportRect());
        *out = gfx::Rect(r.x(), r.y(), r.width(), r.height());
    }
}

void WebContentsViewQt::Focus()
{
    if (!m_client->isEnabled())
        return;
    if (content::RenderWidgetHostView *rwhv = m_webContents->GetRenderWidgetHostView())
        rwhv->Focus();
    m_client->focusContainer();
}

void WebContentsViewQt::SetInitialFocus()
{
    Focus();
}

void WebContentsViewQt::FocusThroughTabTraversal(bool reverse)
{
    content::WebContentsImpl *web_contents = static_cast<content::WebContentsImpl*>(m_webContents);
    if (web_contents->ShowingInterstitialPage()) {
        web_contents->GetInterstitialPage()->FocusThroughTabTraversal(reverse);
        return;
    }
    content::RenderWidgetHostView *fullscreen_view = web_contents->GetFullscreenRenderWidgetHostView();
    if (fullscreen_view) {
        fullscreen_view->Focus();
        return;
    }
    web_contents->GetRenderViewHost()->SetInitialFocus(reverse);
}


ASSERT_ENUMS_MATCH(WebEngineContextMenuData::MediaTypeNone, blink::ContextMenuDataMediaType::kNone)
ASSERT_ENUMS_MATCH(WebEngineContextMenuData::MediaTypeImage, blink::ContextMenuDataMediaType::kImage)
ASSERT_ENUMS_MATCH(WebEngineContextMenuData::MediaTypeVideo, blink::ContextMenuDataMediaType::kVideo)
ASSERT_ENUMS_MATCH(WebEngineContextMenuData::MediaTypeAudio, blink::ContextMenuDataMediaType::kAudio)
ASSERT_ENUMS_MATCH(WebEngineContextMenuData::MediaTypeCanvas, blink::ContextMenuDataMediaType::kCanvas)
ASSERT_ENUMS_MATCH(WebEngineContextMenuData::MediaTypeFile, blink::ContextMenuDataMediaType::kFile)
ASSERT_ENUMS_MATCH(WebEngineContextMenuData::MediaTypePlugin, blink::ContextMenuDataMediaType::kPlugin)

ASSERT_ENUMS_MATCH(WebEngineContextMenuData::MediaNone, blink::WebContextMenuData::kMediaNone)
ASSERT_ENUMS_MATCH(WebEngineContextMenuData::MediaInError, blink::WebContextMenuData::kMediaInError)
ASSERT_ENUMS_MATCH(WebEngineContextMenuData::MediaPaused, blink::WebContextMenuData::kMediaPaused)
ASSERT_ENUMS_MATCH(WebEngineContextMenuData::MediaMuted, blink::WebContextMenuData::kMediaMuted)
ASSERT_ENUMS_MATCH(WebEngineContextMenuData::MediaLoop, blink::WebContextMenuData::kMediaLoop)
ASSERT_ENUMS_MATCH(WebEngineContextMenuData::MediaCanSave, blink::WebContextMenuData::kMediaCanSave)
ASSERT_ENUMS_MATCH(WebEngineContextMenuData::MediaHasAudio, blink::WebContextMenuData::kMediaHasAudio)
ASSERT_ENUMS_MATCH(WebEngineContextMenuData::MediaCanToggleControls, blink::WebContextMenuData::kMediaCanToggleControls)
ASSERT_ENUMS_MATCH(WebEngineContextMenuData::MediaControls, blink::WebContextMenuData::kMediaControls)
ASSERT_ENUMS_MATCH(WebEngineContextMenuData::MediaCanPrint, blink::WebContextMenuData::kMediaCanPrint)
ASSERT_ENUMS_MATCH(WebEngineContextMenuData::MediaCanRotate, blink::WebContextMenuData::kMediaCanRotate)

ASSERT_ENUMS_MATCH(WebEngineContextMenuData::CanDoNone, blink::kCanDoNone)
ASSERT_ENUMS_MATCH(WebEngineContextMenuData::CanUndo, blink::kCanUndo)
ASSERT_ENUMS_MATCH(WebEngineContextMenuData::CanRedo, blink::kCanRedo)
ASSERT_ENUMS_MATCH(WebEngineContextMenuData::CanCut, blink::kCanCut)
ASSERT_ENUMS_MATCH(WebEngineContextMenuData::CanCopy, blink::kCanCopy)
ASSERT_ENUMS_MATCH(WebEngineContextMenuData::CanPaste, blink::kCanPaste)
ASSERT_ENUMS_MATCH(WebEngineContextMenuData::CanDelete, blink::kCanDelete)
ASSERT_ENUMS_MATCH(WebEngineContextMenuData::CanSelectAll, blink::kCanSelectAll)
ASSERT_ENUMS_MATCH(WebEngineContextMenuData::CanTranslate, blink::kCanTranslate)
ASSERT_ENUMS_MATCH(WebEngineContextMenuData::CanEditRichly, blink::kCanEditRichly)

static inline WebEngineContextMenuData fromParams(const content::ContextMenuParams &params)
{
    WebEngineContextMenuData ret;
    ret.setPosition(QPoint(params.x, params.y));
    ret.setLinkUrl(toQt(params.link_url));
    ret.setLinkText(toQt(params.link_text.data()));
    ret.setAltText(toQt(params.alt_text.data()));
    ret.setTitleText(toQt(params.title_text.data()));
    ret.setUnfilteredLinkUrl(toQt(params.unfiltered_link_url));
    ret.setSelectedText(toQt(params.selection_text.data()));
    ret.setMediaUrl(toQt(params.src_url));
    ret.setMediaType((WebEngineContextMenuData::MediaType)params.media_type);
    ret.setHasImageContent(params.has_image_contents);
    ret.setMediaFlags((WebEngineContextMenuData::MediaFlags)params.media_flags);
    ret.setEditFlags((WebEngineContextMenuData::EditFlags)params.edit_flags);
    ret.setSuggestedFileName(toQt(params.suggested_filename.data()));
    ret.setIsEditable(params.is_editable);
#if QT_CONFIG(webengine_spellchecker)
    ret.setMisspelledWord(toQt(params.misspelled_word));
    ret.setSpellCheckerSuggestions(fromVector(params.dictionary_suggestions));
#endif
    ret.setFrameUrl(toQt(params.frame_url));
    ret.setPageUrl(toQt(params.page_url));
    ret.setReferrerPolicy((ReferrerPolicy)params.referrer_policy);
    return ret;
}

void WebContentsViewQt::ShowContextMenu(content::RenderFrameHost *, const content::ContextMenuParams &params)
{
    if (auto rwhv = static_cast<RenderWidgetHostViewQt *>(m_webContents->GetRenderWidgetHostView())) {
        if (rwhv && rwhv->getTouchSelectionControllerClient()->handleContextMenu(params))
            return;
    }

    WebEngineContextMenuData contextMenuData(fromParams(params));
#if QT_CONFIG(webengine_spellchecker)
    // Do not use params.spellcheck_enabled, since it is never
    // correctly initialized for chrome asynchronous spellchecking.
    // Even fixing the initialization in ContextMenuClientImpl::showContextMenu
    // will not work. By default SpellCheck::spellcheck_enabled_
    // must be initialized to true due to the way how the initialization sequence
    // in SpellCheck works ie. typing the first word triggers the creation
    // of the SpellcheckService. Use user preference store instead.
    contextMenuData.setIsSpellCheckerEnabled(m_client->profileAdapter()->isSpellCheckEnabled());
#endif
    m_client->contextMenuRequested(contextMenuData);
}

Qt::DropActions toQtDropActions(blink::WebDragOperationsMask ops)
{
    Qt::DropActions result;
    if (ops & blink::kWebDragOperationCopy)
        result |= Qt::CopyAction;
    if (ops & blink::kWebDragOperationLink)
        result |= Qt::LinkAction;
    if (ops & blink::kWebDragOperationMove || ops & blink::kWebDragOperationDelete)
        result |= Qt::MoveAction;
    return result;
}

void WebContentsViewQt::StartDragging(const content::DropData &drop_data,
                                      blink::WebDragOperationsMask allowed_ops,
                                      const gfx::ImageSkia &image,
                                      const gfx::Vector2d &image_offset,
                                      const content::DragEventSourceInfo &event_info,
                                      content::RenderWidgetHostImpl* source_rwh)
{
#if QT_CONFIG(draganddrop)
    Q_UNUSED(event_info);

    if (!m_client->supportsDragging()) {
        if (source_rwh)
            source_rwh->DragSourceSystemDragEnded();
        return;
    }

    QPixmap pixmap;
    QPoint hotspot;
    pixmap = QPixmap::fromImage(toQImage(image.GetRepresentation(1.0)));
    if (!pixmap.isNull()) {
        hotspot.setX(image_offset.x());
        hotspot.setY(image_offset.y());
    }

    m_client->startDragging(drop_data, toQtDropActions(allowed_ops), pixmap, hotspot);
#endif // QT_CONFIG(draganddrop)
}

void WebContentsViewQt::UpdateDragCursor(blink::WebDragOperation dragOperation)
{
#if QT_CONFIG(draganddrop)
    m_client->webContentsAdapter()->updateDragAction(dragOperation);
#endif // QT_CONFIG(draganddrop)
}

void WebContentsViewQt::GotFocus(content::RenderWidgetHostImpl* render_widget_host)
{
    content::WebContentsImpl *web_contents = static_cast<content::WebContentsImpl*>(m_webContents);
    web_contents->NotifyWebContentsFocused(render_widget_host);
}

void WebContentsViewQt::LostFocus(content::RenderWidgetHostImpl* render_widget_host)
{
    content::WebContentsImpl *web_contents = static_cast<content::WebContentsImpl*>(m_webContents);
    web_contents->NotifyWebContentsLostFocus(render_widget_host);
}

void WebContentsViewQt::TakeFocus(bool reverse)
{
    if (m_webContents->GetDelegate())
        m_webContents->GetDelegate()->TakeFocus(m_webContents, reverse);
}

} // namespace QtWebEngineCore
