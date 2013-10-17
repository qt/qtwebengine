/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

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
