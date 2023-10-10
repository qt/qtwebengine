// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QTPDFGLOBAL_H
#define QTPDFGLOBAL_H

#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

#ifndef Q_PDF_EXPORT
#  ifndef QT_STATIC
#    if defined(QT_BUILD_PDF_LIB)
#      define Q_PDF_EXPORT Q_DECL_EXPORT
#    else
#      define Q_PDF_EXPORT Q_DECL_IMPORT
#    endif
#  else
#    define Q_PDF_EXPORT
#  endif
#endif

#define Q_PDF_PRIVATE_EXPORT Q_PDF_EXPORT

QT_END_NAMESPACE

#endif // QTPDFGLOBAL_H

