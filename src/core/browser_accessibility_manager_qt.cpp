// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "browser_accessibility_manager_qt.h"
#include "qtwebenginecoreglobal_p.h"

#include "content/browser/accessibility/browser_accessibility.h"
#include "ui/accessibility/ax_enums.mojom.h"

#if QT_CONFIG(webengine_extensions)
#include "content/browser/renderer_host/render_frame_host_impl.h"
#include "content/public/browser/web_contents.h"
#endif // QT_CONFIG(webengine_extensions)

#include "browser_accessibility_qt.h"
#include "render_widget_host_view_qt.h"

#include <QtGui/qaccessible.h>

using namespace blink;

namespace content {

// static
BrowserAccessibilityManager *BrowserAccessibilityManager::Create(
        const ui::AXTreeUpdate &initialTree,
        BrowserAccessibilityDelegate *delegate)
{
#if QT_CONFIG(accessibility)
    Q_ASSERT(delegate);
    QtWebEngineCore::WebContentsAccessibilityQt *access = nullptr;
    access = static_cast<QtWebEngineCore::WebContentsAccessibilityQt *>(delegate->AccessibilityGetWebContentsAccessibility());

#if QT_CONFIG(webengine_extensions)
    // Accessibility is not supported for guest views.
    if (!access) {
        Q_ASSERT(content::WebContents::FromRenderFrameHost(
                         static_cast<content::RenderFrameHostImpl *>(delegate))
                         ->GetOuterWebContents());
        return nullptr;
    }
#endif // QT_CONFIG(webengine_extensions)

    return new BrowserAccessibilityManagerQt(access, initialTree, delegate);
#else
    return nullptr;
#endif // QT_CONFIG(accessibility)
}

// static
BrowserAccessibilityManager *BrowserAccessibilityManager::Create(
        BrowserAccessibilityDelegate *delegate)
{
#if QT_CONFIG(accessibility)
    return BrowserAccessibilityManager::Create(BrowserAccessibilityManagerQt::GetEmptyDocument(), delegate);
#else
    return nullptr;
#endif
}

#if QT_CONFIG(accessibility)
BrowserAccessibilityManagerQt::BrowserAccessibilityManagerQt(
    QtWebEngineCore::WebContentsAccessibilityQt *webContentsAccessibility,
    const ui::AXTreeUpdate &initialTree,
    BrowserAccessibilityDelegate* delegate)
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
    content::BrowserAccessibility *parent_node = GetParentNodeFromParentTree();
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
        QAccessible::updateAccessibility(&event);
        break;
    }
    case ax::mojom::Event::kCheckedStateChanged: {
        QAccessible::State change;
        change.checked = true;
        QAccessibleStateChangeEvent event(iface, change);
        QAccessible::updateAccessibility(&event);
        break;
    }
    case ax::mojom::Event::kValueChanged: {
        QVariant value;
        if (QAccessibleValueInterface *valueIface = iface->valueInterface())
            value = valueIface->currentValue();
        QAccessibleValueChangeEvent event(iface, value);
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
                QAccessible::updateAccessibility(&event);
            } else {
                QAccessibleTextSelectionEvent event(iface, start, end);
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
                                                       BrowserAccessibility* node)
{
    auto *iface = toQAccessibleInterface(node);

    switch (event_type) {
    case ui::AXEventGenerator::Event::VALUE_IN_TEXT_FIELD_CHANGED:
        if (iface->role() == QAccessible::EditableText) {
            QAccessibleTextUpdateEvent event(iface, -1, QString(), QString());
            QAccessible::updateAccessibility(&event);
        }
        break;
    default:
        break;
    }
}

#endif // QT_CONFIG(accessibility)

}
