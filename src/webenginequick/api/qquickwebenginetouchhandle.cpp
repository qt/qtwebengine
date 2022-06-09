// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qquickwebenginetouchhandle_p.h"
#include "qquickwebenginetouchhandleprovider_p_p.h"
#include "web_contents_adapter_client.h"
#include <QtQuick/qquickitem.h>

QT_BEGIN_NAMESPACE

QQuickWebEngineTouchHandle::QQuickWebEngineTouchHandle() : QObject(nullptr), m_hasImage(false) { }

void QQuickWebEngineTouchHandle::setBounds(const QRect &bounds)
{
    m_item->setX(bounds.x());
    m_item->setY(bounds.y());
    m_item->setWidth(bounds.width());
    m_item->setHeight(bounds.height());
}

void QQuickWebEngineTouchHandle::setOpacity(float opacity)
{
    m_item->setOpacity(opacity);
}

void QQuickWebEngineTouchHandle::setImage(int orientation)
{
    if (m_hasImage) {
        QUrl url = QQuickWebEngineTouchHandleProvider::url(orientation);
        m_item->setProperty("source", url);
    }
}

void QQuickWebEngineTouchHandle::setVisible(bool visible)
{
    m_item->setVisible(visible);
}

void QQuickWebEngineTouchHandle::setItem(QQuickItem *item, bool hasImage)
{
    m_hasImage = hasImage;
    m_item.reset(item);
}

QT_END_NAMESPACE
