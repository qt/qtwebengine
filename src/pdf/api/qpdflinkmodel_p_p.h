/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtPDF module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QPDFLINKMODEL_P_P_H
#define QPDFLINKMODEL_P_P_H

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

#include "qpdflinkmodel_p.h"
#include <private/qabstractitemmodel_p.h>

#include "third_party/pdfium/public/fpdfview.h"

#include <QUrl>

QT_BEGIN_NAMESPACE

class QPdfLinkModelPrivate: public QAbstractItemModelPrivate
{
    Q_DECLARE_PUBLIC(QPdfLinkModel)

public:
    QPdfLinkModelPrivate();

    void update();

    struct Link {
        // where it is on the current page
        QRectF rect;
        int textStart = -1;
        int textCharCount = 0;
        // destination inside PDF
        int page = -1; // -1 means look at the url instead
        QPointF location;
        qreal zoom = 0; // 0 means no specified zoom: don't change when clicking
        // web destination
        QUrl url;

        QString toString() const;
    };

    QPdfDocument *document = nullptr;
    QVector<Link> links;
    int page = 0;
};

QT_END_NAMESPACE

#endif // QPDFLINKMODEL_P_P_H
