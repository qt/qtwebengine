// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef RENDER_WIDGET_HOST_VIEW_QT_DELEGATE_H
#define RENDER_WIDGET_HOST_VIEW_QT_DELEGATE_H

#include <QtWebEngineCore/private/qtwebenginecoreglobal_p.h>
#include <QtCore/QRect>
#include <QtGui/QColor>
#include <QtGui/QCursor>
#include <QtGui/QImage>

QT_BEGIN_NAMESPACE
class QWheelEvent;
class QWindow;
QT_END_NAMESPACE

namespace QtWebEngineCore {

class WebContentsAdapterClient;
class Q_WEBENGINECORE_PRIVATE_EXPORT RenderWidgetHostViewQtDelegate {
public:
    virtual ~RenderWidgetHostViewQtDelegate() { }
    virtual void initAsPopup(const QRect&) = 0;
    virtual QRectF viewGeometry() const = 0;
    virtual QRect windowGeometry() const = 0;
    virtual void setKeyboardFocus() = 0;
    virtual bool hasKeyboardFocus() = 0;
    virtual void lockMouse() = 0;
    virtual void unlockMouse() = 0;
    virtual void show() = 0;
    virtual void hide() = 0;
    virtual bool isVisible() const = 0;
    virtual QWindow *Window() const = 0;
    virtual void updateCursor(const QCursor &) = 0;
    virtual void resize(int width, int height) = 0;
    virtual void move(const QPoint &) = 0;
    virtual void inputMethodStateChanged(bool editorVisible, bool passwordInput) = 0;
    virtual void setInputMethodHints(Qt::InputMethodHints hints) = 0;
    virtual void setClearColor(const QColor &color) = 0;
    virtual void adapterClientChanged(WebContentsAdapterClient *client) = 0;
    virtual void updateAdapterClientIfNeeded(WebContentsAdapterClient *client) = 0;
    virtual void unhandledWheelEvent(QWheelEvent *) {}
};

} // namespace QtWebEngineCore

#endif // RENDER_WIDGET_HOST_VIEW_QT_DELEGATE_H
