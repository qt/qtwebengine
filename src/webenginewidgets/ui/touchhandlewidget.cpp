// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "touchhandlewidget_p.h"
#include "qwebengineview.h"

#include <QBoxLayout>
#include <QGraphicsOpacityEffect>
#include <QLabel>
#include <QWidget>

namespace QtWebEngineWidgetUI {
TouchHandleWidget::TouchHandleWidget(QWebEngineView *view, const QMap<int, QImage> &images)
    : m_widget(new QWidget(view))
    , m_label(new QLabel(m_widget))
    , m_opacityEffect(new QGraphicsOpacityEffect(m_widget))
    , m_images(images)
{
    m_widget->setAttribute(Qt::WA_AcceptTouchEvents, true);
    m_widget->setAttribute(Qt::WA_TransparentForMouseEvents, true);
}

TouchHandleWidget::~TouchHandleWidget()
{
}

void TouchHandleWidget::setImage(int orientation)
{
    const QImage &img = m_images[orientation];
    m_label->setPixmap(QPixmap::fromImage(img));
    m_label->setFrameStyle(QFrame::NoFrame);
    m_label->resize(img.size());
    m_label->setVisible(true);

    QVBoxLayout layout;
    layout.setSpacing(0);
    layout.setContentsMargins(QMargins());
    layout.addWidget(m_label);
    layout.setParent(m_widget);
    m_widget->setLayout(&layout);
    m_widget->resize(img.size());
}

void TouchHandleWidget::setBounds(const QRect &bounds)
{
    m_widget->setGeometry(bounds);
}

void TouchHandleWidget::setVisible(bool visible)
{
    m_widget->setVisible(visible);
}

void TouchHandleWidget::setOpacity(float opacity)
{
    m_opacityEffect->setOpacity(opacity);
    m_widget->setGraphicsEffect(m_opacityEffect);
}
} // namespace QtWebEngineWidgetUI
