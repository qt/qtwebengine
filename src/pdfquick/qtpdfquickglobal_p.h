// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QTPDFQUICKGLOBAL_H
#define QTPDFQUICKGLOBAL_H

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

#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

#ifndef QT_STATIC
#    if defined(QT_BUILD_PDFQUICK_LIB)
#        define Q_PDFQUICK_EXPORT Q_DECL_EXPORT
#    else
#        define Q_PDFQUICK_EXPORT Q_DECL_IMPORT
#    endif
#else
#    define Q_PDFQUICK_EXPORT
#endif

QT_END_NAMESPACE

#endif // QTPDFQUICKGLOBAL_H
