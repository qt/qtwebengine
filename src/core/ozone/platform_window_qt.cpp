// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#if defined(USE_OZONE)

#include "base/functional/bind.h"
#include "ozone/platform_window_qt.h"
#include "ui/base/cursor/platform_cursor.h"
#include "ui/events/ozone/events_ozone.h"
#include "ui/events/platform/platform_event_source.h"
#include "ui/platform_window/platform_window_delegate.h"

namespace ui {

PlatformWindowQt::PlatformWindowQt(PlatformWindowDelegate* delegate, const gfx::Rect& bounds)
    : delegate_(delegate)
    , bounds_(bounds)
{
    ui::PlatformEventSource::GetInstance()->AddPlatformEventDispatcher(this);
}

PlatformWindowQt::~PlatformWindowQt()
{
    ui::PlatformEventSource::GetInstance()->RemovePlatformEventDispatcher(this);
}

gfx::Rect PlatformWindowQt::GetBoundsInPixels() const
{
    return bounds_;
}

void PlatformWindowQt::Close()
{
    delegate_->OnClosed();
}

void PlatformWindowQt::SetBoundsInPixels(const gfx::Rect& bounds)
{
    if (bounds == bounds_)
        return;
    const bool origin_changed = (bounds_.origin() != bounds.origin());

    bounds_ = bounds;
    delegate_->OnBoundsChanged({origin_changed});
}

bool PlatformWindowQt::CanDispatchEvent(const ui::PlatformEvent& /*ne*/)
{
    return true;
}

gfx::Rect PlatformWindowQt::GetBoundsInDIP() const
{
    return delegate_->ConvertRectToDIP(bounds_);
}

void PlatformWindowQt::SetBoundsInDIP(const gfx::Rect &bounds_in_dip)
{
    SetBoundsInPixels(delegate_->ConvertRectToPixels(bounds_in_dip));
}

uint32_t PlatformWindowQt::DispatchEvent(const ui::PlatformEvent& native_event)
{
    DispatchEventFromNativeUiEvent(
                native_event, base::BindOnce(&PlatformWindowDelegate::DispatchEvent,
                                             base::Unretained(delegate_)));

    return ui::POST_DISPATCH_STOP_PROPAGATION;
}

void PlatformWindowQt::PrepareForShutdown()
{
}

} // namespace ui

#endif // defined(USE_OZONE)
