// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef DESKTOP_MEDIA_CONTROLLER_P_H
#define DESKTOP_MEDIA_CONTROLLER_P_H

#include <QtCore/QObject>
#include <QtWebEngineCore/private/qtwebenginecoreglobal_p.h>

#include "content/public/browser/desktop_media_id.h"
#include "base/functional/callback.h"

namespace QtWebEngineCore {
class Q_WEBENGINECORE_EXPORT DesktopMediaControllerPrivate
{
public:
    DesktopMediaControllerPrivate(base::OnceCallback<void(content::DesktopMediaID)> doneCallback);
    void selectScreen(int index);
    void selectWindow(int index);
    void cancel();
    base::OnceCallback<void(content::DesktopMediaID)> doneCallback;
    QScopedPointer<DesktopMediaListQt> screens;
    QScopedPointer<DesktopMediaListQt> windows;
    int numInitialized;
};

} // namespace QtWebEngineCore

#endif // DESKTOP_MEDIA_CONTROLLER_P_H
