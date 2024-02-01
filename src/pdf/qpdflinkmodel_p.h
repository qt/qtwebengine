// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QPDFLINKMODEL_P_H
#define QPDFLINKMODEL_P_H

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

#include "qpdflinkmodel.h"
#include <private/qabstractitemmodel_p.h>

QT_BEGIN_NAMESPACE

class QPdfLinkModelPrivate
{
    QPdfLinkModel *q_ptr;
    Q_DECLARE_PUBLIC(QPdfLinkModel)

public:
    explicit QPdfLinkModelPrivate(QPdfLinkModel *qq)
        : q_ptr(qq) {}

    void update();

    QHash<int, QByteArray> roleNames;
    QPdfDocument *document = nullptr;
    QList<QPdfLink> links;
    int page = 0;
};

QT_END_NAMESPACE

#endif // QPDFLINKMODEL_P_H
