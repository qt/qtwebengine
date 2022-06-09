// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

// Functions and macros that really need to be in QTestLib

#include "util.h"

#include <QApplication>

#define W_QTEST_MAIN(TestObject, params) \
int main(int argc, char *argv[]) \
{ \
    QList<const char *> w_argv(argc); \
    QLatin1String arg("--webEngineArgs"); \
    for (int i = 0; i < argc; ++i) \
        w_argv[i] = argv[i]; \
    w_argv.append(arg.data()); \
    for (int i = 0; i < params.size(); ++i) \
        w_argv.append(params[i].data()); \
    int w_argc = w_argv.size(); \
    \
    QApplication app(w_argc, const_cast<char **>(w_argv.data())); \
    app.setAttribute(Qt::AA_Use96Dpi, true); \
    QTEST_DISABLE_KEYPAD_NAVIGATION \
    TestObject tc; \
    QTEST_SET_MAIN_SOURCE_PATH \
    return QTest::qExec(&tc, argc, argv); \
}
