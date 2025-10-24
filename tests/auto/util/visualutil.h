// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef VISUALUTIL_H
#define VISUALUTIL_H

// Separate utils that require linking with GuiPrivate

#include <QtTest/QtTest>
#include <QtGui/private/qguiapplication_p.h>
#include <QtGui/qpa/qplatformintegration.h>

#define SKIP_IF_NO_WINDOW_ACTIVATION() \
do { \
    if (!(QGuiApplicationPrivate::platformIntegration()->hasCapability(QPlatformIntegration::WindowActivation))) \
        QSKIP("Window activation is not supported on this platform"); \
} while (false)

#endif /* VISUALUTIL_H */
