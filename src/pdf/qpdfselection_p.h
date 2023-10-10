// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QPDFSELECTION_P_H
#define QPDFSELECTION_P_H

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

#include "qpdfselection.h"

#include <QList>
#include <QPolygonF>

QT_BEGIN_NAMESPACE

class QPdfSelectionPrivate : public QSharedData
{
public:
    QPdfSelectionPrivate() = default;
    QPdfSelectionPrivate(const QString &text, QList<QPolygonF> bounds, QRectF boundingRect, int startIndex, int endIndex)
        : text(text),
          bounds(bounds),
          boundingRect(boundingRect),
          startIndex(startIndex),
          endIndex(endIndex) { }

    QString text;
    QList<QPolygonF> bounds;
    QRectF boundingRect;
    int startIndex;
    int endIndex;
};

QT_END_NAMESPACE

#endif // QPDFSELECTION_P_H
