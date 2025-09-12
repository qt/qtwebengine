// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef DISPLAY_INFO_PROVIDER_QT_H_
#define DISPLAY_INFO_PROVIDER_QT_H_

#include "extensions/browser/display_info_provider_base.h"

namespace extensions {

class DisplayInfoProviderQt : public DisplayInfoProviderBase
{
public:
    DisplayInfoProviderQt();
};

}  // namespace extensions

#endif // DISPLAY_INFO_PROVIDER_QT_H_
