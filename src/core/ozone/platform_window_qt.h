// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef PLATFORM_WINDOW_QT_H
#define PLATFORM_WINDOW_QT_H

#if defined(USE_OZONE)

#include "ui/base/cursor/platform_cursor.h"
#include "ui/events/platform/platform_event_dispatcher.h"
#include "ui/gfx/geometry/point.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/platform_window/platform_window.h"
#include "ui/platform_window/platform_window_delegate.h"

namespace ui {

class PlatformWindowQt : public PlatformWindow, public PlatformEventDispatcher
{
public:
    PlatformWindowQt(PlatformWindowDelegate* delegate, const gfx::Rect& bounds);
    ~PlatformWindowQt() override;
    // PlatformWindow:
    gfx::Rect GetBoundsInPixels() const override;
    void SetBoundsInPixels(const gfx::Rect& bounds) override;
    gfx::Rect GetBoundsInDIP() const override;
    void SetBoundsInDIP(const gfx::Rect& bounds) override;
    void Show(bool inactive = false) override { }
    void Hide() override { }
    void Close() override;
    bool IsVisible() const override { return true; }
    void SetTitle(const std::u16string&) override { }
    void SetCapture() override { }
    void ReleaseCapture() override { }
    bool HasCapture() const override { return false; }
    void SetFullscreen(bool, int64_t) override { }
    void Maximize() override { }
    void Minimize() override { }
    void Restore() override { }
    PlatformWindowState GetPlatformWindowState() const override { return PlatformWindowState::kUnknown; }
    void SetCursor(scoped_refptr<PlatformCursor>) override { }
    void MoveCursorTo(const gfx::Point&) override { }
    void ConfineCursorToBounds(const gfx::Rect&) override { }
    void SetRestoredBoundsInDIP(const gfx::Rect& bounds)  override { }
    gfx::Rect GetRestoredBoundsInDIP() const  override { return gfx::Rect(); }
    void Activate() override { }
    void Deactivate() override { }
    void SetUseNativeFrame(bool use_native_frame) override { }
    bool ShouldUseNativeFrame() const override { return false; }
    void SetWindowIcons(const gfx::ImageSkia& window_icon,
                        const gfx::ImageSkia& app_icon) override { }
    void SizeConstraintsChanged() override { }

    // PlatformEventDispatcher:
    bool CanDispatchEvent(const PlatformEvent& event) override;
    uint32_t DispatchEvent(const PlatformEvent& event) override;
    void PrepareForShutdown() override;

private:
    PlatformWindowDelegate* delegate_;
    gfx::Rect bounds_;
};

}

#endif // defined(USE_OZONE)
#endif //PLATFORM_WINDOW_QT_H
