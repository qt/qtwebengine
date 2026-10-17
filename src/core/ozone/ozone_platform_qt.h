// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef OZONE_PLATFORM_QT_H
#define OZONE_PLATFORM_QT_H

namespace ui {
class OzonePlatform;

// Constructor hook for use in ozone_platform_list.cc
OzonePlatform *CreateOzonePlatformQt();

}  // namespace ui

#endif // OZONE_PLATFORM_QT_H
