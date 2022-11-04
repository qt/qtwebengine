// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#ifndef MACOS_CONTEXT_TYPE_HELPER_H_
#define MACOS_CONTEXT_TYPE_HELPER_H_
bool isCurrentContextSoftware();
void* cglContext(NSOpenGLContext*);
#endif // MACOS_CONTEXT_TYPE_HELPER_H_
