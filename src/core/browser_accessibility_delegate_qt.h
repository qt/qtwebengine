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

#ifndef BROWSER_ACCESSIBILITY_DELEGATE_QT_H
#define BROWSER_ACCESSIBILITY_DELEGATE_QT_H

#include <QtCore/qglobal.h>

#include "content/browser/accessibility/browser_accessibility_manager.h"
#include "content/browser/renderer_host/render_widget_host_view_base.h"

class BrowserAccessibilityDelegateQt
        : public content::BrowserAccessibilityDelegate
{
public:
    BrowserAccessibilityDelegateQt(content::RenderWidgetHostViewBase *rwhv);

    virtual bool HasFocus() const Q_DECL_OVERRIDE;
    virtual void SetAccessibilityFocus(int acc_obj_id) Q_DECL_OVERRIDE;

    virtual void AccessibilityDoDefaultAction(int acc_obj_id) Q_DECL_OVERRIDE;

    virtual gfx::Rect GetViewBounds() const Q_DECL_OVERRIDE;
    virtual void AccessibilityScrollToMakeVisible(
        int acc_obj_id, gfx::Rect subfocus) Q_DECL_OVERRIDE;
    virtual void AccessibilityScrollToPoint(
        int acc_obj_id, gfx::Point point) Q_DECL_OVERRIDE;

    virtual void AccessibilitySetTextSelection(
        int acc_obj_id, int start_offset, int end_offset) Q_DECL_OVERRIDE;

    virtual gfx::Point GetLastTouchEventLocation() const Q_DECL_OVERRIDE;

    virtual void FatalAccessibilityTreeError() Q_DECL_OVERRIDE;

private:
    Q_DISABLE_COPY(BrowserAccessibilityDelegateQt)
    content::RenderWidgetHostViewBase *m_rwhv;
};

#endif
