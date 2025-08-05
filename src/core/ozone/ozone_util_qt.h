// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef OZONE_UTIL_QT_H
#define OZONE_UTIL_QT_H

#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

class QOpenGLContext;

namespace OzoneUtilQt {
void *getXDisplay();
QOpenGLContext *getQOpenGLContext();
bool usingGLX();
bool usingEGL();
} // namespace OzoneUtilQt

QT_END_NAMESPACE

#endif // OZONE_UTIL_QT_H
