// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QPDFFILE_P_H
#define QPDFFILE_P_H

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

#include "qpdfdocument.h"

#include <QtCore/qfile.h>

QT_BEGIN_NAMESPACE

class Q_PDF_EXPORT QPdfFile : public QFile
{
    Q_OBJECT
public:
    QPdfFile(QPdfDocument *doc);
    QPdfDocument *document() { return m_document; }

private:
    QPdfDocument *m_document;
};

QT_END_NAMESPACE

#endif // QPDFFILE_P_H
