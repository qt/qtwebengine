// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "accessibility_activation_observer.h"

#if QT_CONFIG(accessibility)

#include "content/browser/accessibility/browser_accessibility_state_impl.h"

namespace QtWebEngineCore {

namespace {

bool isAccessibilityEnabled() {
    // On Linux accessibility is disabled by default due to performance issues,
    // and can be re-enabled by setting the QTWEBENGINE_ENABLE_LINUX_ACCESSIBILITY environment
    // variable. For details, see QTBUG-59922.
#ifdef Q_OS_LINUX
    static bool accessibility_enabled
            = qEnvironmentVariableIsSet("QTWEBENGINE_ENABLE_LINUX_ACCESSIBILITY");
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

#endif // QT_CONFIG(accessibility)
