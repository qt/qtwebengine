// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QTPDFWIDGETSGLOBAL_H
#define QTPDFWIDGETSGLOBAL_H

#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

#ifndef Q_PDF_WIDGETS_EXPORT
#  ifndef QT_STATIC
#    if defined(QT_BUILD_PDFWIDGETS_LIB)
#      define Q_PDF_WIDGETS_EXPORT Q_DECL_EXPORT
#    else
#      define Q_PDF_WIDGETS_EXPORT Q_DECL_IMPORT
#    endif
#  else
#    define Q_PDF_WIDGETS_EXPORT
#  endif
#endif

QT_END_NAMESPACE

#endif // QTPDFWIDGETSGLOBAL_H

