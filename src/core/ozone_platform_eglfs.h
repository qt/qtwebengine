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

#ifndef UI_OZONE_PLATFORM_EGLFS_OZONE_PLATFORM_EGLFS_H_
#define UI_OZONE_PLATFORM_EGLFS_OZONE_PLATFORM_EGLFS_H_

#include "ui/events/ozone/evdev/event_factory.h"
#include "ui/gfx/ozone/dri/dri_surface_factory.h"
#include "ui/ozone/ozone_platform.h"

#include "surface_factory_qt.h"

#if defined(USE_OZONE)

namespace ui {

class OzonePlatformEglfs : public OzonePlatform {
 public:
  OzonePlatformEglfs();
  virtual ~OzonePlatformEglfs();

  virtual gfx::SurfaceFactoryOzone* GetSurfaceFactoryOzone() OVERRIDE;
  virtual ui::EventFactoryOzone* GetEventFactoryOzone() OVERRIDE;

 private:
  SurfaceFactoryQt surface_factory_ozone_;
  ui::EventFactoryEvdev event_factory_ozone_;

  DISALLOW_COPY_AND_ASSIGN(OzonePlatformEglfs);
};

// Constructor hook for use in ozone_platform_list.cc
OzonePlatform* CreateOzonePlatformEglfs();

}  // namespace ui

#endif // defined(USE_OZONE)
#endif // UI_OZONE_PLATFORM_EGLFS_OZONE_PLATFORM_EGLFS_H_
