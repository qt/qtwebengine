// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QPDFLINK_P_H
#define QPDFLINK_P_H

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

#include "qpdflink.h"

#include <QPointF>
#include <QRectF>
#include <QUrl>

QT_BEGIN_NAMESPACE

class QPdfLinkPrivate : public QSharedData
{
public:
    QPdfLinkPrivate() = default;
    QPdfLinkPrivate(int page, QPointF location, qreal zoom)
        : page(page),
          location(location),
          zoom(zoom) { }
    QPdfLinkPrivate(int page, QList<QRectF> rects, QString contextBefore, QString contextAfter)
        : page(page),
          location(rects.first().topLeft()),
          zoom(0),
          contextBefore{std::move(contextBefore)},
          contextAfter{std::move(contextAfter)},
          rects{std::move(rects)} {}

    int page = -1;
    QPointF location;
    qreal zoom = 1;
    QString contextBefore;
    QString contextAfter;
    QUrl url;
    QList<QRectF> rects;
};

QT_END_NAMESPACE

#endif // QPDFLINK_P_H
