// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QTWEBENGINEWIDGETSGLOBAL_H
#define QTWEBENGINEWIDGETSGLOBAL_H

#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

#ifndef QT_STATIC
#  if defined(QT_BUILD_WEBENGINEWIDGETS_LIB)
#      define QWEBENGINEWIDGETS_EXPORT Q_DECL_EXPORT
#  else
#      define QWEBENGINEWIDGETS_EXPORT Q_DECL_IMPORT
#  endif
#else
#  define QWEBENGINEWIDGETS_EXPORT
#endif

QT_END_NAMESPACE

#endif // QTWEBENGINEWIDGETSGLOBAL_H
