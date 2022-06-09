// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef OZONE_PLATFORM_QT_H
#define OZONE_PLATFORM_QT_H

#if defined(USE_OZONE)

#include "ui/ozone/public/ozone_platform.h"

namespace ui {

// Constructor hook for use in ozone_platform_list.cc
OzonePlatform* CreateOzonePlatformQt();

}  // namespace ui

#endif // defined(USE_OZONE)
#endif // OZONE_PLATFORM_QT_H
