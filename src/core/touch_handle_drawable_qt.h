// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef TOUCH_HANDLE_DRAWABLE_QT_H
#define TOUCH_HANDLE_DRAWABLE_QT_H

#include "ui/touch_selection/touch_handle.h"
#include "ui/touch_selection/touch_handle_orientation.h"
#include "ui/gfx/image/image.h"

#include <QtCore/QScopedPointer>

namespace QtWebEngineCore {

class TouchHandleDrawableDelegate;

class TouchHandleDrawableQt : public ui::TouchHandleDrawable
{
public:
    explicit TouchHandleDrawableQt(TouchHandleDrawableDelegate *delegate);
    ~TouchHandleDrawableQt() override;
    static gfx::Image *GetHandleImage(ui::TouchHandleOrientation orientation);

private:
    void UpdateBounds();
    bool IsVisible() const;

    // ui::TouchHandleDrawable overrides
    void SetEnabled(bool enabled) override;
    void SetOrientation(ui::TouchHandleOrientation orientation,
                        bool mirror_vertical,
                        bool mirror_horizontal) override;
    void SetOrigin(const gfx::PointF& position) override;
    void SetAlpha(float alpha) override;
    gfx::RectF GetVisibleBounds() const override;
    float GetDrawableHorizontalPaddingRatio() const override;

    RenderWidgetHostViewQt *m_rwhv;
    QScopedPointer<TouchHandleDrawableDelegate> m_delegate;

    bool m_enabled;
    float m_alpha;
    ui::TouchHandleOrientation m_orientation;
    gfx::RectF m_relativeBounds;
    gfx::PointF m_originPosition;
};

} // namespace QtWebEngineCore

#endif // TOUCH_HANDLE_DRAWABLE_QT_H
