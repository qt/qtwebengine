/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef SURFACE_FACTORY_QT
#define SURFACE_FACTORY_QT

#if defined(USE_OZONE)

#include "ui/gfx/ozone/surface_factory_ozone.h"

#include <QtGlobal>

class SurfaceFactoryQt
    : public gfx::SurfaceFactoryOzone
{
    virtual bool LoadEGLGLES2Bindings(AddGLLibraryCallback add_gl_library, SetGLGetProcAddressProcCallback set_gl_get_proc_address) Q_DECL_OVERRIDE;
    virtual intptr_t GetNativeDisplay() Q_DECL_OVERRIDE;
    virtual gfx::SurfaceFactoryOzone::HardwareState InitializeHardware() Q_DECL_OVERRIDE { return gfx::SurfaceFactoryOzone::INITIALIZED; }
    virtual void ShutdownHardware() Q_DECL_OVERRIDE {}
    virtual gfx::AcceleratedWidget GetAcceleratedWidget() Q_DECL_OVERRIDE { return 0; }
    virtual gfx::AcceleratedWidget RealizeAcceleratedWidget(gfx::AcceleratedWidget w) Q_DECL_OVERRIDE { return 0; }
    virtual bool AttemptToResizeAcceleratedWidget(gfx::AcceleratedWidget w, const gfx::Rect& bounds) Q_DECL_OVERRIDE { return false; }
    virtual gfx::VSyncProvider* GetVSyncProvider(gfx::AcceleratedWidget w) Q_DECL_OVERRIDE { return NULL; }
};

#endif

#endif // SURFACE_FACTORY_QT

