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

#ifndef TOUCH_HANDLE_DRAWABLE_QT_H
#define TOUCH_HANDLE_DRAWABLE_QT_H

#include "ui/touch_selection/touch_handle.h"
#include "ui/touch_selection/touch_handle_orientation.h"

#include <QtCore/QScopedPointer>

namespace QtWebEngineCore {

class RenderWidgetHostViewQt;
class TouchHandleDrawableClient;

class TouchHandleDrawableQt : public ui::TouchHandleDrawable
{
public:
    explicit TouchHandleDrawableQt(RenderWidgetHostViewQt *rwhv);
    ~TouchHandleDrawableQt() override;

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
    QScopedPointer<TouchHandleDrawableClient> m_client;

    bool m_enabled;
    float m_alpha;
    ui::TouchHandleOrientation m_orientation;
    gfx::RectF m_relativeBounds;
    gfx::PointF m_originPosition;

    DISALLOW_COPY_AND_ASSIGN(TouchHandleDrawableQt);
};

} // namespace QtWebEngineCore

#endif // TOUCH_HANDLE_DRAWABLE_QT_H
