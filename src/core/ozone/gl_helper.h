// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef GL_HELPER_H
#define GL_HELPER_H

#include <QtCore/qscopedpointer.h>

// This is a workaround to avoid to include //gpu/GLES2/gl2chromium.h
#define GPU_GLES2_GL2CHROMIUM_H_
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#undef glCreateMemoryObjectsEXT
#undef glDeleteMemoryObjectsEXT
#undef glEGLImageTargetTexture2DOES
#undef glImportMemoryFdEXT
#undef glTextureStorageMem2DEXT

QT_BEGIN_NAMESPACE

const char *getGLErrorString(uint32_t error);

class GLHelper
{
public:
    struct GLExtFunctions
    {
        GLExtFunctions();

        PFNGLCREATEMEMORYOBJECTSEXTPROC glCreateMemoryObjectsEXT;
        PFNGLDELETEMEMORYOBJECTSEXTPROC glDeleteMemoryObjectsEXT;
        PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES;
        PFNGLIMPORTMEMORYFDEXTPROC glImportMemoryFdEXT;
        PFNGLISMEMORYOBJECTEXTPROC glIsMemoryObjectEXT;
        PFNGLMEMORYOBJECTPARAMETERIVEXTPROC glMemoryObjectParameterivEXT;
        PFNGLTEXSTORAGEMEM2DEXTPROC glTexStorageMem2DEXT;
    };

    static GLHelper *instance();

    GLExtFunctions *functions() const { return m_functions.get(); }

private:
    GLHelper();

    QScopedPointer<GLExtFunctions> m_functions;
};

QT_END_NAMESPACE

#endif // GL_HELPER_H
