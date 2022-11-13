// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QQUICKWEBENGINETOUCHHANDLE_P_H
#define QQUICKWEBENGINETOUCHHANDLE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//
#include "touch_handle_drawable_client.h"
#include <QtQml/qqmlregistration.h>
#include <QtCore/qrect.h>
#include <QtCore/qobject.h>

namespace QtWebEngineCore {
class WebContentsAdapterClient;
}

QT_BEGIN_NAMESPACE

class QQuickItem;
class QQuickWebEngineTouchHandle : public QtWebEngineCore::TouchHandleDrawableDelegate,
                                   public QObject
{
public:
    QQuickWebEngineTouchHandle();
    void setImage(int orintation) override;
    void setBounds(const QRect &bounds) override;
    void setVisible(bool visible) override;
    void setOpacity(float opacity) override;
    void setItem(QQuickItem *item, bool hasImage);

private:
    QScopedPointer<QQuickItem> m_item;
    bool m_hasImage;
};

QT_END_NAMESPACE

#endif // QQUICKWEBENGINETOUCHHANDLE_P_H
