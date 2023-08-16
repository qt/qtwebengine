// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QPDFPAGESELECTOR_P_H
#define QPDFPAGESELECTOR_P_H

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

#include "qpdfpageselector.h"

#include <QPointer>

QT_BEGIN_NAMESPACE

class QPdfPageSelectorPrivate
{
    Q_DECLARE_PUBLIC(QPdfPageSelector)

public:
    QPdfPageSelectorPrivate(QPdfPageSelector *q);

    void documentStatusChanged();

    QPdfPageSelector *q_ptr;
    QPointer<QPdfDocument> document;
    QMetaObject::Connection documentStatusChangedConnection;
};

QT_END_NAMESPACE

#endif // QPDFPAGESELECTOR_P_H
