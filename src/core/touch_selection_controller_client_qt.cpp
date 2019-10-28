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

#include "render_widget_host_view_qt.h"
#include "touch_handle_drawable_qt.h"
#include "touch_selection_controller_client_qt.h"
#include "touch_selection_menu_controller.h"
#include "type_conversion.h"
#include "web_contents_adapter.h"
#include "web_contents_adapter_client.h"

#include "content/browser/frame_host/render_frame_host_impl.h"
#include "content/browser/renderer_host/render_widget_host_impl.h"
#include "ui/gfx/geometry/size_conversions.h"

#include <QClipboard>
#include <QGuiApplication>

namespace QtWebEngineCore {

TouchSelectionControllerClientQt::TouchSelectionControllerClientQt(RenderWidgetHostViewQt *rwhv)
    : m_rwhv(rwhv)
    , m_menuController(new TouchSelectionMenuController(this))
    , m_menuShowing(false)
    , m_menuRequested(false)
    , m_touchDown(false)
    , m_scrollInProgress(false)
    , m_handleDragInProgress(false)
{
    Q_ASSERT(rwhv);
}

TouchSelectionControllerClientQt::~TouchSelectionControllerClientQt()
{
}

bool TouchSelectionControllerClientQt::handleContextMenu(const content::ContextMenuParams& params)
{
    if ((params.source_type == ui::MENU_SOURCE_LONG_PRESS ||
         params.source_type == ui::MENU_SOURCE_LONG_TAP) &&
        params.is_editable && params.selection_text.empty()) {
        m_menuRequested = true;
        updateMenu();
        return true;
    }

    const bool from_touch = params.source_type == ui::MENU_SOURCE_LONG_PRESS ||
                            params.source_type == ui::MENU_SOURCE_LONG_TAP ||
                            params.source_type == ui::MENU_SOURCE_TOUCH;
    if (from_touch && !params.selection_text.empty())
        return true;

    GetTouchSelectionController()->HideAndDisallowShowingAutomatically();
    return false;
}

void TouchSelectionControllerClientQt::onTouchDown()
{
    m_touchDown = true;
    updateMenu();
}

void TouchSelectionControllerClientQt::onTouchUp()
{
    m_touchDown = false;
    updateMenu();
}

void TouchSelectionControllerClientQt::onScrollBegin()
{
    m_scrollInProgress = true;
    GetTouchSelectionController()->SetTemporarilyHidden(true);
    updateMenu();
}

void TouchSelectionControllerClientQt::onScrollEnd()
{
    m_scrollInProgress = false;
    GetTouchSelectionController()->SetTemporarilyHidden(false);
    updateMenu();
}

bool TouchSelectionControllerClientQt::IsCommandIdEnabled(int command_id) const
{
    bool editable = m_rwhv->getTextInputType() != ui::TEXT_INPUT_TYPE_NONE;
    bool readable = m_rwhv->getTextInputType() != ui::TEXT_INPUT_TYPE_PASSWORD;
    bool hasSelection = !m_rwhv->GetSelectedText().empty();

    switch (command_id) {
    case TouchSelectionMenuController::Cut:
        return editable && readable && hasSelection;
    case TouchSelectionMenuController::Copy:
        return readable && hasSelection;
    case TouchSelectionMenuController::Paste:
        return editable && !QGuiApplication::clipboard()->text().isEmpty();
    default:
        return false;
    }
}

void TouchSelectionControllerClientQt::ExecuteCommand(int command_id, int event_flags)
{
    Q_UNUSED(event_flags);
    GetTouchSelectionController()->HideAndDisallowShowingAutomatically();

    WebContentsAdapterClient *adapterClient = m_rwhv->adapterClient();
    Q_ASSERT(adapterClient);
    WebContentsAdapter *adapter = adapterClient->webContentsAdapter();
    Q_ASSERT(adapter);

    switch (command_id) {
    case TouchSelectionMenuController::Cut:
        adapter->cut();
        break;
    case TouchSelectionMenuController::Copy:
        adapter->copy();
        break;
    case TouchSelectionMenuController::Paste:
        adapter->paste();
        break;
    default:
        NOTREACHED();
        break;
    }
}

void TouchSelectionControllerClientQt::RunContextMenu()
{
    gfx::RectF anchorRect = GetTouchSelectionController()->GetRectBetweenBounds();
    gfx::PointF anchorPoint = gfx::PointF(anchorRect.CenterPoint().x(), anchorRect.y());

    content::RenderWidgetHostImpl *host = m_rwhv->host();
    host->ShowContextMenuAtPoint(gfx::ToRoundedPoint(anchorPoint),
                                 ui::MENU_SOURCE_TOUCH_EDIT_MENU);

    // Hide selection handles after getting rect-between-bounds from touch
    // selection controller; otherwise, rect would be empty and the above
    // calculations would be invalid.
    GetTouchSelectionController()->HideAndDisallowShowingAutomatically();
}

void TouchSelectionControllerClientQt::DidStopFlinging()
{
    onScrollEnd();
}

void TouchSelectionControllerClientQt::UpdateClientSelectionBounds(const gfx::SelectionBound& start,
                                                                   const gfx::SelectionBound& end)
{
    UpdateClientSelectionBounds(start, end, this, this);
}

void TouchSelectionControllerClientQt::UpdateClientSelectionBounds(const gfx::SelectionBound& start,
                                                                   const gfx::SelectionBound& end,
                                                                   ui::TouchSelectionControllerClient* client,
                                                                   ui::TouchSelectionMenuClient* menu_client)
{
    Q_UNUSED(client);
    Q_UNUSED(menu_client);

    GetTouchSelectionController()->OnSelectionBoundsChanged(start, end);
}

void TouchSelectionControllerClientQt::InvalidateClient(ui::TouchSelectionControllerClient* client)
{
    Q_UNUSED(client);
}

ui::TouchSelectionController* TouchSelectionControllerClientQt::GetTouchSelectionController()
{
    return m_rwhv->getTouchSelectionController();
}

void TouchSelectionControllerClientQt::AddObserver(Observer* observer)
{
    Q_UNUSED(observer);
}

void TouchSelectionControllerClientQt::RemoveObserver(Observer* observer)
{
    Q_UNUSED(observer);
}

bool TouchSelectionControllerClientQt::SupportsAnimation() const
{
    return false;
}

void TouchSelectionControllerClientQt::SetNeedsAnimate()
{
    NOTREACHED();
}

void TouchSelectionControllerClientQt::MoveCaret(const gfx::PointF& position)
{
    content::mojom::FrameInputHandler *frameInputHandler = m_rwhv->getFrameInputHandler();
    if (!frameInputHandler)
        return;

    frameInputHandler->MoveCaret(gfx::ToRoundedPoint(position));
}

void TouchSelectionControllerClientQt::MoveRangeSelectionExtent(const gfx::PointF& extent)
{
    content::mojom::FrameInputHandler *frameInputHandler = m_rwhv->getFrameInputHandler();
    if (!frameInputHandler)
        return;

    frameInputHandler->MoveRangeSelectionExtent(gfx::ToRoundedPoint(extent));
}

void TouchSelectionControllerClientQt::SelectBetweenCoordinates(const gfx::PointF& base, const gfx::PointF& extent)
{
    content::mojom::FrameInputHandler *frameInputHandler = m_rwhv->getFrameInputHandler();
    if (!frameInputHandler)
        return;

    frameInputHandler->SelectRange(gfx::ToRoundedPoint(base), gfx::ToRoundedPoint(extent));
}

void TouchSelectionControllerClientQt::OnSelectionEvent(ui::SelectionEventType event)
{
    switch (event) {
    case ui::SELECTION_HANDLES_SHOWN:
        m_menuRequested = true;
        break;
    case ui::INSERTION_HANDLE_SHOWN:
        break;
    case ui::SELECTION_HANDLES_CLEARED:
    case ui::INSERTION_HANDLE_CLEARED:
        m_menuRequested = false;
        break;
    case ui::SELECTION_HANDLE_DRAG_STARTED:
    case ui::INSERTION_HANDLE_DRAG_STARTED:
        m_handleDragInProgress = true;
        break;
    case ui::SELECTION_HANDLE_DRAG_STOPPED:
    case ui::INSERTION_HANDLE_DRAG_STOPPED:
        m_handleDragInProgress = false;
        break;
    case ui::SELECTION_HANDLES_MOVED:
    case ui::INSERTION_HANDLE_MOVED:
        break;
    case ui::INSERTION_HANDLE_TAPPED:
        m_menuRequested = !m_menuRequested;
        break;
    }

    updateMenu();
}

void TouchSelectionControllerClientQt::OnDragUpdate(const gfx::PointF& position)
{
    Q_UNUSED(position);
}

std::unique_ptr<ui::TouchHandleDrawable> TouchSelectionControllerClientQt::CreateDrawable()
{
    return std::unique_ptr<ui::TouchHandleDrawable>(new TouchHandleDrawableQt(m_rwhv));
}

void TouchSelectionControllerClientQt::DidScroll()
{
}

void TouchSelectionControllerClientQt::showMenu()
{
    gfx::RectF rect = GetTouchSelectionController()->GetRectBetweenBounds();
    gfx::PointF origin = rect.origin();
    gfx::PointF bottom_right = rect.bottom_right();

    gfx::Vector2dF diagonal = bottom_right - origin;
    gfx::SizeF size(diagonal.x(), diagonal.y());
    gfx::RectF anchor_rect(origin, size);

    // Calculate maximum handle image size;
    gfx::SizeF max_handle_size = GetTouchSelectionController()->GetStartHandleRect().size();
    max_handle_size.SetToMax(GetTouchSelectionController()->GetEndHandleRect().size());

    WebContentsAdapterClient *adapterClient = m_rwhv->adapterClient();
    Q_ASSERT(adapterClient);
    adapterClient->showTouchSelectionMenu(m_menuController.data(),
                                          QRect(toQt(gfx::ToEnclosingRect(anchor_rect))),
                                          QSize(toQt(gfx::ToRoundedSize(max_handle_size))));
    m_menuShowing = true;
}

void TouchSelectionControllerClientQt::hideMenu()
{
    WebContentsAdapterClient *adapterClient = m_rwhv->adapterClient();
    Q_ASSERT(adapterClient);
    adapterClient->hideTouchSelectionMenu();
    m_menuShowing = false;
}

void TouchSelectionControllerClientQt::updateMenu()
{
    if (m_menuShowing)
        hideMenu();

    if (m_menuRequested && !m_touchDown &&
            !m_scrollInProgress && !m_handleDragInProgress) {
        showMenu();
    }
}

} // namespace QtWebEngineCore
