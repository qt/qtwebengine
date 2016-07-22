/******************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt PDF Module.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
******************************************************************************/

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

QT_END_NAMESPACE

#endif // QTPDFGLOBAL_H

