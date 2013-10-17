

#ifndef BROWSER_ACCESSIBILITY_MANAGER_QT_H
#define BROWSER_ACCESSIBILITY_MANAGER_QT_H

#include "content/browser/accessibility/browser_accessibility_manager.h"
#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE
class QAccessibleInterface;
QT_END_NAMESPACE

namespace content {

class BrowserAccessibilityFactoryQt : public BrowserAccessibilityFactory
{
public:
    BrowserAccessibility* Create() Q_DECL_OVERRIDE;
};

class BrowserAccessibilityManagerQt : public BrowserAccessibilityManager
{
public:
    BrowserAccessibilityManagerQt(
        QObject* parentObject,
        const AccessibilityNodeData& src,
        BrowserAccessibilityDelegate* delegate,
        BrowserAccessibilityFactory* factory = new BrowserAccessibilityFactoryQt());

    void NotifyRootChanged() Q_DECL_OVERRIDE;
    void NotifyAccessibilityEvent(
        int type,
        BrowserAccessibility* node) Q_DECL_OVERRIDE;

    QAccessibleInterface *rootParentAccessible();

private:
    QObject *m_parentObject;
};

}

#endif
