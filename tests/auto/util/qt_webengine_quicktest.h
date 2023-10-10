// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef QT_WEBENGINE_QUICKTEST_H
#define QT_WEBENGINE_QUICKTEST_H

#include <QtQuickTest/quicktestglobal.h>

#ifdef QT_WIDGETS_LIB
#include <QtWidgets/QApplication>
#else
#include <QtGui/QGuiApplication>
#endif

QT_BEGIN_NAMESPACE

#ifdef QT_WIDGETS_LIB
#define Application QApplication
#else
#define Application QGuiApplication
#endif

QT_END_NAMESPACE

#endif // QT_WEBENGINE_QUICKTEST_H
