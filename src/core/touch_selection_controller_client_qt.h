// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef TOUCH_SELECTION_CONTROLLER_CLIENT_QT_H
#define TOUCH_SELECTION_CONTROLLER_CLIENT_QT_H

#include "content/public/browser/context_menu_params.h"
#include "content/public/browser/touch_selection_controller_client_manager.h"
#include "ui/touch_selection/touch_selection_controller.h"
#include "ui/touch_selection/touch_selection_menu_runner.h"

#include <QtCore/QScopedPointer>

namespace QtWebEngineCore {

class RenderWidgetHostViewQt;
class TouchSelectionMenuController;
class TouchHandleDrawableDelegate;

class TouchSelectionControllerClientQt
    : public ui::TouchSelectionControllerClient
    , public ui::TouchSelectionMenuClient
    , public content::TouchSelectionControllerClientManager
{
public:
    explicit TouchSelectionControllerClientQt(RenderWidgetHostViewQt *rwhv);
    ~TouchSelectionControllerClientQt() override;

    void UpdateClientSelectionBounds(const gfx::SelectionBound& start,
                                     const gfx::SelectionBound& end);
    bool handleContextMenu(const content::ContextMenuParams& params);
    void onTouchDown();
    void onTouchUp();
    void onScrollBegin();
    void onScrollEnd();

    // ui::TouchSelectionMenuClient overrides
    bool IsCommandIdEnabled(int command_id) const override;
    void ExecuteCommand(int command_id, int event_flags) override;
    void RunContextMenu() override;
    bool ShouldShowQuickMenu() override { return false; }
    std::u16string GetSelectedText() override { return std::u16string(); }

    // content::TouchSelectionControllerClientManager overrides
    void DidStopFlinging() override;
    void UpdateClientSelectionBounds(const gfx::SelectionBound& start,
                                     const gfx::SelectionBound& end,
                                     ui::TouchSelectionControllerClient* client,
                                     ui::TouchSelectionMenuClient* menu_client) override;
    void InvalidateClient(ui::TouchSelectionControllerClient* client) override;
    ui::TouchSelectionController* GetTouchSelectionController() override;
    void AddObserver(Observer* observer) override;
    void RemoveObserver(Observer* observer) override;

    // ui::TouchSelectionControllerClient overrides
    bool SupportsAnimation() const override;
    void SetNeedsAnimate() override;
    void MoveCaret(const gfx::PointF& position) override;
    void MoveRangeSelectionExtent(const gfx::PointF& extent) override;
    void SelectBetweenCoordinates(const gfx::PointF& base, const gfx::PointF& extent) override;
    void OnSelectionEvent(ui::SelectionEventType event) override;
    void OnDragUpdate(const ui::TouchSelectionDraggable::Type type, const gfx::PointF& position) override;
    std::unique_ptr<ui::TouchHandleDrawable> CreateDrawable() override;
    void DidScroll() override;

    void resetControls();

private:
    void showMenu();
    void hideMenu();
    void updateMenu();

    RenderWidgetHostViewQt *m_rwhv;
    QScopedPointer<TouchSelectionMenuController> m_menuController;

    bool m_menuShowing;
    bool m_menuRequested;
    bool m_touchDown;
    bool m_scrollInProgress;
    bool m_handleDragInProgress;
};

} // namespace QtWebEngineCore

#endif // TOUCH_SELECTION_CONTROLLER_CLIENT_QT_H
