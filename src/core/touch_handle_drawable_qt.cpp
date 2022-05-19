// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.Chromium file.

// This implementation is based on chromium/ui/touch_selection/touch_handle_drawable_aura.cc

#include "render_widget_host_view_qt.h"
#include "touch_handle_drawable_client.h"
#include "touch_handle_drawable_qt.h"
#include "type_conversion.h"
#include "web_contents_adapter_client.h"

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

bool IsNearlyZero(float value)
{
    return std::abs(value) < kEpsilon;
}

} // namespace

TouchHandleDrawableQt::TouchHandleDrawableQt(TouchHandleDrawableDelegate *delegate)
    : m_delegate(delegate)
    , m_enabled(false)
    , m_alpha(0)
    , m_orientation(ui::TouchHandleOrientation::UNDEFINED)
{
}

TouchHandleDrawableQt::~TouchHandleDrawableQt()
{
}

void TouchHandleDrawableQt::UpdateBounds()
{
    if (!m_delegate)
        return;

    gfx::RectF newBounds = m_relativeBounds;
    newBounds.Offset(m_originPosition.x(), m_originPosition.y());
    m_delegate->setBounds(toQt(gfx::ToEnclosingRect(newBounds)));
}

bool TouchHandleDrawableQt::IsVisible() const
{
    return m_enabled && !IsNearlyZero(m_alpha);
}

void TouchHandleDrawableQt::SetEnabled(bool enabled)
{
    if (!m_delegate)
        return;

    if (enabled == m_enabled)
        return;

    m_enabled = enabled;
    m_delegate->setVisible(enabled);
}

void TouchHandleDrawableQt::SetOrientation(ui::TouchHandleOrientation orientation, bool mirror_vertical, bool mirror_horizontal)
{
    if (!m_delegate)
        return;

    // TODO: Implement adaptive handle orientation logic
    DCHECK(!mirror_vertical);
    DCHECK(!mirror_horizontal);

    if (m_orientation == orientation)
        return;
    m_orientation = orientation;
    gfx::Image* image = GetHandleImage(orientation);
    m_delegate->setImage(static_cast<int>(orientation));

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
    if (!m_delegate)
        return;

    if (alpha == m_alpha)
        return;

    m_alpha = alpha;
    m_delegate->setOpacity(m_alpha);
    m_delegate->setVisible(IsVisible());
}

gfx::RectF TouchHandleDrawableQt::GetVisibleBounds() const
{
    gfx::RectF bounds = m_relativeBounds;
    bounds.Offset(m_originPosition.x(), m_originPosition.y());

    gfx::RectF visibleBounds(bounds);
    visibleBounds.Inset(gfx::InsetsF::TLBR(
                            kSelectionHandlePadding,
                            kSelectionHandlePadding + kSelectionHandleVerticalVisualOffset,
                            kSelectionHandlePadding,
                            kSelectionHandlePadding));
    return visibleBounds;
}

float TouchHandleDrawableQt::GetDrawableHorizontalPaddingRatio() const
{
    // Qt does not have any transparent padding for its handle drawable.
    return 0.0;
}

// Returns the appropriate handle image based on the handle orientation.
gfx::Image *TouchHandleDrawableQt::GetHandleImage(ui::TouchHandleOrientation orientation)
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

} // namespace QtWebEngineCore
