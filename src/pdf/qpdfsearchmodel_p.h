// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QPDFSEARCHMODEL_P_H
#define QPDFSEARCHMODEL_P_H

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

#include "qpdfsearchmodel.h"
#include <private/qabstractitemmodel_p.h>

#include "third_party/pdfium/public/fpdfview.h"

QT_BEGIN_NAMESPACE

class QPdfSearchModelPrivate : public QAbstractItemModelPrivate
{
    Q_DECLARE_PUBLIC(QPdfSearchModel)

public:
    QPdfSearchModelPrivate();
    void clearResults();
    bool doSearch(int page);

    struct PageAndIndex {
        int page;
        int index;
    };
    PageAndIndex pageAndIndexForResult(int resultIndex);
    int rowsBeforePage(int page);

    QPdfDocument *document = nullptr;
    QString searchString;
    QList<bool> pagesSearched;
    QList<QList<QPdfLink>> searchResults;
    int rowCountSoFar = 0;
    int updateTimerId = -1;
    int nextPageToUpdate = 0;

    QMetaObject::Connection documentConnection;
};

QT_END_NAMESPACE

#endif // QPDFSEARCHMODEL_P_H
