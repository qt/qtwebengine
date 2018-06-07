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

// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.Chromium file.

// This implementation is based on chromium/ui/touch_selection/touch_handle_drawable_aura.cc

#include "render_widget_host_view_qt.h"
#include "touch_handle_drawable_client.h"
#include "touch_handle_drawable_qt.h"
#include "type_conversion.h"
#include "web_contents_adapter_client.h"

#include "ui/gfx/image/image.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/resources/grit/ui_resources.h"

namespace QtWebEngineCore {

namespace {
// The distance by which a handle image is offset from the focal point (i.e.
// text baseline) downwards.
const int kSelectionHandleVerticalVisualOffset = 2;

// The padding around the selection handle image can be used to extend the
// handle window so that touch events near the selection handle image are
// targeted to the selection handle window.
const int kSelectionHandlePadding = 0;

// Epsilon value used to compare float values to zero.
const float kEpsilon = 1e-8f;

// Returns the appropriate handle image based on the handle orientation.
gfx::Image* GetHandleImage(ui::TouchHandleOrientation orientation)
{
    int resource_id = 0;
    switch (orientation) {
    case ui::TouchHandleOrientation::LEFT:
        resource_id = IDR_TEXT_SELECTION_HANDLE_LEFT;
        break;
    case ui::TouchHandleOrientation::CENTER:
        resource_id = IDR_TEXT_SELECTION_HANDLE_CENTER;
        break;
    case ui::TouchHandleOrientation::RIGHT:
        resource_id = IDR_TEXT_SELECTION_HANDLE_RIGHT;
        break;
    case ui::TouchHandleOrientation::UNDEFINED:
        NOTREACHED() << "Invalid touch handle bound type.";
        return nullptr;
    };
    return &ui::ResourceBundle::GetSharedInstance().GetImageNamed(resource_id);
}

bool IsNearlyZero(float value)
{
    return std::abs(value) < kEpsilon;
}

} // namespace

TouchHandleDrawableQt::TouchHandleDrawableQt(RenderWidgetHostViewQt *rwhv)
    : m_rwhv(rwhv)
    , m_enabled(false)
    , m_alpha(0)
    , m_orientation(ui::TouchHandleOrientation::UNDEFINED)
{
    QMap<int, QImage> images;
    for (int orientation = 0; orientation < static_cast<int>(ui::TouchHandleOrientation::UNDEFINED); ++orientation) {
        gfx::Image* image = GetHandleImage(static_cast<ui::TouchHandleOrientation>(orientation));
        images.insert(orientation, toQImage(image->AsBitmap()));
    }

    Q_ASSERT(m_rwhv);
    Q_ASSERT(m_rwhv->adapterClient());
    m_client.reset(m_rwhv->adapterClient()->createTouchHandle(images));
}

TouchHandleDrawableQt::~TouchHandleDrawableQt()
{
}

void TouchHandleDrawableQt::UpdateBounds()
{
    if (!m_client)
        return;

    gfx::RectF newBounds = m_relativeBounds;
    newBounds.Offset(m_originPosition.x(), m_originPosition.y());
    m_client->setBounds(toQt(gfx::ToEnclosingRect(newBounds)));
}

bool TouchHandleDrawableQt::IsVisible() const
{
    return m_enabled && !IsNearlyZero(m_alpha);
}

void TouchHandleDrawableQt::SetEnabled(bool enabled)
{
    if (!m_client)
        return;

    if (enabled == m_enabled)
        return;

    m_enabled = enabled;
    m_client->setVisible(enabled);
}

void TouchHandleDrawableQt::SetOrientation(ui::TouchHandleOrientation orientation, bool mirror_vertical, bool mirror_horizontal)
{
    if (!m_client)
        return;

    // TODO: Implement adaptive handle orientation logic
    DCHECK(!mirror_vertical);
    DCHECK(!mirror_horizontal);

    if (m_orientation == orientation)
        return;
    m_orientation = orientation;
    gfx::Image* image = GetHandleImage(orientation);
    m_client->setImage(static_cast<int>(orientation));

    // Calculate the relative bounds.
    gfx::Size image_size = image->Size();
    int window_width = image_size.width() + 2 * kSelectionHandlePadding;
    int window_height = image_size.height() + 2 * kSelectionHandlePadding;
    m_relativeBounds =
            gfx::RectF(-kSelectionHandlePadding,
                       kSelectionHandleVerticalVisualOffset - kSelectionHandlePadding,
                       window_width, window_height);
    UpdateBounds();
}

void TouchHandleDrawableQt::SetOrigin(const gfx::PointF& position)
{
    m_originPosition = position;
    UpdateBounds();
}

void TouchHandleDrawableQt::SetAlpha(float alpha)
{
    if (!m_client)
        return;

    if (alpha == m_alpha)
        return;

    m_alpha = alpha;
    m_client->setOpacity(m_alpha);
    m_client->setVisible(IsVisible());
}

gfx::RectF TouchHandleDrawableQt::GetVisibleBounds() const
{
    gfx::RectF bounds = m_relativeBounds;
    bounds.Offset(m_originPosition.x(), m_originPosition.y());

    gfx::RectF visibleBounds(bounds);
    visibleBounds.Inset(kSelectionHandlePadding,
                        kSelectionHandlePadding + kSelectionHandleVerticalVisualOffset,
                        kSelectionHandlePadding,
                        kSelectionHandlePadding);
    return visibleBounds;
}

float TouchHandleDrawableQt::GetDrawableHorizontalPaddingRatio() const
{
    // Qt does not have any transparent padding for its handle drawable.
    return 0.0;
}

} // namespace QtWebEngineCore
