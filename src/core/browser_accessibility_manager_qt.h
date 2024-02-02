// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef BROWSER_ACCESSIBILITY_MANAGER_QT_H
#define BROWSER_ACCESSIBILITY_MANAGER_QT_H

#include "content/browser/accessibility/browser_accessibility_manager.h"

#include <QtCore/qtclasshelpermacros.h>
#include <QtCore/qtconfigmacros.h>

QT_FORWARD_DECLARE_CLASS(QAccessibleInterface)

namespace QtWebEngineCore {
class WebContentsAccessibilityQt;
}

namespace content {

class BrowserAccessibilityManagerQt : public BrowserAccessibilityManager
{
public:
    BrowserAccessibilityManagerQt(QtWebEngineCore::WebContentsAccessibilityQt *webContentsAccessibility,
                                  const ui::AXTreeUpdate &initialTree,
                                  WebAXPlatformTreeManagerDelegate *delegate);
    ~BrowserAccessibilityManagerQt() override;
    void FireBlinkEvent(ax::mojom::Event event_type,
                        BrowserAccessibility *node,
                        int action_request_id) override;
    void FireGeneratedEvent(ui::AXEventGenerator::Event event_type,
                            const ui::AXNode *node) override;

    QAccessibleInterface *rootParentAccessible();
    bool isValid() const { return m_valid; }

private:
    Q_DISABLE_COPY(BrowserAccessibilityManagerQt)
    QtWebEngineCore::WebContentsAccessibilityQt *m_webContentsAccessibility;
    bool m_valid = false;
};

}

#endif // BROWSER_ACCESSIBILITY_MANAGER_QT_H
