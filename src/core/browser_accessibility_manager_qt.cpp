// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qtwebenginecoreglobal.h"

#include "content/browser/accessibility/browser_accessibility_manager.h"

#include <QtGui/qtguiglobal.h>

#if QT_CONFIG(accessibility)
#include "browser_accessibility_qt.h"
#include "browser_accessibility_manager_qt.h"
#include "render_widget_host_view_qt.h" // WebContentsAccessibilityQt

#include "content/browser/accessibility/browser_accessibility.h"
#include "ui/accessibility/ax_enums.mojom.h"

#include <QtGui/qaccessible.h>
#endif // QT_CONFIG(accessibility)

namespace content {

// static
BrowserAccessibilityManager *BrowserAccessibilityManager::Create(
        const ui::AXTreeUpdate &initialTree,
        WebAXPlatformTreeManagerDelegate *delegate)
{
#if QT_CONFIG(accessibility)
    Q_ASSERT(delegate);
    QtWebEngineCore::WebContentsAccessibilityQt *access = nullptr;
    access = static_cast<QtWebEngineCore::WebContentsAccessibilityQt *>(delegate->AccessibilityGetWebContentsAccessibility());

    // Accessibility is not supported for guest views and child frames.
    if (!access) {
        return nullptr;
    }

    return new BrowserAccessibilityManagerQt(access, initialTree, delegate);
#else
    Q_UNUSED(initialTree);
    Q_UNUSED(delegate);
    return nullptr;
#endif // QT_CONFIG(accessibility)
}

// static
BrowserAccessibilityManager *BrowserAccessibilityManager::Create(
        WebAXPlatformTreeManagerDelegate *delegate)
{
#if QT_CONFIG(accessibility)
    return BrowserAccessibilityManager::Create(BrowserAccessibilityManagerQt::GetEmptyDocument(), delegate);
#else
    Q_UNUSED(delegate);
    return nullptr;
#endif
}

#if QT_CONFIG(accessibility)
BrowserAccessibilityManagerQt::BrowserAccessibilityManagerQt(
    QtWebEngineCore::WebContentsAccessibilityQt *webContentsAccessibility,
    const ui::AXTreeUpdate &initialTree,
    WebAXPlatformTreeManagerDelegate* delegate)
      : BrowserAccessibilityManager(delegate)
      , m_webContentsAccessibility(webContentsAccessibility)
{
    Initialize(initialTree);
    m_valid = true; // BrowserAccessibilityQt can start using the AXTree
}

BrowserAccessibilityManagerQt::~BrowserAccessibilityManagerQt()
{
    m_valid = false; // BrowserAccessibilityQt should stop using the AXTree
}

QAccessibleInterface *BrowserAccessibilityManagerQt::rootParentAccessible()
{
    content::BrowserAccessibility *parent_node = GetParentNodeFromParentTreeAsBrowserAccessibility();
    if (!parent_node) {
        Q_ASSERT(m_webContentsAccessibility);
        return QAccessible::queryAccessibleInterface(m_webContentsAccessibility->accessibilityParentObject());
    }

    auto *parent_manager =
            static_cast<BrowserAccessibilityManagerQt *>(parent_node->manager());
    return parent_manager->rootParentAccessible();
}

void BrowserAccessibilityManagerQt::FireBlinkEvent(ax::mojom::Event event_type,
                                                   BrowserAccessibility *node,
                                                   int action_request_id)
{
    auto *iface = toQAccessibleInterface(node);

    switch (event_type) {
    case ax::mojom::Event::kFocus: {
        QAccessibleEvent event(iface, QAccessible::Focus);
        if (event.object())
            event.setChild(-1);
        QAccessible::updateAccessibility(&event);
        break;
    }
    case ax::mojom::Event::kCheckedStateChanged: {
        QAccessible::State change;
        change.checked = true;
        QAccessibleStateChangeEvent event(iface, change);
        if (event.object())
            event.setChild(-1);
        QAccessible::updateAccessibility(&event);
        break;
    }
    case ax::mojom::Event::kValueChanged: {
        QVariant value;
        if (QAccessibleValueInterface *valueIface = iface->valueInterface())
            value = valueIface->currentValue();
        QAccessibleValueChangeEvent event(iface, value);
        if (event.object())
            event.setChild(-1);
        QAccessible::updateAccessibility(&event);
        break;
    }
    case ax::mojom::Event::kChildrenChanged:
        break;
    case ax::mojom::Event::kLayoutComplete:
        break;
    case ax::mojom::Event::kLoadComplete:
        break;
    case ax::mojom::Event::kTextChanged: {
        QAccessibleTextUpdateEvent event(iface, -1, QString(), QString());
        if (event.object())
            event.setChild(-1);
        QAccessible::updateAccessibility(&event);
        break;
    }
    case ax::mojom::Event::kTextSelectionChanged: {
        QAccessibleTextInterface *textIface = iface->textInterface();
        if (textIface) {
            int start = 0;
            int end = 0;
            textIface->selection(0, &start, &end);
            if (start == end) {
                QAccessibleTextCursorEvent event(iface, start);
                if (event.object())
                    event.setChild(-1);
                QAccessible::updateAccessibility(&event);
            } else {
                QAccessibleTextSelectionEvent event(iface, start, end);
                if (event.object())
                    event.setChild(-1);
                QAccessible::updateAccessibility(&event);
            }
        }
        break;
    }
    default:
        break;
    }
}

void BrowserAccessibilityManagerQt::FireGeneratedEvent(ui::AXEventGenerator::Event event_type,
                                                       const ui::AXNode *node)
{
    BrowserAccessibilityManager::FireGeneratedEvent(event_type, node);

    BrowserAccessibility *wrapper = GetFromAXNode(node);
    DCHECK(wrapper);
    auto *iface = toQAccessibleInterface(wrapper);

    switch (event_type) {
    case ui::AXEventGenerator::Event::VALUE_IN_TEXT_FIELD_CHANGED:
        if (iface->role() == QAccessible::EditableText) {
            QAccessibleTextUpdateEvent event(iface, -1, QString(), QString());
            if (event.object())
                event.setChild(-1);
            QAccessible::updateAccessibility(&event);
        }
        break;
    default:
        break;
    }
}

#endif // QT_CONFIG(accessibility)

}
