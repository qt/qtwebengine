// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef RENDER_WIDGET_HOST_VIEW_QT_DELEGATE_QUICKWINDOW_H
#define RENDER_WIDGET_HOST_VIEW_QT_DELEGATE_QUICKWINDOW_H

#include "render_widget_host_view_qt_delegate.h"
#include "render_widget_host_view_qt_delegate_item.h"

#include <QtCore/qpointer.h>
#include <QtQuick/qquickwindow.h>

// silence syncqt
QT_BEGIN_NAMESPACE
QT_END_NAMESPACE

namespace QtWebEngineCore {

class RenderWidgetHostViewQtDelegateQuickWindow : public QQuickWindow , public WidgetDelegate {

public:
    RenderWidgetHostViewQtDelegateQuickWindow(RenderWidgetHostViewQtDelegateItem *realDelegate,
                                              QWindow *parent);
    ~RenderWidgetHostViewQtDelegateQuickWindow();

    void InitAsPopup(const QRect &screenRect) override;
    void SetClearColor(const QColor &) override;
    bool ActiveFocusOnPress() override;
    void MoveWindow(const QPoint &) override;
    void Bind(WebContentsAdapterClient *) override;
    void Unbind() override;
    void Destroy() override;
    void Resize(int width, int height) override;

    void setVirtualParent(QQuickItem *virtualParent);

private:
    QPointer<RenderWidgetHostViewQtDelegateItem> m_realDelegate;
    QQuickItem *m_virtualParent;
    QRect m_rect;
    bool m_rotated;
};

} // namespace QtWebEngineCore

#endif // RENDER_WIDGET_HOST_VIEW_QT_DELEGATE_QUICKWINDOW_H
