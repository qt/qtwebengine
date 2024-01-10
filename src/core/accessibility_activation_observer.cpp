// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "accessibility_activation_observer.h"

#include "content/browser/accessibility/browser_accessibility_state_impl.h"

namespace QtWebEngineCore {

namespace {

bool isAccessibilityEnabled() {
    // On Linux accessibility can be disabled due to performance issues by setting the
    // QTWEBENGINE_ENABLE_LINUX_ACCESSIBILITY environment variable to 0. For details,
    // see QTBUG-59922.
#ifdef Q_OS_LINUX
    static bool accessibility_enabled
            = qEnvironmentVariable("QTWEBENGINE_ENABLE_LINUX_ACCESSIBILITY", QLatin1String("1"))
                == QLatin1String("1");
#else
    const bool accessibility_enabled = true;
#endif
    return accessibility_enabled;
}

} // namespace

AccessibilityActivationObserver::AccessibilityActivationObserver()
{
    if (isAccessibilityEnabled()) {
        QAccessible::installActivationObserver(this);
        if (QAccessible::isActive())
            content::BrowserAccessibilityStateImpl::GetInstance()->EnableAccessibility();
    }
}

AccessibilityActivationObserver::~AccessibilityActivationObserver()
{
    QAccessible::removeActivationObserver(this);
}

void AccessibilityActivationObserver::accessibilityActiveChanged(bool active)
{
    if (active)
        content::BrowserAccessibilityStateImpl::GetInstance()->EnableAccessibility();
    else
        content::BrowserAccessibilityStateImpl::GetInstance()->DisableAccessibility();
}

} // namespace QtWebEngineCore
