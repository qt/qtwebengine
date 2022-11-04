// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

#include "macos_context_type_helper.h"

bool isCurrentContextSoftware()
{
    int rendererID = 0;
    [NSOpenGLContext.currentContext getValues:&rendererID forParameter:NSOpenGLContextParameterCurrentRendererID];
    return (rendererID & kCGLRendererIDMatchingMask) == kCGLRendererGenericFloatID;
}

void* cglContext(NSOpenGLContext *nsOpenGLContext)
{
   return [nsOpenGLContext CGLContextObj];
}
