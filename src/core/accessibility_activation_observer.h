// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef ACCESSIBILITY_ACTIVATION_OBSERVER_H
#define ACCESSIBILITY_ACTIVATION_OBSERVER_H

#include <memory>
#include <QtGui/qaccessible.h>

namespace content {
class ScopedAccessibilityMode;
}

namespace QtWebEngineCore {

class AccessibilityActivationObserver : public QAccessible::ActivationObserver
{
public:
    AccessibilityActivationObserver();
    ~AccessibilityActivationObserver();

    void accessibilityActiveChanged(bool active) override;

private:
    std::unique_ptr<content::ScopedAccessibilityMode> scoped_accessibility_mode_;
};

} // namespace QtWebEngineCore

#endif // ACCESSIBILITY_ACTIVATION_OBSERVER_H
