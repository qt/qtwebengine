/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "surface_factory_qt.h"

#include "gl_context_qt.h"
#include "type_conversion.h"

#include "base/files/file_path.h"
#include "base/native_library.h"
#include "ui/gl/gl_context_egl.h"
#include "ui/gl/gl_implementation.h"
#include "ui/gl/gl_surface.h"
#include "ui/gl/init/gl_initializer.h"
#include "ui/gl/init/gl_factory.h"
#include "ui/ozone/common/gl_ozone_egl.h"

#include <QGuiApplication>

#if defined(USE_OZONE)

#include <EGL/egl.h>
#include <QOpenGLContext>
#include <dlfcn.h>

Q_GUI_EXPORT QOpenGLContext *qt_gl_global_share_context();

namespace QtWebEngineCore {

class GLOzoneQt : public ui::GLOzoneEGL {
public:
    scoped_refptr<gl::GLSurface> CreateViewGLSurface(gfx::AcceleratedWidget /*window*/) override
    {
        return nullptr;
    }
    scoped_refptr<gl::GLSurface> CreateOffscreenGLSurface(const gfx::Size& /*size*/) override
    {
        return nullptr;
    }

protected:
    // Returns native platform display handle. This is used to obtain the EGL
    // display connection for the native display.
    intptr_t GetNativeDisplay() override;

    // Sets up GL bindings for the native surface.
    bool LoadGLES2Bindings(gl::GLImplementation implementation) override;
};

base::NativeLibrary LoadLibrary(const base::FilePath& filename) {
    base::NativeLibraryLoadError error;
    base::NativeLibrary library = base::LoadNativeLibrary(filename, &error);
    if (!library) {
        LOG(ERROR) << "Failed to load " << filename.MaybeAsASCII() << ": " << error.ToString();
        return NULL;
    }
    return library;
}

bool GLOzoneQt::LoadGLES2Bindings(gl::GLImplementation /*implementation*/)
{
    base::NativeLibrary eglgles2Library = dlopen(NULL, RTLD_LAZY);
    if (!eglgles2Library) {
        LOG(ERROR) << "Failed to open EGL/GLES2 context " << dlerror();
        return false;
    }

    gl::GLGetProcAddressProc get_proc_address =
            reinterpret_cast<gl::GLGetProcAddressProc>(
                base::GetFunctionPointerFromNativeLibrary(eglgles2Library,
                                                          "eglGetProcAddress"));
    if (!get_proc_address) {
        // QTBUG-63341 most likely libgles2 not linked with libegl -> fallback to qpa
        if (QOpenGLContext *context = qt_gl_global_share_context()) {
            get_proc_address = reinterpret_cast<gl::GLGetProcAddressProc>(
                context->getProcAddress("eglGetProcAddress"));
        }
    }

    if (!get_proc_address) {
        LOG(ERROR) << "eglGetProcAddress not found.";
        base::UnloadNativeLibrary(eglgles2Library);
        return false;
    }

    gl::SetGLGetProcAddressProc(get_proc_address);
    gl::AddGLNativeLibrary(eglgles2Library);
    return true;
}

intptr_t GLOzoneQt::GetNativeDisplay()
{
    static void *display = GLContextHelper::getNativeDisplay();

    if (display)
        return reinterpret_cast<intptr_t>(display);

    return reinterpret_cast<intptr_t>(EGL_DEFAULT_DISPLAY);
}

std::vector<gl::GLImplementation> SurfaceFactoryQt::GetAllowedGLImplementations()
{
    std::vector<gl::GLImplementation> impls;
    impls.push_back(gl::kGLImplementationEGLGLES2);
    return impls;
}

ui::GLOzone* SurfaceFactoryQt::GetGLOzone(gl::GLImplementation implementation)
{
    return new GLOzoneQt();
}

} // namespace QtWebEngineCore

#endif // defined(USE_OZONE)

