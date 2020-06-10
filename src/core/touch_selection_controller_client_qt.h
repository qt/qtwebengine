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
    base::string16 GetSelectedText() override { return base::string16(); }

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
    void OnDragUpdate(const gfx::PointF& position) override;
    std::unique_ptr<ui::TouchHandleDrawable> CreateDrawable() override;
    void DidScroll() override;

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
