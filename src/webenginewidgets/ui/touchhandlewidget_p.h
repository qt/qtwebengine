// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef TOUCHHANDLEWIDGET_P_H
#define TOUCHHANDLEWIDGET_P_H

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

#include "touch_handle_drawable_client.h"

#include <QImage>
#include <QMap>

QT_BEGIN_NAMESPACE
class QGraphicsOpacityEffect;
class QLabel;
class QWebEngineView;
class QWidget;
QT_END_NAMESPACE

namespace QtWebEngineWidgetUI {
class TouchHandleWidget : public QtWebEngineCore::TouchHandleDrawableDelegate
{
public:
    TouchHandleWidget(QWebEngineView *view, const QMap<int, QImage> &images);
    ~TouchHandleWidget();

    void setImage(int orientation) override;
    void setBounds(const QRect &bounds) override;
    void setVisible(bool visible) override;
    void setOpacity(float opacity) override;

private:
    QWidget *m_widget;
    QLabel *m_label;
    QGraphicsOpacityEffect *m_opacityEffect;
    QMap<int, QImage> m_images;
};
} // namespace QtWebEngineWidgetUI

#endif // TOUCHHANDLEWIDGET_P_H
