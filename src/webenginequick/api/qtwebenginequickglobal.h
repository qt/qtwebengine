// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QTWEBENGINEQUICKGLOBAL_H
#define QTWEBENGINEQUICKGLOBAL_H

#include <QtCore/qglobal.h>
#include <QtWebEngineQuick/qtwebenginequick-config.h>

QT_BEGIN_NAMESPACE

#ifndef QT_STATIC
#  if defined(QT_BUILD_WEBENGINE_LIB)
#      define Q_WEBENGINEQUICK_EXPORT Q_DECL_EXPORT
#  else
#      define Q_WEBENGINEQUICK_EXPORT Q_DECL_IMPORT
#  endif
#else
#  define Q_WEBENGINEQUICK_EXPORT
#endif

namespace QtWebEngineQuick
{
    Q_WEBENGINEQUICK_EXPORT void initialize();
}

QT_END_NAMESPACE

#endif // QTWEBENGINEQUICKGLOBAL_H
