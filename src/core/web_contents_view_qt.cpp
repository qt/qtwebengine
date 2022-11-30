// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "web_contents_view_qt.h"

#include "render_widget_host_view_qt.h"
#include "render_widget_host_view_qt_delegate.h"
#include "touch_selection_controller_client_qt.h"
#include "type_conversion.h"
#include "web_contents_adapter.h"
#include "web_contents_adapter_client.h"

#include "content/browser/web_contents/web_contents_impl.h"
#include "content/public/browser/context_menu_params.h"
#include "content/public/browser/web_contents_delegate.h"
#include "third_party/blink/public/common/context_menu_data/context_menu_data.h"
#include "third_party/blink/public/common/context_menu_data/edit_flags.h"
#include "third_party/blink/public/mojom/context_menu/context_menu.mojom.h"

#include "ui/gfx/image/image_skia.h"

#include <QtGui/QPixmap>
#include <QtWebEngineCore/private/qwebenginecontextmenurequest_p.h>

namespace QtWebEngineCore {

WebContentsViewQt::WebContentsViewQt(content::WebContents *webContents)
    : m_webContents(webContents)
    , m_client(nullptr)
    , m_factoryClient(nullptr)
    , m_contextMenuRequest(
        new QWebEngineContextMenuRequest(new QWebEngineContextMenuRequestPrivate()))
{
}

void WebContentsViewQt::setFactoryClient(WebContentsAdapterClient* client)
{
    if (m_factoryClient)
        return;
    m_factoryClient = client;

    // Check if a RWHV was created before the pre-initialization.
    if (auto view = static_cast<RenderWidgetHostViewQt *>(m_webContents->GetRenderWidgetHostView())) {
        view->setDelegate(m_factoryClient->CreateRenderWidgetHostViewQtDelegate(view->delegateClient()));
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
        view->setDelegate(m_factoryClient->CreateRenderWidgetHostViewQtDelegate(view->delegateClient()));
        if (m_client)
            view->setAdapterClient(m_client);
    }

    return view;
}

content::RenderWidgetHostViewBase* WebContentsViewQt::CreateViewForChildWidget(content::RenderWidgetHost* render_widget_host)
{
    RenderWidgetHostViewQt *view = new RenderWidgetHostViewQt(render_widget_host);

    Q_ASSERT(m_client);
    view->setDelegate(m_client->CreateRenderWidgetHostViewQtDelegateForPopup(view->delegateClient()));
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

gfx::Rect WebContentsViewQt::GetContainerBounds() const
{
    if (m_client) {
        const QRectF r(m_client->viewportRect());
        return gfx::Rect(r.x(), r.y(), r.width(), r.height());
    }
    return gfx::Rect();
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
    web_contents->GetRenderViewHost()->SetInitialFocus(reverse);
}

ASSERT_ENUMS_MATCH(QWebEngineContextMenuRequest::MediaTypeNone,
                   blink::mojom::ContextMenuDataMediaType::kNone)
ASSERT_ENUMS_MATCH(QWebEngineContextMenuRequest::MediaTypeImage,
                   blink::mojom::ContextMenuDataMediaType::kImage)
ASSERT_ENUMS_MATCH(QWebEngineContextMenuRequest::MediaTypeVideo,
                   blink::mojom::ContextMenuDataMediaType::kVideo)
ASSERT_ENUMS_MATCH(QWebEngineContextMenuRequest::MediaTypeAudio,
                   blink::mojom::ContextMenuDataMediaType::kAudio)
ASSERT_ENUMS_MATCH(QWebEngineContextMenuRequest::MediaTypeCanvas,
                   blink::mojom::ContextMenuDataMediaType::kCanvas)
ASSERT_ENUMS_MATCH(QWebEngineContextMenuRequest::MediaTypeFile,
                   blink::mojom::ContextMenuDataMediaType::kFile)
ASSERT_ENUMS_MATCH(QWebEngineContextMenuRequest::MediaTypePlugin,
                   blink::mojom::ContextMenuDataMediaType::kPlugin)

ASSERT_ENUMS_MATCH(QWebEngineContextMenuRequest::MediaInError,
                   blink::ContextMenuData::kMediaInError)
ASSERT_ENUMS_MATCH(QWebEngineContextMenuRequest::MediaPaused,
                   blink::ContextMenuData::kMediaPaused)
ASSERT_ENUMS_MATCH(QWebEngineContextMenuRequest::MediaMuted, blink::ContextMenuData::kMediaMuted)
ASSERT_ENUMS_MATCH(QWebEngineContextMenuRequest::MediaLoop, blink::ContextMenuData::kMediaLoop)
ASSERT_ENUMS_MATCH(QWebEngineContextMenuRequest::MediaCanSave,
                   blink::ContextMenuData::kMediaCanSave)
ASSERT_ENUMS_MATCH(QWebEngineContextMenuRequest::MediaHasAudio,
                   blink::ContextMenuData::kMediaHasAudio)
ASSERT_ENUMS_MATCH(QWebEngineContextMenuRequest::MediaCanToggleControls,
                   blink::ContextMenuData::kMediaCanToggleControls)
ASSERT_ENUMS_MATCH(QWebEngineContextMenuRequest::MediaControls,
                   blink::ContextMenuData::kMediaControls)
ASSERT_ENUMS_MATCH(QWebEngineContextMenuRequest::MediaCanPrint,
                   blink::ContextMenuData::kMediaCanPrint)
ASSERT_ENUMS_MATCH(QWebEngineContextMenuRequest::MediaCanRotate,
                   blink::ContextMenuData::kMediaCanRotate)

ASSERT_ENUMS_MATCH(QWebEngineContextMenuRequest::CanUndo, blink::kCanUndo)
ASSERT_ENUMS_MATCH(QWebEngineContextMenuRequest::CanRedo, blink::kCanRedo)
ASSERT_ENUMS_MATCH(QWebEngineContextMenuRequest::CanCut, blink::kCanCut)
ASSERT_ENUMS_MATCH(QWebEngineContextMenuRequest::CanCopy, blink::kCanCopy)
ASSERT_ENUMS_MATCH(QWebEngineContextMenuRequest::CanPaste, blink::kCanPaste)
ASSERT_ENUMS_MATCH(QWebEngineContextMenuRequest::CanDelete, blink::kCanDelete)
ASSERT_ENUMS_MATCH(QWebEngineContextMenuRequest::CanSelectAll, blink::kCanSelectAll)
ASSERT_ENUMS_MATCH(QWebEngineContextMenuRequest::CanTranslate, blink::kCanTranslate)
ASSERT_ENUMS_MATCH(QWebEngineContextMenuRequest::CanEditRichly, blink::kCanEditRichly)

// static
void WebContentsViewQt::update(QWebEngineContextMenuRequest *request,
                               const content::ContextMenuParams &params, bool spellcheckEnabled)
{
    auto *re = request->d.data();
    re->m_position = QPoint(params.x, params.y);
    re->m_filteredLinkUrl = toQt(params.link_url);
    re->m_linkText = toQt(params.link_text);
    re->m_altText = toQt(params.alt_text);
    re->m_titleText = toQt(params.title_text);
    re->m_unfilteredLinkUrl = toQt(params.unfiltered_link_url);
    re->m_selectedText = toQt(params.selection_text);
    re->m_mediaUrl = toQt(params.src_url);
    re->m_mediaType = (QWebEngineContextMenuRequest::MediaType)params.media_type;
    re->m_hasImageContent = params.has_image_contents;
    re->m_mediaFlags = (QWebEngineContextMenuRequest::MediaFlags)params.media_flags;
    re->m_editFlags = (QWebEngineContextMenuRequest::EditFlags)params.edit_flags;
    re->m_suggestedFileName = toQt(params.suggested_filename);
    re->m_isEditable = params.is_editable;
#if QT_CONFIG(webengine_spellchecker)
    re->m_misspelledWord = toQt(params.misspelled_word);
    re->m_spellCheckerSuggestions = fromVector(params.dictionary_suggestions);
#endif
    re->m_frameUrl = toQt(params.frame_url);
    re->m_pageUrl = toQt(params.page_url);
    re->m_referrerPolicy = (ReferrerPolicy)params.referrer_policy;
#if QT_CONFIG(webengine_spellchecker)
    // Do not use params.spellcheck_enabled, since it is never
    // correctly initialized for chrome asynchronous spellchecking.
    // Even fixing the initialization in ContextMenuClientImpl::showContextMenu
    // will not work. By default SpellCheck::spellcheck_enabled_
    // must be initialized to true due to the way how the initialization sequence
    // in SpellCheck works ie. typing the first word triggers the creation
    // of the SpellcheckService. Use user preference store instead.
    re->m_isSpellCheckerEnabled = spellcheckEnabled;
#endif
}

void WebContentsViewQt::ShowContextMenu(content::RenderFrameHost &,
                                        const content::ContextMenuParams &params)
{
    if (auto rwhv =
                static_cast<RenderWidgetHostViewQt *>(m_webContents->GetRenderWidgetHostView())) {
        if (rwhv && rwhv->getTouchSelectionControllerClient()->handleContextMenu(params))
            return;
    }
    const bool spellcheckEnabled = m_client->profileAdapter()->isSpellCheckEnabled();
    update(m_contextMenuRequest.get(), params, spellcheckEnabled);
    m_client->contextMenuRequested(m_contextMenuRequest.get());
}

static Qt::DropActions toQtDropActions(blink::DragOperationsMask ops)
{
    Qt::DropActions result;
    if (ops & blink::kDragOperationCopy)
        result |= Qt::CopyAction;
    if (ops & blink::kDragOperationLink)
        result |= Qt::LinkAction;
    if (ops & blink::kDragOperationMove)
        result |= Qt::MoveAction;
    return result;
}

void WebContentsViewQt::StartDragging(const content::DropData &drop_data,
                                      blink::DragOperationsMask allowed_ops,
                                      const gfx::ImageSkia &image,
                                      const gfx::Vector2d &image_offset,
                                      const gfx::Rect &drag_obj_rect,
                                      const blink::mojom::DragEventSourceInfo &event_info,
                                      content::RenderWidgetHostImpl *source_rwh)
{
#if QT_CONFIG(draganddrop)
    Q_UNUSED(event_info);

    QObject *dragSource = m_client->dragSource();
    if (!dragSource) {
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

    m_client->webContentsAdapter()->startDragging(dragSource, drop_data, toQtDropActions(allowed_ops), pixmap, hotspot);
#endif // QT_CONFIG(draganddrop)
}

void WebContentsViewQt::UpdateDragCursor(ui::mojom::DragOperation dragOperation)
{
#if QT_CONFIG(draganddrop)
    m_client->webContentsAdapter()->updateDragAction(int(dragOperation));
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
