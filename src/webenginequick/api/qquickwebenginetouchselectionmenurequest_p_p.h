// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QQUICKWEBENGINETOUCHSELECTIONMENUREQUEST_P_P_H
#define QQUICKWEBENGINETOUCHSELECTIONMENUREQUEST_P_P_H

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

#include <QtWebEngineQuick/private/qquickwebengineview_p.h>
#include "qquickwebenginetouchselectionmenurequest_p.h"

namespace QtWebEngineCore {
class TouchSelectionMenuController;
}

QT_BEGIN_NAMESPACE

class QQuickWebEngineTouchSelectionMenuRequest;

class QQuickWebEngineTouchSelectionMenuRequestPrivate
{
public:
    QQuickWebEngineTouchSelectionMenuRequestPrivate(
            QRect bounds, QtWebEngineCore::TouchSelectionMenuController *touchSelectionMenuController);

    bool accepted = false;
    QRect selectionBounds;
    uint touchSelectionCommandFlags;
    int buttonCount;
};

QT_END_NAMESPACE

#endif // QQUICKWEBENGINETOUCHSELECTIONMENUREQUEST_P_P_H
