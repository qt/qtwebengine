// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "gl_helper.h"
#include "ozone_util_qt.h"

#include <QtGui/qopenglcontext.h>

QT_BEGIN_NAMESPACE

const char *getGLErrorString(uint32_t error)
{
    switch (error) {
    case GL_INVALID_ENUM:
        return "GL_INVALID_ENUM";
    case GL_INVALID_VALUE:
        return "GL_INVALID_VALUE";
    case GL_INVALID_OPERATION:
        return "GL_INVALID_OPERATION";
    case GL_STACK_OVERFLOW:
        return "GL_STACK_OVERFLOW";
    case GL_STACK_UNDERFLOW:
        return "GL_STACK_UNDERFLOW";
    case GL_OUT_OF_MEMORY:
        return "GL_OUT_OF_MEMORY";
    case GL_INVALID_FRAMEBUFFER_OPERATION:
        return "GL_INVALID_FRAMEBUFFER_OPERATION";
    case GL_CONTEXT_LOST:
        return "GL_CONTEXT_LOST";
    case GL_NO_ERROR:
        return "GL_NO_ERROR";
    default:
        return "UNKNOWN";
    }
}

GLHelper::GLExtFunctions::GLExtFunctions()
{
    QOpenGLContext *context = OzoneUtilQt::getQOpenGLContext();

    glCreateMemoryObjectsEXT = reinterpret_cast<PFNGLCREATEMEMORYOBJECTSEXTPROC>(
            context->getProcAddress("glCreateMemoryObjectsEXT"));
    glDeleteMemoryObjectsEXT = reinterpret_cast<PFNGLDELETEMEMORYOBJECTSEXTPROC>(
            context->getProcAddress("glDeleteMemoryObjectsEXT"));
    glEGLImageTargetTexture2DOES = reinterpret_cast<PFNGLEGLIMAGETARGETTEXTURE2DOESPROC>(
            context->getProcAddress("glEGLImageTargetTexture2DOES"));
    glImportMemoryFdEXT = reinterpret_cast<PFNGLIMPORTMEMORYFDEXTPROC>(
            context->getProcAddress("glImportMemoryFdEXT"));
    glIsMemoryObjectEXT = reinterpret_cast<PFNGLISMEMORYOBJECTEXTPROC>(
            context->getProcAddress("glIsMemoryObjectEXT"));
    glMemoryObjectParameterivEXT = reinterpret_cast<PFNGLMEMORYOBJECTPARAMETERIVEXTPROC>(
            context->getProcAddress("glMemoryObjectParameterivEXT"));
    glTexStorageMem2DEXT = reinterpret_cast<PFNGLTEXSTORAGEMEM2DEXTPROC>(
            context->getProcAddress("glTexStorageMem2DEXT"));
}

GLHelper *GLHelper::instance()
{
    static GLHelper glHelper;
    return &glHelper;
}

GLHelper::GLHelper() : m_functions(new GLHelper::GLExtFunctions()) { }

QT_END_NAMESPACE
