

#ifndef BROWSER_ACCESSIBILITY_QT_H
#define BROWSER_ACCESSIBILITY_QT_H

#include <QtGui/qaccessible.h>
#include "content/browser/accessibility/browser_accessibility.h"

namespace content {

class BrowserAccessibilityQt
    : public BrowserAccessibility
    , public QAccessibleInterface
{
public:
    // QAccessibleInterface
    virtual bool isValid() const Q_DECL_OVERRIDE;
    virtual QObject *object() const Q_DECL_OVERRIDE;
    virtual QAccessibleInterface *childAt(int x, int y) const Q_DECL_OVERRIDE;

    // navigation, hierarchy
    virtual QAccessibleInterface *parent() const Q_DECL_OVERRIDE;
    virtual QAccessibleInterface *child(int index) const Q_DECL_OVERRIDE;
    virtual int childCount() const Q_DECL_OVERRIDE;
    virtual int indexOfChild(const QAccessibleInterface *) const Q_DECL_OVERRIDE;

    // properties and state
    virtual QString text(QAccessible::Text t) const Q_DECL_OVERRIDE;
    virtual void setText(QAccessible::Text t, const QString &text) Q_DECL_OVERRIDE;
    virtual QRect rect() const Q_DECL_OVERRIDE;
    virtual QAccessible::Role role() const Q_DECL_OVERRIDE;
    virtual QAccessible::State state() const Q_DECL_OVERRIDE;

    // BrowserAccessible
    void NativeAddReference() Q_DECL_OVERRIDE;
    void NativeReleaseReference() Q_DECL_OVERRIDE;
    bool IsNative() const Q_DECL_OVERRIDE { return true; }
};

}

#endif
