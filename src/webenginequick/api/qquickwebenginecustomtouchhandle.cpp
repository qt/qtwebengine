/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qquickwebenginecustomtouchhandle_p.h"
#include "web_contents_adapter_client.h"
#include <QtQuick/qquickitem.h>

QT_BEGIN_NAMESPACE

/*!
    \class QQuickWebEngineCustomTouchHandle
    \since 6.4
    \brief The QQuickWebEngineCustomTouchHandle class provides information about the touch handle.
    \inmodule QtWebEngineQuick
    QQuickWebEngineCustomTouchHandle is returned after a touch selection event,
    and contains information about the touch handle.
*/

QQuickWebEngineCustomTouchHandle::QQuickWebEngineCustomTouchHandle()
    : QObject(nullptr)
{
}

void QQuickWebEngineCustomTouchHandle::setBounds(const QRect &bounds)
{
    item->setX(bounds.x());
    item->setY(bounds.y());
    item->setWidth(bounds.width());
    item->setHeight(bounds.height());
}

void QQuickWebEngineCustomTouchHandle::setOpacity(float opacity)
{
    item->setOpacity(opacity);
}

void QQuickWebEngineCustomTouchHandle::setImage(int orientation)
{
    Q_UNUSED(orientation);
}

void QQuickWebEngineCustomTouchHandle::setVisible(bool visible)
{
    item->setVisible(visible);
}

QT_END_NAMESPACE
