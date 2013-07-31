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

#ifndef RENDER_WIDGET_HOST_VIEW_QT_DELEGATE_H
#define RENDER_WIDGET_HOST_VIEW_QT_DELEGATE_H

#include "qtwebengineglobal.h"

#include <QRect>
#include <QScopedPointer>

class BackingStoreQt;
class QEvent;
class QPainter;
class QWindow;
class RenderWidgetHostViewQt;

class QWEBENGINE_EXPORT RenderWidgetHostViewQtDelegate {

public:
    virtual ~RenderWidgetHostViewQtDelegate();
    virtual QRectF screenRect() const = 0;
    virtual void setKeyboardFocus() = 0;
    virtual bool hasKeyboardFocus() = 0;
    virtual void show() = 0;
    virtual void hide() = 0;
    virtual bool isVisible() const = 0;
    virtual QWindow* window() const = 0;
    virtual void update(const QRect& rect = QRect()) = 0;
    virtual void resize(int width, int height) = 0;
    void resetView(RenderWidgetHostViewQt*);

protected:
    RenderWidgetHostViewQtDelegate();
    void paint(QPainter*, const QRectF& boundingRect);
    void fetchBackingStore();
    void notifyResize();
    bool forwardEvent(QEvent*);

private:
    QScopedPointer<RenderWidgetHostViewQt> m_view;
    BackingStoreQt *m_backingStore;
};

#endif
