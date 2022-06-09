// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef TOUCH_HANDLE_DRAWABLE_CLIENT_H
#define TOUCH_HANDLE_DRAWABLE_CLIENT_H

#include <QtWebEngineCore/private/qtwebenginecoreglobal_p.h>
#include <QRect>

namespace QtWebEngineCore {

class Q_WEBENGINECORE_PRIVATE_EXPORT TouchHandleDrawableDelegate {
public:
    virtual ~TouchHandleDrawableDelegate() { }

    virtual void setImage(int orientation) = 0;
    virtual void setBounds(const QRect &bounds) = 0;
    virtual void setVisible(bool visible) = 0;
    virtual void setOpacity(float opacity) = 0;
};

} // namespace QtWebEngineCore

#endif // TOUCH_HANDLE_DRAWABLE_CLIENT_H
