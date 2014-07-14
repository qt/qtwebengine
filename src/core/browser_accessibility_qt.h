/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
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

#ifndef BROWSER_ACCESSIBILITY_QT_H
#define BROWSER_ACCESSIBILITY_QT_H

#include <QtGui/qaccessible.h>
#include "content/browser/accessibility/browser_accessibility.h"

namespace content {

class BrowserAccessibilityQt
    : public BrowserAccessibility
    , public QAccessibleInterface
    , public QAccessibleTextInterface
{
public:
    BrowserAccessibilityQt();

    // QAccessibleInterface
    virtual bool isValid() const Q_DECL_OVERRIDE;
    virtual QObject *object() const Q_DECL_OVERRIDE;
    virtual QAccessibleInterface *childAt(int x, int y) const Q_DECL_OVERRIDE;
    virtual void *interface_cast(QAccessible::InterfaceType type) Q_DECL_OVERRIDE;

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

    // QAccessibleTextInterface
    void addSelection(int startOffset, int endOffset) Q_DECL_OVERRIDE;
    QString attributes(int offset, int *startOffset, int *endOffset) const Q_DECL_OVERRIDE;
    int cursorPosition() const Q_DECL_OVERRIDE;
    QRect characterRect(int offset) const Q_DECL_OVERRIDE;
    int selectionCount() const Q_DECL_OVERRIDE;
    int offsetAtPoint(const QPoint &point) const Q_DECL_OVERRIDE;
    void selection(int selectionIndex, int *startOffset, int *endOffset) const Q_DECL_OVERRIDE;
    QString text(int startOffset, int endOffset) const Q_DECL_OVERRIDE;
    void removeSelection(int selectionIndex) Q_DECL_OVERRIDE;
    void setCursorPosition(int position) Q_DECL_OVERRIDE;
    void setSelection(int selectionIndex, int startOffset, int endOffset) Q_DECL_OVERRIDE;
    int characterCount() const Q_DECL_OVERRIDE;
    void scrollToSubstring(int startIndex, int endIndex) Q_DECL_OVERRIDE;
};

}

#endif
