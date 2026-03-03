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

// FIXME This shouldn't be necessary, but some DEs like KDE Plasma suppress (frequent) window
// activations even when requested. If any of these lines is removed, the test becomes flaky.
// This is probably a bug in QWayland and/or qtbase.
#define MAKE_WINDOW_ACTIVE(view)                                                                   \
    do {                                                                                           \
        QVERIFY2(QGuiApplicationPrivate::platformIntegration()->hasCapability(                     \
                         QPlatformIntegration::WindowActivation),                                  \
                 "Platform should support window activation for MAKE_WINDOW_ACTIVE macro");        \
        (view).show();                                                                             \
        QVERIFY(QTest::qWaitForWindowExposed(&(view)));                                            \
        (view).window()->windowHandle()->requestActivate();                                        \
        QVERIFY(QTest::qWaitForWindowActive(&(view)));                                             \
    } while (false)

#endif /* VISUALUTIL_H */
