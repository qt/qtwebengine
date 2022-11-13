// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qquickwebenginetouchhandleprovider_p_p.h"

// static
QString QQuickWebEngineTouchHandleProvider::identifier()
{
    return QStringLiteral("touchhandle");
}

// static
QUrl QQuickWebEngineTouchHandleProvider::url(int orientation)
{
    return QUrl(QStringLiteral("image://%1/%2").arg(identifier(), QString::number(orientation)));
}

QQuickWebEngineTouchHandleProvider::QQuickWebEngineTouchHandleProvider()
    : QQuickImageProvider(QQuickImageProvider::Image)
{
}

QQuickWebEngineTouchHandleProvider::~QQuickWebEngineTouchHandleProvider()
{
}

void QQuickWebEngineTouchHandleProvider::init(const QMap<int, QImage> &images)
{
    if (!m_touchHandleMap.empty()) {
        Q_ASSERT(images.size() == m_touchHandleMap.size());
        return;
    }

    m_touchHandleMap = images;
}

QImage QQuickWebEngineTouchHandleProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(size);
    Q_UNUSED(requestedSize);

    Q_ASSERT(m_touchHandleMap.contains(id.toInt()));
    return m_touchHandleMap.value(id.toInt());
}
