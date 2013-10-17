
#include "browser_accessibility_manager_qt.h"

#include "content/common/accessibility_notification.h"
#include "browser_accessibility_qt.h"

#include <qdebug.h>


namespace content {

BrowserAccessibility *BrowserAccessibilityFactoryQt::Create()
{
    return new BrowserAccessibilityQt();
}

BrowserAccessibilityManagerQt::BrowserAccessibilityManagerQt(
    QObject* parentObject,
    const AccessibilityNodeData& src,
    BrowserAccessibilityDelegate* delegate,
    BrowserAccessibilityFactory* factory)
    : BrowserAccessibilityManager(delegate, factory)
    , m_parentObject(parentObject) {
    Initialize(src);
}

QAccessibleInterface *BrowserAccessibilityManagerQt::rootParentAccessible()
{
    return QAccessible::queryAccessibleInterface(m_parentObject);
}

void BrowserAccessibilityManagerQt::NotifyRootChanged()
{
    qDebug() << "Notify root changed";
}

void BrowserAccessibilityManagerQt::NotifyAccessibilityEvent(
    int type,
    BrowserAccessibility* node)
{
    qDebug() << "NotifyAccessibilityEvent" << type << node;
    switch (type) {
    case AccessibilityNotificationFocusChanged: {
        qDebug() << "GOT FOCUS CHANGED";
        // FIXME make sure we keep this thing alive...???
//        node->NativeAddReference();
        BrowserAccessibilityQt *iface = static_cast<BrowserAccessibilityQt*>(node);
        QAccessibleEvent event(iface, QAccessible::Focus);
        QAccessible::updateAccessibility(&event);
        break;
    }
    case AccessibilityNotificationCheckStateChanged:
        qDebug() << "handle me: Check state changed";
        break;
    case AccessibilityNotificationChildrenChanged:
        qDebug() << "handle me: Children changed";
        break;
    case AccessibilityNotificationLayoutComplete:
        qDebug() << "handle me: Layout complete";
        break;
    case AccessibilityNotificationLoadComplete:
        qDebug() << "handle me: Load complete";
        break;
    case AccessibilityNotificationTextChanged:
        qDebug() << "handle me: Text changed";
        break;
    case AccessibilityNotificationTextInserted:
        qDebug() << "handle me: Text inserted";
        break;
    case AccessibilityNotificationTextRemoved:
        qDebug() << "handle me: Text removed";
        break;
    default:
        qDebug() << "UNHANDLED NOTIFICATION: " << type;
        break;
    }
}

}
