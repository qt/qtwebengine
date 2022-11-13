// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef COLOR_CHOOSER_CONTROLLER_P_H
#define COLOR_CHOOSER_CONTROLLER_P_H

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

#include <QColor>

namespace content {
class WebContents;
}

namespace QtWebEngineCore {

class ColorChooserControllerPrivate {

public:
    ColorChooserControllerPrivate(content::WebContents *, const QColor &);
    content::WebContents *m_content;
    QColor m_initialColor;
};

} // namespace QtWebEngineCore

#endif // COLOR_CHOOSER_CONTROLLER_P_H

