// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef ACCESSIBILITY_ACTIVATION_OBSERVER_H
#define ACCESSIBILITY_ACTIVATION_OBSERVER_H

#include <QtGui/qaccessible.h>

namespace QtWebEngineCore {

class AccessibilityActivationObserver : public QAccessible::ActivationObserver
{
public:
    AccessibilityActivationObserver();
    ~AccessibilityActivationObserver();

    void accessibilityActiveChanged(bool active) override;
};

} // namespace QtWebEngineCore

#endif // ACCESSIBILITY_ACTIVATION_OBSERVER_H
